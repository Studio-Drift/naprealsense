/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/device.h>
#include <nap/resourceptr.h>
#include <nap/signalslot.h>
#include <rtti/factory.h>
#include <thread>
#include <future>
#include <atomic>

// Local includes
#include "realsensetypes.h"
#include "realsenseframesetlistenercomponent.h"


namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    // forward declares
    class RealSenseService;
    class RealSenseFrameSetListenerComponentInstance;
    class RealSenseFrameSetFilter;

    /**
     * Describes a RealSenseStream provided by the device/camera
     */
    class NAPAPI RealSenseStreamDescription final : public Resource
    {
    RTTI_ENABLE(Resource)
    public:
        /**
         * Constructor
         */
        RealSenseStreamDescription();

        /**
         * Destructor
         */
        virtual ~RealSenseStreamDescription();

        ERealSenseStreamFormat  mFormat     = ERealSenseStreamFormat::REALSENSE_FORMAT_RGBA8; ///< Property: 'Format' stream format
        ERealSenseStreamType    mStream     = ERealSenseStreamType::REALSENSE_STREAMTYPE_COLOR; ///< Property: 'Stream' stream type
    };

    /**
     * Interface for a RealSense device/camera
     */
    class NAPAPI RealSenseDevice final : public Device
    {
    RTTI_ENABLE(Device)
    public:
        /**
         * Constructor
         * @param service
         */
        RealSenseDevice(RealSenseService& service);

        /**
         * Destructor
         */
        virtual ~RealSenseDevice();

        virtual bool init(utility::ErrorState& errorState) override;

        /**
         * Called when device needs to start
         * @param errorState contains any errors
         * @return true on success
         */
        virtual bool start(utility::ErrorState& errorState) override final;

        bool restart(utility::ErrorState& errorState);

        /**
         * Called when device needs to stop
         */
        virtual void stop() override final;

        /**
         * Called before deconstruction
         */
        virtual void onDestroy() override final;

        /**
         * Adds a frameset listener
         * @param frameSetListener
         */
        void addFrameSetListener(RealSenseFrameSetListenerComponentInstance *frameSetListener);

        /**
         * Removes a frameset listener
         * @param frameSetListener
         */
        void removeFrameSetListener(RealSenseFrameSetListenerComponentInstance* frameSetListener);

        /**
         * Returns map of camera intrinsics of every available stream
         * @return map of camera intrinsics of every available stream
         */
        const std::unordered_map<ERealSenseStreamType, RealSenseCameraIntrinsics>& getIntrincicsMap() const{ return mCameraIntrinsics; }

        /**
         * Returns current depth scale in meters per unit
         * @return current depth scale in meters per unit
         */
        float getDepthScale() const;

        /**
         * Returns camera info
         * @return camera info
         */
        const RealSenseCameraInfo& getCameraInfo() const{ return mCameraInfo; }

        /**
         * Returns whether current device is connected
         * @return true on connected
         */
        bool getIsConnected() const{ return mIsConnected; }

        // properties
        std::string mSerial; ///< Property: 'Serial' serial of device to use, keep empty to get first device available
        int mMaxFrameSize = 5; ///< Property: 'MaxFrameSize' maximum size of frame queue
        std::vector<ResourcePtr<RealSenseStreamDescription>> mStreams; ///< Property: 'Streams' settings for desired streams
        bool mAllowFailure = false; ///< Property: 'AllowFailure' return init success upon init failure
        float mMinimalRequiredUSBType = 3.0f; ///< Property: 'MinimumUSBVersion'
    private:
        /**
         * Handles error, when device is allowed to fail, will return true and log the error, otherwise it will return false
         * preventing successful app initialization
         * @param successCondition the success condition
         * @param errorMessage corresponding error message
         * @param errorState errorState struct to full
         * @return true on success, false on error
         */
        bool handleError(bool successCondition, const std::string& errorMessage, utility::ErrorState& errorState);

        /**
         * The threaded process function
         */
        void process();

        std::future<void>		mCaptureTask;
        std::atomic_bool        mRun = { false };
        std::atomic<float>      mLatestDepthScale;

        bool mIsConnected = false;

        RealSenseService&       mService;
        RealSenseCameraInfo     mCameraInfo;

        struct Impl;
        std::unique_ptr<Impl>   mImplementation;
        std::string mDeviceSerial;

        std::vector<RealSenseFrameSetListenerComponentInstance*> mFrameSetListeners;
        std::mutex mFrameSetListenerMutex;
        std::unordered_map<ERealSenseStreamType, RealSenseCameraIntrinsics> mCameraIntrinsics;
    };

    using RealSenseDeviceObjectCreator = rtti::ObjectCreator<RealSenseDevice, RealSenseService>;
}
