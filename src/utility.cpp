#include <cstring>
#include "utility.h"

#include <rs.hpp>

namespace nap
{
namespace realsense
{
    void removeBackground(const rs2::depth_frame& depth_frame,
                           rs2::video_frame& other_frame,
                           float depth_scale,
                           glm::vec2 clipping_dist)
    {
        const uint16_t *p_depth_frame = reinterpret_cast<const uint16_t *>(depth_frame.get_data());
        uint8_t *p_other_frame = reinterpret_cast<uint8_t *>(const_cast<void *>(other_frame.get_data()));

        int width = other_frame.get_width();
        int height = other_frame.get_height();
        int other_bpp = other_frame.get_bytes_per_pixel();

#pragma omp parallel for schedule(dynamic) //Using OpenMP to try to parallelise the loop
        for (int y = 0; y < height; y++)
        {
            auto depth_pixel_index = y * width;
            for (int x = 0; x < width; x++, ++depth_pixel_index)
            {
                // Get the depth value of the current pixel
                auto pixels_distance = depth_scale * p_depth_frame[depth_pixel_index];

                // Check if the depth value is invalid (<=0) or greater than the threashold
                if (pixels_distance <= clipping_dist.x || pixels_distance > clipping_dist.y)
                {
                    // Calculate the offset in other frame's buffer to current pixel
                    auto offset = depth_pixel_index * other_bpp;

                    // Set pixel to "background" color
                    std::memset(&p_other_frame[offset], 0, other_bpp);
                }
            }
        }
    }


    void crop(const rs2::depth_frame& depth_frame,
              rs2::video_frame& other_frame,
              const glm::vec4& crop)
    {
        const uint16_t *p_depth_frame = reinterpret_cast<const uint16_t *>(depth_frame.get_data());
        uint8_t *p_other_frame = reinterpret_cast<uint8_t *>(const_cast<void *>(other_frame.get_data()));

        int width = other_frame.get_width();
        int height = other_frame.get_height();
        int other_bpp = other_frame.get_bytes_per_pixel();

#pragma omp parallel for schedule(dynamic) //Using OpenMP to try to parallelise the loop
        for (int y = 0; y < height; y++)
        {
            auto depth_pixel_index = y * width;
            float y_norm = static_cast<float>(y) / static_cast<float>(height);

            if(y_norm > crop.y && y_norm < 1.0f - crop.w)
            {
                for (int x = 0; x < width; x++, ++depth_pixel_index)
                {
                    float x_norm = static_cast<float>(x) / static_cast<float>(width);
                    if(x_norm < crop.x || x_norm > 1.0f - crop.z)
                    {
                        // Calculate the offset in other frame's buffer to current pixel
                        auto offset = depth_pixel_index * other_bpp;

                        // Set pixel to "background" color
                        std::memset(&p_other_frame[offset], 0, other_bpp);
                    }
                }
            }else
            {
                for (int x = 0; x < width; x++, ++depth_pixel_index)
                {
                    // Calculate the offset in other frame's buffer to current pixel
                    auto offset = depth_pixel_index * other_bpp;

                    // Set pixel to "background" color
                    std::memset(&p_other_frame[offset], 0, other_bpp);
                }
            }
        }
    }
} // realsense
} // nap