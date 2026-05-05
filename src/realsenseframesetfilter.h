/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <rtti/rtti.h>
#include <nap/resourceptr.h>

// Local includes
#include "realsensetypes.h"
#include "realsenseframesetlistenercomponent.h"

namespace rs2
{
    class frameset;
}

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    /**
     * RealSenseFrameSetFilter is a filter that can be applied to a set of frames
     */
    class NAPAPI RealSenseFrameSetFilter : public Resource
    {
    RTTI_ENABLE(Resource)
    public:
        /**
         * Constructor
         */
        RealSenseFrameSetFilter();

        /**
         * Destructor
         */
        virtual ~RealSenseFrameSetFilter();

        /**
         * Called from RealSenseDevice on RealSense process thread before frameset is passed onto FrameSetListener components
         * @param frameset the frameset
         * @return the filtered frameset
         */
        virtual rs2::frameset process(RealSenseDevice* device, const rs2::frameset& frameset) = 0;
    protected:
    };

    /**
     * Aligns a frameset to a specified stream type (f.e. color to depth or depth to color)
     */
    class NAPAPI RealSenseFrameSetAlignFilter final : public RealSenseFrameSetFilter
    {
    RTTI_ENABLE(RealSenseFrameSetFilter)
    public:
        /**
         * Constructor
         */
        RealSenseFrameSetAlignFilter();

        /**
         * Destructor
         */
        virtual ~RealSenseFrameSetAlignFilter();

        /**
         * Called from RealSenseDevice on RealSense process thread before frameset is passed onto FrameSetListener components
         * @param frameset the frameset
         * @return the filtered frameset
         */
        rs2::frameset process(RealSenseDevice* device, const rs2::frameset& frameset) override;

        /**
         * Initialization
         * @param errorState contains any errors
         * @return true on success
         */
        bool init(utility::ErrorState& errorState) override;

        ERealSenseStreamType mStreamType = ERealSenseStreamType::REALSENSE_STREAMTYPE_DEPTH; ///< Property: 'Stream' stream type to align to
    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };


    class NAPAPI RealSenseFrameSetCutDistanceFilter final : public RealSenseFrameSetFilter
    {
    RTTI_ENABLE(RealSenseFrameSetFilter)
    public:
        /**
         * Constructor
         */
        RealSenseFrameSetCutDistanceFilter();

        /**
         * Destructor
         */
        virtual ~RealSenseFrameSetCutDistanceFilter();

        /**
         * Called from RealSenseDevice on RealSense process thread before frameset is passed onto FrameSetListener components
         * @param frameset the frameset
         * @return the filtered frameset
         */
        rs2::frameset process(RealSenseDevice* device, const rs2::frameset& frameset) override;

        /**
         * Initialization
         * @param errorState contains any errors
         * @return true on success
         */
        bool init(utility::ErrorState& errorState) override;

        /**
         * Changes cutting range
         * @param range
         */
        void changeCuttingRange(const glm::vec2& range);

        glm::vec2 mCuttingRange = { 0.2f, 2.0f };    ///< Property: 'CuttingRange'
    private:
        std::mutex mMutex;
    };


    class NAPAPI RealSenseFrameCropBordersFilter final : public RealSenseFrameSetFilter
    {
    RTTI_ENABLE(RealSenseFrameSetFilter)
    public:
        /**
         * Constructor
         */
        RealSenseFrameCropBordersFilter();

        /**
         * Destructor
         */
        virtual ~RealSenseFrameCropBordersFilter();

        /**
         * Called from RealSenseDevice on RealSense process thread before frameset is passed onto FrameSetListener components
         * @param frameset the frameset
         * @return the filtered frameset
         */
        rs2::frameset process(RealSenseDevice* device, const rs2::frameset& frameset) override;

        /**
         * Initialization
         * @param errorState contains any errors
         * @return true on success
         */
        bool init(utility::ErrorState& errorState) override;

        /**
         * Changes crop
         * @param range
         */
        void changeCrop(const glm::vec4& crop);

        glm::vec4 mBorderCrop = { 0.0f, 0.0f, 0.0f, 0.0f };    ///< Property: 'BorderCrop'
    private:
        std::mutex mMutex;
    };


    class NAPAPI RealSenseFrameHolesFillingFilter final : public RealSenseFrameSetFilter
    {
    RTTI_ENABLE(RealSenseFrameSetFilter)
    public:
        enum NAPAPI EHoleFilling : int
        {
            FILL_FROM_LEFT = 0,
            FARTHEST_FROM_AROUND = 1,
            NEAREST_FROM_AROUND = 2
        };
        /**
         * Constructor
         */
        RealSenseFrameHolesFillingFilter();

        /**
         * Destructor
         */
        virtual ~RealSenseFrameHolesFillingFilter();

        /**
         * Called from RealSenseDevice on RealSense process thread before frameset is passed onto FrameSetListener components
         * @param frameset the frameset
         * @return the filtered frameset
         */
        rs2::frameset process(RealSenseDevice* device, const rs2::frameset& frameset) override;

        /**
         * Initialization
         * @param errorState contains any errors
         * @return true on success
         */
        bool init(utility::ErrorState& errorState) override;

        EHoleFilling mHoleFilling = EHoleFilling::FARTHEST_FROM_AROUND;
    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };


    class NAPAPI RealSenseTemporalFilter final : public RealSenseFrameSetFilter
    {
    RTTI_ENABLE(RealSenseFrameSetFilter)
    public:
        /**
         * Constructor
         */
        RealSenseTemporalFilter();

        /**
         * Destructor
         */
        virtual ~RealSenseTemporalFilter();

        /**
         * Called from RealSenseDevice on RealSense process thread before frameset is passed onto FrameSetListener components
         * @param frameset the frameset
         * @return the filtered frameset
         */
        rs2::frameset process(RealSenseDevice* device, const rs2::frameset& frameset) override;

        /**
         * Initialization
         * @param errorState contains any errors
         * @return true on success
         */
        bool init(utility::ErrorState& errorState) override;

        float mSmoothAlpha = 0.4f;
        int mSmoothDelta = 20;
        int mPersistencyIndex = 3;
    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };
}
