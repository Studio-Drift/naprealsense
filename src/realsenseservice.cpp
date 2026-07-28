/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "realsensedevice.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>
#include <iostream>
#include <utility/stringutils.h>

// Local Includes
#include "realsenseservice.h"

// RealSense includes
#include <rs.hpp>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseService)
    RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    // RealSenseService
    //////////////////////////////////////////////////////////////////////////

    struct RealSenseService::Impl
    {
        Impl(const std::string& settings) :
            mContext(settings) {}

        rs2::context mContext;
    };

	RealSenseService::RealSenseService(ServiceConfiguration* configuration) :
		Service(configuration)
	{ }


	RealSenseService::~RealSenseService()
	{ }


	void RealSenseService::registerObjectCreators(rtti::Factory& factory)
	{
        factory.addObjectCreator(std::make_unique<RealSenseDeviceObjectCreator>(*this));
	}


	bool RealSenseService::init(utility::ErrorState& errorState)
	{
        // Ensure DDS is enabled
        const auto settings = R"(
            {
	            "dds": {
		            "enabled": true,
					"device-initialization-timeout-ms": 5000,
					"device": {
					    "control": {
					        "reply-timeout-ms": 2000
					    }
					}
	            }
            }
        )";

        // Create the rs2 context
        mImpl = std::make_unique<Impl>(settings);

	    // Scan for RealSense devices
	    if (!scan(errorState))
	        return false;

	    try
	    {
            mImpl->mContext.set_devices_changed_callback([this](rs2::event_information& info)
            {
                mQueryDevices.store(true);
            });
        }
	    catch(const rs2::error& e)
        {
            errorState.fail("Error query RealSense devices : %s(%s)\n      %s",
                e.get_failed_function().c_str(),
                e.get_failed_args().c_str(),
                e.what());
	        return false;
        }
        catch(const std::exception& e)
        {
            errorState.fail(utility::stringFormat("Error query RealSense devices : %s", e.what()));
            return false;
        }
		return true;
	}


    bool RealSenseService::scan(utility::ErrorState& errorState)
    {
	    // Create lists of devices to either stop or restart, they will be pushed to the concurrent queue later
	    std::vector<std::string> devices_found;
	    std::vector<std::string> devices_to_restart;

	    try
	    {
	        // Get a snapshot of currently connected devices
            const auto list = mImpl->mContext.query_devices(RS2_PRODUCT_LINE_ANY_INTEL);
            Logger::info("RealSense scan: found %d connected device(s).", list.size());

            for (size_t i = 0 ; i < list.size(); i++)
            {
                const auto device = list[i];

                if (!device.supports(RS2_CAMERA_INFO_SERIAL_NUMBER))
                {
                    Logger::warn("Error querying serial for RealSense device %d", i);
                    continue;
                }

            	const std::string serial = device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);
            	if (serial.empty())
            	{
            		Logger::warn("Error querying serial for RealSense device %d", i);
            		continue;
            	}

            	rs2::eth_config_device eth_device(device);
            	if (eth_device.supports_eth_config())
            	{
            		eth_device.set_link_priority(RS2_LINK_PRIORITY_ETH_FIRST);
            		eth_device.set_link_timeout(30000);

            		// Not supported on all firmware
            		// eth_device.set_transmission_delay(48);
            	}

                // Find new devices
                devices_found.emplace_back(device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
                if (auto it = std::find(mConnectedSerialNumbers.begin(), mConnectedSerialNumbers.end(), serial); it == mConnectedSerialNumbers.end())
                {
                    RealSenseCameraInfo info{device};
                    Logger::info("RealSense device %i: %s", i, info.mName.c_str());
                    Logger::info("    Connection type: %s", info.mType.c_str());
                    Logger::info("    Serial number: %s", info.mSerial.c_str());
                    Logger::info("    Product line: %s", info.mProductLine.c_str());
                    Logger::info("    Firmware version: %s", info.mFirmware.c_str());
                	Logger::info("    USB Description: %s", info.mUSBDescription.c_str());

                    mDeviceAdded.trigger(serial);
                    mConnectedSerialNumbers.emplace_back(serial);
                    mAvailableCameraInfos.emplace(serial, std::move(info));
                    devices_to_restart.emplace_back(serial);
                }
            }
	    }
	    catch(const rs2::error& e)
	    {
	        errorState.fail("Error query RealSense devices : %s(%s)\n      %s",
                e.get_failed_function().c_str(),
                e.get_failed_args().c_str(),
                e.what());
	        return false;
	    }
	    catch(const std::exception& e)
	    {
	        errorState.fail("Error query RealSense devices : %s", e.what());
	        return false;
	    }

        // Iterate over previous serials and see if a device has been disconnected or disappeared
	    std::vector<std::string> devices_to_stop;
        auto it = mConnectedSerialNumbers.begin();
        while (it != mConnectedSerialNumbers.end())
        {
            if (std::find(devices_found.begin(), devices_found.end(), *it) == devices_found.end())
            {
                Logger::info(utility::stringFormat("RealSense device disconnected %s", it->c_str()));
                devices_to_stop.emplace_back(*it);

                it = mConnectedSerialNumbers.erase(it);
                mAvailableCameraInfos.erase(*it);
                mDeviceRemoved.trigger(*it);
                continue;
            }
            ++it;
        }
	    mConnectedSerialNumbers = devices_found;

	    // Stop devices
	    for (const auto& dev_to_stop : devices_to_stop)
	    {
	        for (const auto& dev : mDevices)
	        {
	            if (dev->isConnected() && dev->getCameraInfo().mSerial == dev_to_stop)
	                dev->stop();
	        }
	    }

	    // Restart devices
        for (const auto& dev_to_restart : devices_to_restart)
        {
            // Iterate through registered devices
            for (const auto& dev : mDevices)
            {
                // Check if the device is connected and whether another device can claim the current device
                if (dev->isConnected() || (dev->getCameraInfo().mSerial != dev_to_restart && !dev->mSerial.empty()))
                    continue;

                // First check if any other devices make a claim on this device
                bool skip = false;
                for (auto& other_dev : mDevices)
                {
                    if (other_dev == dev)
                        continue;

                    // another device has a claim OR other device is disconnected and was previously connected to this serial
                    if (other_dev->mSerial == dev_to_restart || (other_dev->getCameraInfo().mSerial == dev_to_restart && !other_dev->isConnected()))
                    {
                        skip = true;
                        break;
                    }
                }
                if (skip)
                    continue;

                // Restart this device
                Logger::info("Restarting device : %s", dev_to_restart.c_str());
                utility::ErrorState err;
                if (!dev->restart(err))
                    Logger::error("Error restarting device %s : %s", dev_to_restart.c_str(), err.toString().c_str());
            }
        }
	    return true;
	}


    const RealSenseCameraInfo& RealSenseService::getCameraInfo(const std::string& serial)
    {
        assert(mAvailableCameraInfos.find(serial)!=mAvailableCameraInfos.end());
        return mAvailableCameraInfos[serial];
    }


	void RealSenseService::update(double deltaTime)
	{
        if (mQueryDevices.load())
        {
            Logger::info("RealSenseService: Device change detected");
            mQueryDevices.store(false);

            utility::ErrorState error_state;
            if (!scan(error_state))
                Logger::error(error_state.toString());
        }
	}


	void RealSenseService::shutdown()
	{
	}


    bool RealSenseService::registerDevice(nap::RealSenseDevice *device, utility::ErrorState& errorState)
    {
		if (device->mSerial.empty())
		{
			errorState.fail("Empty serial specified");
			return false;
		}

        if (std::find(mDevices.begin(), mDevices.end(), device) != mDevices.end())
        {
            errorState.fail("Device already registered");
            return false;
        }

		const auto it = std::find_if(mDevices.begin(), mDevices.end(), [dev=device](auto& other) {
			return other->mSerial == dev->mSerial;
		});

		if (!errorState.check(it == mDevices.end(), "Device with serial %s already registered", device->mSerial.c_str()))
			return false;

        mDevices.emplace_back(device);
        return true;
    }


    bool RealSenseService::hasSerialNumber(const std::string& serialNumber)
    {
        return std::find(mConnectedSerialNumbers.begin(), mConnectedSerialNumbers.end(), serialNumber) != mConnectedSerialNumbers.end();
    }


    void RealSenseService::removeDevice(nap::RealSenseDevice *device)
    {
        auto it = std::find(mDevices.begin(), mDevices.end(), device);
        assert(it != mDevices.end()); // device does not exist
        mDevices.erase(it);
    }


	const void* RealSenseService::getContext() const
    {
		return &mImpl->mContext;
	}
}
