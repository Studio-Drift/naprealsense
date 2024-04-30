#include <rtti/rtti.h>
#include <rs.h>

#include "realsensetypes.h"

RTTI_BEGIN_ENUM(nap::ERealSenseStreamFormat)
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_ANY,              "Any"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_Z16,              "Z16"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_DISPARITY16,      "Disparity16"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_XYZ32F,           "XYZ32F"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_YUYV,             "YUYV"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_RGB8,             "RGB8"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_BGR8,             "BGR8"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_RGBA8,            "RGBA8"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_BGRA8,            "BGRA8"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_Y8,               "Y8"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_Y16,              "Y16"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_RAW10,            "RAW10"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_RAW16,            "RAW16"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_RAW8,             "RAW8"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_UYVY,             "UYVY"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_MOTION_RAW,       "Motion Raw"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_GPIO_RAW,         "GPIO Raw"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_6_DOF,            "6 DOF"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_DISPARITY32,      "Disparity32"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_Y10BPACK,         "Y10B Pack"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_DISTANCE,         "Distance"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_MJPEG,            "MJPEG"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_Y8I,              "Y8I"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_Y12I,             "Y12I"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_INZI,             "INZI"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_INVI,             "INVI"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_W10,              "W10"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_Z16H,             "Z16H"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_FG,               "FG"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_Y411,             "Y411"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::REALSENSE_FORMAT_COUNT,            "Count")
RTTI_END_ENUM

RTTI_BEGIN_ENUM(nap::ERealSenseStreamType)
RTTI_ENUM_VALUE(nap::ERealSenseStreamType::REALSENSE_STREAMTYPE_ANY, "Any"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamType::REALSENSE_STREAMTYPE_DEPTH, "Depth"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamType::REALSENSE_STREAMTYPE_COLOR, "Color"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamType::REALSENSE_STREAMTYPE_INFRARED, "Infrared"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamType::REALSENSE_STREAMTYPE_FISH_EYE, "Fish Eye"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamType::REALSENSE_STREAMTYPE_GYRO, "Gyro"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamType::REALSENSE_STREAMTYPE_ACCEL, "Accel"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamType::REALSENSE_STREAMTYPE_GPIO, "GPIO"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamType::REALSENSE_STREAMTYPE_POSE, "Pose"),
RTTI_ENUM_VALUE(nap::ERealSenseStreamType::REALSENSE_STREAMTYPE_CONFIDENCE, "Confidence")
RTTI_END_ENUM

namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    // RealSenseCameraIntrincics
    //////////////////////////////////////////////////////////////////////////

    RealSenseCameraIntrinsics RealSenseCameraIntrinsics::fromRS2Intrinsics(const rs2_intrinsics& intrinsics)
    {
        RealSenseCameraIntrinsics realsense_intrinsics;
        realsense_intrinsics.mHeight = intrinsics.height;
        realsense_intrinsics.mWidth = intrinsics.width;
        for(int i = 0 ; i < 5; i++)
        {
            realsense_intrinsics.mCoeffs[i] = intrinsics.coeffs[i];
        }
        realsense_intrinsics.mFX = intrinsics.fx;
        realsense_intrinsics.mFY = intrinsics.fy;
        realsense_intrinsics.mPPX = intrinsics.ppx;
        realsense_intrinsics.mPPY = intrinsics.ppy;
        realsense_intrinsics.mModel = static_cast<ERealSenseDistortionModels>(intrinsics.model);
        return realsense_intrinsics;
    }


    rs2_intrinsics RealSenseCameraIntrinsics::toRS2Intrinsics() const
    {
        rs2_intrinsics intrinsics;
        intrinsics.height = mHeight;
        intrinsics.width = mWidth;
        for(int i = 0 ; i < 5; i++)
        {
            intrinsics.coeffs[i] = mCoeffs[i];
        }
        intrinsics.fx = mFX;
        intrinsics.fy = mFY;
        intrinsics.ppx = mPPX;
        intrinsics.ppy = mPPY;
        intrinsics.model = static_cast<rs2_distortion>(intrinsics.model);
        return intrinsics;
    }
}