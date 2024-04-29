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
    public:
        rs2::context mContext;
    };

	RealSenseService::RealSenseService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}


	RealSenseService::~RealSenseService()
	{ }


	void RealSenseService::registerObjectCreators(rtti::Factory& factory)
	{
        factory.addObjectCreator(std::make_unique<RealSenseDeviceObjectCreator>(*this));
	}


	bool RealSenseService::init(nap::utility::ErrorState& errorState)
	{
        try
        {
            // create the rs2 context
            mImpl = std::make_unique<Impl>();

            // Get a snapshot of currently connected devices
            std::vector<std::string> serials;
            auto list = mImpl->mContext.query_devices();

            nap::Logger::info("There are %d connected RealSense devices.", list.size());
            for(size_t i = 0 ; i < list.size(); i++)
            {
                rs2::device device = list[i];

                std::string serial = std::string(device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
                RealSenseCameraInfo info;
                info.mSerial = serial;
                info.mSerial = std::string(device.get_info(rs2_camera_info::RS2_CAMERA_INFO_SERIAL_NUMBER));
                info.mFirmware = std::string(device.get_info(rs2_camera_info::RS2_CAMERA_INFO_FIRMWARE_VERSION));
                info.mProductID = std::string(device.get_info(rs2_camera_info::RS2_CAMERA_INFO_PRODUCT_ID));
                info.mProductLine = std::string(device.get_info(rs2_camera_info::RS2_CAMERA_INFO_PRODUCT_LINE));
                info.mUSBDescription = std::string(device.get_info(rs2_camera_info::RS2_CAMERA_INFO_USB_TYPE_DESCRIPTOR));
                info.mName = std::string(device.get_info(rs2_camera_info::RS2_CAMERA_INFO_NAME));

                nap::Logger::info("RealSense device %i, an %s", i, info.mProductID.c_str());
                nap::Logger::info("    Serial number: %s", info.mSerial.c_str());
                nap::Logger::info("    Firmware version: %s", info.mFirmware.c_str());
                nap::Logger::info("    USB description: %s", info.mUSBDescription.c_str());

                serials.emplace_back(serial);
                mConnectedSerialNumbers.emplace_back(serial);
                mAvailableCameraInfos.emplace(serial, info);
                mDeviceAdded.trigger(serial);
            }

            // set the serial numbers
            mConnectedSerialNumbers = serials;

            //
            mImpl->mContext.set_devices_changed_callback([this](rs2::event_information& info)
            {
                mQueryDevices.store(true);
            });
        }catch(const rs2::error& e)
        {
            nap::Logger::error(utility::stringFormat("Error query RealSense devices : %s(%s)\n      %s",
                                           e.get_failed_function().c_str(),
                                           e.get_failed_args().c_str(),
                                           e.what()));
        }
        catch(const std::exception& e)
        {
            nap::Logger::error(utility::stringFormat("Error query RealSense devices : %s",
                                                     e.what()));
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
        if(mQueryDevices.load())
        {
            nap::Logger::info("RealSenseService: %s", "Device change detected");
            mQueryDevices.store(false);

            try
            {
                auto list = mImpl->mContext.query_devices();

                // create lists of devices to either stop or restart, they will be pushed to the concurrent queue later
                std::vector<std::string> found_devices;
                std::vector<std::string> devices_to_restart;
                std::vector<std::string> devices_to_stop;

                //
                // Iterate over connected serials and look if any new devices have been added
                // If so, add them to the devices to restart
                for(size_t i = 0 ; i < list.size(); i++)
                {
                    rs2::device device = list[i];
                    auto serial = std::string(device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
                    found_devices.emplace_back(serial);
                    auto it = std::find_if(mConnectedSerialNumbers.begin(), mConnectedSerialNumbers.end(), [serial](const std::string& other)
                    {
                        return other == serial;
                    });
                    if(it == mConnectedSerialNumbers.end())
                    {
                        nap::Logger::info(utility::stringFormat("New RealSense device connected : %s", serial.c_str()));

                        std::string serial = std::string(device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
                        RealSenseCameraInfo info;
                        info.mSerial = serial;
                        info.mSerial = std::string(device.get_info(rs2_camera_info::RS2_CAMERA_INFO_SERIAL_NUMBER));
                        info.mFirmware = std::string(device.get_info(rs2_camera_info::RS2_CAMERA_INFO_FIRMWARE_VERSION));
                        info.mProductID = std::string(device.get_info(rs2_camera_info::RS2_CAMERA_INFO_PRODUCT_ID));
                        info.mProductLine = std::string(device.get_info(rs2_camera_info::RS2_CAMERA_INFO_PRODUCT_LINE));
                        info.mUSBDescription = std::string(device.get_info(rs2_camera_info::RS2_CAMERA_INFO_USB_TYPE_DESCRIPTOR));
                        info.mName = std::string(device.get_info(rs2_camera_info::RS2_CAMERA_INFO_NAME));

                        nap::Logger::info("RealSense device %i, an %s", i, info.mProductID.c_str());
                        nap::Logger::info("    Serial number: %s", info.mSerial.c_str());
                        nap::Logger::info("    Firmware version: %s", info.mFirmware.c_str());
                        nap::Logger::info("    USB description: %s", info.mUSBDescription.c_str());

                        mConnectedSerialNumbers.emplace_back(serial);
                        mAvailableCameraInfos.emplace(serial, info);
                        devices_to_restart.emplace_back(serial);
                        mDeviceAdded.trigger(serial);
                    }
                }

                //
                // Iterate over previous serials and see if a device has been disconnected or disappeared
                auto c_it = mConnectedSerialNumbers.begin();
                while(c_it != mConnectedSerialNumbers.end())
                {
                    auto serial = *(c_it);
                    auto found = std::find_if(found_devices.begin(), found_devices.end(), [serial](const std::string& other)
                    {
                        return other == serial;
                    }) != found_devices.end();
                    if(!found)
                    {
                        nap::Logger::info(utility::stringFormat("RealSense device disconnected %s", serial.c_str()));
                        devices_to_stop.emplace_back(serial);
                        c_it = mConnectedSerialNumbers.erase(c_it);
                        mAvailableCameraInfos.erase(serial);
                        mDeviceRemoved.trigger(serial);
                    }else
                    {
                        c_it++;
                    }
                }

                mConnectedSerialNumbers = found_devices;

                //  stop devices that need to stop
                for(const auto& device_to_stop : devices_to_stop)
                {
                    for(const auto& device : mDevices)
                    {
                        if(device->getIsConnected() && device->getCameraInfo().mSerial == device_to_stop)
                        {
                            device->stop();
                        }
                    }
                }

                // restart devices that need to restart
                for(const auto& device_to_restart : devices_to_restart)
                {
                    // Iterate trough registered devices
                    for(const auto& device : mDevices)
                    {
                        if(!device->getIsConnected() && // device is disconnected
                           (device->getCameraInfo().mSerial == device_to_restart || // and serial matches the device to restart
                            device->mSerial.empty())) // OR serial is empty, in which case it maybe can make a claim on this device
                        {
                            // First check if any other devices make a claim on this device
                            bool skip = false;
                            for(auto& other_device : mDevices)
                            {
                                if(other_device!=device)
                                {
                                    if(other_device->mSerial == device_to_restart || // another device has a claim
                                       // OR, other device is disconnected and was previously connected to this serial
                                       (other_device->getCameraInfo().mSerial == device_to_restart && !other_device->getIsConnected()))
                                    {
                                        skip = true;
                                        break;
                                    }
                                }
                            }
                            if(skip)
                                continue;

                            // Restart this device
                            nap::Logger::info("Restarting device : %s", device_to_restart.c_str());
                            utility::ErrorState error_state;
                            if(!device->restart(error_state))
                            {
                                nap::Logger::error("Error restarting device %s : %s", device_to_restart.c_str(), error_state.toString().c_str());
                            }
                        }
                    }
                }
            }catch(const rs2::error& e)
            {
                nap::Logger::error(utility::stringFormat("Error query RealSense devices : %s(%s)\n      %s",
                                                         e.get_failed_function().c_str(),
                                                         e.get_failed_args().c_str(),
                                                         e.what()));
            }
            catch(const std::exception& e)
            {
                nap::Logger::error(utility::stringFormat("Error query RealSense devices : %s",
                                                         e.what()));
            }
        }
	}


	void RealSenseService::shutdown()
	{
	}


    bool RealSenseService::registerDevice(nap::RealSenseDevice *device, utility::ErrorState& errorState)
    {
        auto it = std::find(mDevices.begin(), mDevices.end(), device);
        if(it != mDevices.end())
        {
            errorState.fail("Device already registered");
            return false;
        }
        for(auto* other : mDevices)
        {
            if(!other->mSerial.empty() && !device->mSerial.empty())
            {
                if(other->mSerial==device->mSerial)
                {
                    errorState.fail(utility::stringFormat("Device with serial %s already registered", device->mSerial.c_str()));
                    return false;
                }
            }
        }

        mDevices.emplace_back(device);

        return true;
    }


    bool RealSenseService::hasSerialNumber(const std::string& serialNumber)
    {
        auto it = std::find_if(mConnectedSerialNumbers.begin(), mConnectedSerialNumbers.end(), [this, serialNumber](const std::string& other)
        {
            return other == serialNumber;
        });
        return it != mConnectedSerialNumbers.end();
    }


    void RealSenseService::removeDevice(nap::RealSenseDevice *device)
    {
        auto it = std::find(mDevices.begin(), mDevices.end(), device);
        assert(it != mDevices.end()); // device does not exist
        mDevices.erase(it);
    }
}
