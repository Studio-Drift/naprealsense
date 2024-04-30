#pragma once

#include <nap/core.h>

// forward declares
struct rs2_intrinsics;

namespace nap
{
    enum NAPAPI ERealSenseStreamType : int
    {
        REALSENSE_STREAMTYPE_ANY            = 0,
        REALSENSE_STREAMTYPE_DEPTH          = 1, /**< Native stream of depth data produced by RealSense device */
        REALSENSE_STREAMTYPE_COLOR          = 2, /**< Native stream of color data captured by RealSense device */
        REALSENSE_STREAMTYPE_INFRARED       = 3, /**< Native stream of infrared data captured by RealSense device */
        REALSENSE_STREAMTYPE_FISH_EYE       = 4, /**< Native stream of fish-eye (wide) data captured from the dedicate motion camera */
        REALSENSE_STREAMTYPE_GYRO           = 5, /**< Native stream of gyroscope motion data produced by RealSense device */
        REALSENSE_STREAMTYPE_ACCEL          = 6, /**< Native stream of accelerometer motion data produced by RealSense device */
        REALSENSE_STREAMTYPE_GPIO           = 7, /**< Signals from external device connected through GPIO */
        REALSENSE_STREAMTYPE_POSE           = 8, /**< 6 Degrees of Freedom pose data, calculated by RealSense device */
        REALSENSE_STREAMTYPE_CONFIDENCE     = 9  /**< 4 bit per-pixel depth confidence level */
    };

    enum NAPAPI ERealSenseStreamFormat : int
    {
        REALSENSE_FORMAT_ANY             = 0, /**< When passed to enable stream, librealsense will try to provide best suited format */
        REALSENSE_FORMAT_Z16             = 1, /**< 16-bit linear depth values. The depth is meters is equal to depth scale * pixel value. */
        REALSENSE_FORMAT_DISPARITY16     = 2, /**< 16-bit float-point disparity values. Depth->Disparity conversion : Disparity = Baseline*FocalLength/Depth. */
        REALSENSE_FORMAT_XYZ32F          = 3, /**< 32-bit floating point 3D coordinates. */
        REALSENSE_FORMAT_YUYV            = 4, /**< 32-bit y0, u, y1, v data for every two pixels. Similar to YUV422 but packed in a different order - https://en.wikipedia.org/wiki/YUV */
        REALSENSE_FORMAT_RGB8            = 5, /**< 8-bit red, green and blue channels */
        REALSENSE_FORMAT_BGR8            = 6, /**< 8-bit blue, green, and red channels -- suitable for OpenCV */
        REALSENSE_FORMAT_RGBA8           = 7, /**< 8-bit red, green and blue channels + constant alpha channel equal to FF */
        REALSENSE_FORMAT_BGRA8           = 8, /**< 8-bit blue, green, and red channels + constant alpha channel equal to FF */
        REALSENSE_FORMAT_Y8              = 9, /**< 8-bit per-pixel grayscale image */
        REALSENSE_FORMAT_Y16             = 10, /**< 16-bit per-pixel grayscale image */
        REALSENSE_FORMAT_RAW10           = 11, /**< Four 10 bits per pixel luminance values packed into a 5-byte macropixel */
        REALSENSE_FORMAT_RAW16           = 12, /**< 16-bit raw image */
        REALSENSE_FORMAT_RAW8            = 13, /**< 8-bit raw image */
        REALSENSE_FORMAT_UYVY            = 14, /**< Similar to the standard YUYV pixel format, but packed in a different order */
        REALSENSE_FORMAT_MOTION_RAW      = 15, /**< Raw data from the motion sensor */
        REALSENSE_FORMAT_MOTION_XYZ32F   = 16, /**< Motion data packed as 3 32-bit float values, for X, Y, and Z axis */
        REALSENSE_FORMAT_GPIO_RAW        = 17, /**< Raw data from the external sensors hooked to one of the GPIO's */
        REALSENSE_FORMAT_6_DOF           = 18, /**< Pose data packed as floats array, containing translation vector, rotation quaternion and prediction velocities and accelerations vectors */
        REALSENSE_FORMAT_DISPARITY32     = 19, /**< 32-bit float-point disparity values. Depth->Disparity conversion : Disparity = Baseline*FocalLength/Depth */
        REALSENSE_FORMAT_Y10BPACK        = 20, /**< 16-bit per-pixel grayscale image unpacked from 10 bits per pixel packed ([8:8:8:8:2222]) grey-scale image. The data is unpacked to LSB and padded with 6 zero bits */
        REALSENSE_FORMAT_DISTANCE        = 21, /**< 32-bit float-point depth distance value.  */
        REALSENSE_FORMAT_MJPEG           = 22, /**< Bitstream encoding for video in which an image of each frame is encoded as JPEG-DIB   */
        REALSENSE_FORMAT_Y8I             = 23, /**< 8-bit per pixel interleaved. 8-bit left, 8-bit right.  */
        REALSENSE_FORMAT_Y12I            = 24, /**< 12-bit per pixel interleaved. 12-bit left, 12-bit right. Each pixel is stored in a 24-bit word in little-endian order. */
        REALSENSE_FORMAT_INZI            = 25, /**< multi-planar Depth 16bit + IR 10bit.  */
        REALSENSE_FORMAT_INVI            = 26, /**< 8-bit IR stream.  */
        REALSENSE_FORMAT_W10             = 27, /**< Grey-scale image as a bit-packed array. 4 pixel data stream taking 5 bytes */
        REALSENSE_FORMAT_Z16H            = 28, /**< Variable-length Huffman-compressed 16-bit depth values. */
        REALSENSE_FORMAT_FG              = 29, /**< 16-bit per-pixel frame grabber format. */
        REALSENSE_FORMAT_Y411            = 30, /**< 12-bit per-pixel. */
        REALSENSE_FORMAT_COUNT           = 31  /**< Number of enumeration values. Not a valid input: intended to be used in for-loops. */
    };

    enum NAPAPI ERealSenseDistortionModels : int
    {
        RS2_DISTORTION_NONE                  = 0, /**< Rectilinear images. No distortion compensation required. */
        RS2_DISTORTION_MODIFIED_BROWN_CONRADY= 1, /**< Equivalent to Brown-Conrady distortion, except that tangential distortion is applied to radially distorted points */
        RS2_DISTORTION_INVERSE_BROWN_CONRADY = 2, /**< Equivalent to Brown-Conrady distortion, except undistorts image instead of distorting it */
        RS2_DISTORTION_FTHETA                = 3, /**< F-Theta fish-eye distortion model */
        RS2_DISTORTION_BROWN_CONRADY         = 4, /**< Unmodified Brown-Conrady distortion model */
        RS2_DISTORTION_KANNALA_BRANDT4       = 5, /**< Four parameter Kannala Brandt distortion model */
        RS2_DISTORTION_COUNT                 = 6  /**< Number of enumeration values. Not a valid input: intended to be used in for-loops. */
    } ;

    struct NAPAPI RealSenseCameraIntrinsics
    {
        int           mWidth;     /**< Width of the image in pixels */
        int           mHeight;    /**< Height of the image in pixels */
        float         mPPX;       /**< Horizontal coordinate of the principal point of the image, as a pixel offset from the left edge */
        float         mPPY;       /**< Vertical coordinate of the principal point of the image, as a pixel offset from the top edge */
        float         mFX;        /**< Focal length of the image plane, as a multiple of pixel width */
        float         mFY;        /**< Focal length of the image plane, as a multiple of pixel height */
        ERealSenseDistortionModels mModel;    /**< Distortion model of the image */
        float         mCoeffs[5]; /**< Distortion coefficients. Order for Brown-Conrady: [k1, k2, p1, p2, k3]. Order for F-Theta Fish-eye: [k1, k2, k3, k4, 0]. Other models are subject to their own interpretations */

        static RealSenseCameraIntrinsics fromRS2Intrinsics(const rs2_intrinsics& intrinsics);

        rs2_intrinsics toRS2Intrinsics() const;
    };

    struct NAPAPI RealSenseCameraInfo
    {
        std::string mName;
        std::string mSerial;
        std::string mFirmware;
        std::string mProductID;
        std::string mProductLine;
        std::string mUSBDescription;
    };
}