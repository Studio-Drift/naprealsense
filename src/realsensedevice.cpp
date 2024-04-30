#include "realsensedevice.h"
#include "realsenseservice.h"
#include "realsenseframesetlistenercomponent.h"
#include "realsenseframesetfilter.h"

// RealSense includes
#include <rs.hpp>

RTTI_BEGIN_CLASS(nap::RealSenseStreamDescription)
    RTTI_PROPERTY("Format", &nap::RealSenseStreamDescription::mFormat, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Stream", &nap::RealSenseStreamDescription::mStream, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseDevice)
    RTTI_CONSTRUCTOR(nap::RealSenseService&)
    RTTI_PROPERTY("Serial", &nap::RealSenseDevice::mSerial, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("MaxFrameSize", &nap::RealSenseDevice::mMaxFrameSize, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Streams", &nap::RealSenseDevice::mStreams, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("AllowFailure", &nap::RealSenseDevice::mAllowFailure, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("MinUSBVersion", &nap::RealSenseDevice::mMinimalRequiredUSBType, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    // RealSenseStreamDescription
    //////////////////////////////////////////////////////////////////////////

    RealSenseStreamDescription::RealSenseStreamDescription(){}


    RealSenseStreamDescription::~RealSenseStreamDescription(){}


    //////////////////////////////////////////////////////////////////////////
    // RealSenseDevice::Impl
    //////////////////////////////////////////////////////////////////////////


    struct RealSenseDevice::Impl
    {
    public:
        // Declare RealSense pipeline, encapsulating the actual device and sensors
        rs2::pipeline mPipe;

        // Frame queue
        rs2::frame_queue mFrameQueue;

        // rs2 configuration
        rs2::config mConfig;
    };


    //////////////////////////////////////////////////////////////////////////
    // RealSenseDevice
    //////////////////////////////////////////////////////////////////////////


    RealSenseDevice::RealSenseDevice(RealSenseService &service) : mService(service)
    {
    }


    RealSenseDevice::~RealSenseDevice(){}


    bool RealSenseDevice::init(utility::ErrorState &errorState)
    {
        if(!handleError(mService.registerDevice(this, errorState), "Cannot register device", errorState))
            return mAllowFailure;

        return true;
    }


    bool RealSenseDevice::start(utility::ErrorState &errorState)
    {
        if(!mRun.load())
        {
            mImplementation = std::make_unique<Impl>();
            mImplementation->mFrameQueue = rs2::frame_queue(mMaxFrameSize);

            if(!handleError(!mService.getConnectedSerialNumbers().empty(), "No RealSense devices connected!", errorState))
                return mAllowFailure;

            if(!mSerial.empty())
            {
                if(!handleError(mService.hasSerialNumber(mSerial),
                                utility::stringFormat("Device with serial number %s is not connected", mSerial.c_str()),
                                errorState))
                    return mAllowFailure;
            }

            // disable all streams
            mImplementation->mConfig.disable_all_streams();

            std::vector<ERealSenseStreamType> stream_types;
            for(const auto& stream : mStreams)
            {
                auto stream_type = stream->mStream;
                if(std::find_if(stream_types.begin(),
                                stream_types.end(),
                                [stream_type](ERealSenseStreamType other)
                                    { return stream_type == other; }) != stream_types.end())
                {
                    errorState.fail("Cannot open multiple streams of the same stream type!");
                    return mAllowFailure;
                }

                stream_types.emplace_back(stream_type);

                rs2_stream rs2_stream_type      = static_cast<rs2_stream>(stream->mStream);
                rs2_format rs2_stream_format    = static_cast<rs2_format>(stream->mFormat);
                mImplementation->mConfig.enable_stream(rs2_stream_type, rs2_stream_format);
            }

            //
            if(!mSerial.empty())
                mImplementation->mConfig.enable_device(mSerial);

            try
            {
                mImplementation->mPipe.start(mImplementation->mConfig);

                /**
                 * Gather camera intrinsics for all streams
                 */
                for(auto& stream : mStreams)
                {
                    if(mImplementation->mPipe.get_active_profile()
                        .get_stream(static_cast<rs2_stream>(stream->mStream))
                        .is<rs2::video_stream_profile>())
                    {
                        auto intrinsics_rs2 = mImplementation->mPipe
                                .get_active_profile()
                                .get_stream(static_cast<rs2_stream>(stream->mStream))
                                .as<rs2::video_stream_profile>()
                                .get_intrinsics();
                        mCameraIntrinsics.emplace(stream->mStream, RealSenseCameraIntrinsics::fromRS2Intrinsics(intrinsics_rs2));
                    }
                }

                /**
                 * Gather device info
                 */
                mCameraInfo.mName = std::string(mImplementation->mPipe.get_active_profile().get_device().get_info(rs2_camera_info::RS2_CAMERA_INFO_NAME));
                mCameraInfo.mSerial = std::string(mImplementation->mPipe.get_active_profile().get_device().get_info(rs2_camera_info::RS2_CAMERA_INFO_SERIAL_NUMBER));
                mCameraInfo.mFirmware = std::string(mImplementation->mPipe.get_active_profile().get_device().get_info(rs2_camera_info::RS2_CAMERA_INFO_FIRMWARE_VERSION));
                mCameraInfo.mProductID = std::string(mImplementation->mPipe.get_active_profile().get_device().get_info(rs2_camera_info::RS2_CAMERA_INFO_PRODUCT_ID));
                mCameraInfo.mProductLine = std::string(mImplementation->mPipe.get_active_profile().get_device().get_info(rs2_camera_info::RS2_CAMERA_INFO_PRODUCT_LINE));
                mCameraInfo.mUSBDescription = std::string(mImplementation->mPipe.get_active_profile().get_device().get_info(rs2_camera_info::RS2_CAMERA_INFO_USB_TYPE_DESCRIPTOR));

                float usb_version = std::stof(mCameraInfo.mUSBDescription);
                if(usb_version < mMinimalRequiredUSBType)
                {
                    handleError(false, utility::stringFormat("USB type invalid, must be equal or higher then %.2f, got %.2f", mMinimalRequiredUSBType, usb_version), errorState);
                    mImplementation->mPipe.stop();
                    return mAllowFailure;
                }

                // store depth scale
                mLatestDepthScale.store(mImplementation->mPipe.get_active_profile().get_device().first<rs2::depth_sensor>().get_depth_scale());
            }catch(const rs2::error& e)
            {
                handleError(false, utility::stringFormat("RealSense error calling %s(%s)\n     %s,",
                                                         e.get_failed_function().c_str(),
                                                         e.get_failed_args().c_str(),
                                                         e.what()), errorState);
                return mAllowFailure;

            }
            catch(const std::exception& e)
            {
                handleError(false, e.what(), errorState);
                return mAllowFailure;
            }

            mRun.store(true);
            mCaptureTask = std::async(std::launch::async, [this] { process(); });
            mIsConnected = true;

            return true;
        }

        handleError(false, "RealSenseDevice already started.", errorState);

        return mAllowFailure;
    }


    bool RealSenseDevice::restart(utility::ErrorState &errorState)
    {
        // If we are running, first stop
        if(mRun.load())
        {
            mRun.store(false);
            if(mCaptureTask.valid())
                mCaptureTask.wait();

            try
            {
                mImplementation->mPipe.stop();
            }catch(const rs2::error& e)
            {
                nap::Logger::error(*this, utility::stringFormat("RealSense error calling %s(%s)\n     %s,",
                                                                e.get_failed_function().c_str(),
                                                                e.get_failed_args().c_str(),
                                                                e.what()));

            }
            catch(const std::exception& e)
            {
                nap::Logger::error(*this, e.what());
            }
        }

        // Start
        return start(errorState);
    }


    bool RealSenseDevice::handleError(bool successCondition, const std::string& errorMessage, utility::ErrorState& errorState)
    {
        if(!successCondition)
        {
            if(mAllowFailure)
            {
                nap::Logger::error(*this, errorMessage);
            }else
            {
                errorState.fail(errorMessage);
            }

            return false;
        }

        return true;
    }


    float RealSenseDevice::getDepthScale() const
    {
        return mLatestDepthScale.load();
    }


    void RealSenseDevice::stop()
    {
        if(mRun.load())
        {
            mRun.store(false);
            if(mCaptureTask.valid())
                mCaptureTask.wait();

            try
            {
                mImplementation->mPipe.stop();
            }catch(const rs2::error& e)
            {
                nap::Logger::error(*this, utility::stringFormat("RealSense error calling %s(%s)\n     %s,",
                                                          e.get_failed_function().c_str(),
                                                          e.get_failed_args().c_str(),
                                                          e.what()));

            }
            catch(const std::exception& e)
            {
                nap::Logger::error(*this, e.what());
            }

            for(auto& frameset_listener : mFrameSetListeners)
            {
                frameset_listener->clear();
            }

            mCameraIntrinsics.clear();

            mIsConnected = false;
        }
    }

    void RealSenseDevice::onDestroy()
    {
        stop();
        mService.removeDevice(this);
    }


    void RealSenseDevice::addFrameSetListener(RealSenseFrameSetListenerComponentInstance* frameSetListener)
    {
        auto it = std::find(mFrameSetListeners.begin(), mFrameSetListeners.end(), frameSetListener);
        assert(it == mFrameSetListeners.end()); // device already exists

        std::lock_guard l(mFrameSetListenerMutex);
        mFrameSetListeners.emplace_back(frameSetListener);
    }


    void RealSenseDevice::removeFrameSetListener(RealSenseFrameSetListenerComponentInstance* frameSetListener)
    {
        auto it = std::find(mFrameSetListeners.begin(), mFrameSetListeners.end(), frameSetListener);
        assert(it != mFrameSetListeners.end()); // device does not exist

        std::lock_guard l(mFrameSetListenerMutex);
        mFrameSetListeners.erase(it);
    }


    void RealSenseDevice::process()
    {
        try
        {
            while(mRun.load())
            {
                uint millis = 0;

                // poll for new frameset
                rs2::frameset data;
                if(mImplementation->mPipe.poll_for_frames(&data))
                {
                    SteadyTimer timer;
                    timer.start();

                    std::lock_guard l(mFrameSetListenerMutex);
                    for(auto* frameset_listener : mFrameSetListeners)
                    {
                        frameset_listener->trigger(this, data);
                    }

                    millis = timer.getMillis().count();
                }

                int wait = 20 - millis;
                if(wait < 0)
                    wait = 0;

                std::this_thread::sleep_for(std::chrono::milliseconds(wait));
            }
        }catch(const rs2::error& e)
        {
            nap::Logger::error(*this, utility::stringFormat("RealSense error calling %s(%s)\n     %s,",
                                                            e.get_failed_function().c_str(),
                                                            e.get_failed_args().c_str(),
                                                            e.what()));

        }
        catch(const std::exception& e)
        {
            nap::Logger::error(*this, e.what());
        }

        mRun.store(false);
        mIsConnected = false;
    }
}
