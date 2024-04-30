#pragma once

#include <nap/core.h>
#include <glm/glm.hpp>

// rs2 forward declares
namespace rs2
{
    class depth_frame;
    class video_frame;
}

namespace nap
{
namespace realsense
{
    void NAPAPI removeBackground(const rs2::depth_frame& depth_frame,
                                 rs2::video_frame& other_frame,
                                 float depth_scale,
                                 glm::vec2 clipping_dist);

    void NAPAPI crop(const rs2::depth_frame& depth_frame,
                     rs2::video_frame& other_frame,
                     const glm::vec4& crop);
} // realsense
} // nap