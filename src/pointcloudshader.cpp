/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes

#include "realsenseservice.h"

// External includes
#include <nap/core.h>
#include <pointcloudshader.h>
#include <renderservice.h>

// nap::PointCloudShader run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PointCloudShader)
    RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// PointCloudShader
//////////////////////////////////////////////////////////////////////////

namespace nap
{
    namespace shader
    {
        inline constexpr const char* pointcloud = "pointcloud";
    }


    PointCloudShader::PointCloudShader(Core& core) : Shader(core),
                                                     mRenderService(core.getService<RenderService>()) { }



    bool PointCloudShader::init(utility::ErrorState& errorState)
    {
        std::string relative_path = utility::joinPath({ "shaders", utility::appendFileExtension(shader::pointcloud, "vert") });
        const std::string vertex_shader_path = mRenderService->getCore().getService<RealSenseService>()->getModule().findAsset(relative_path);
        if (!errorState.check(!vertex_shader_path.empty(),
                              "%s: Unable to find %s vertex shader %s",
                              mRenderService->getCore().getService<RealSenseService>()->getModule().getName().c_str(),
                              shader::pointcloud,
                              vertex_shader_path.c_str()))
            return false;

        relative_path = utility::joinPath({ "shaders", utility::appendFileExtension(shader::pointcloud, "frag") });
        const std::string fragment_shader_path = mRenderService->getCore().getService<RealSenseService>()->getModule().findAsset(relative_path);
        if (!errorState.check(!vertex_shader_path.empty(),
                              "%s: Unable to find %s fragment shader %s",
                              mRenderService->getCore().getService<RealSenseService>()->getModule().getName().c_str(),
                              shader::pointcloud,
                              fragment_shader_path.c_str()))
            return false;

        // Read vert shader file
        std::string vert_source;
        if (!errorState.check(utility::readFileToString(vertex_shader_path, vert_source, errorState), "Unable to read %s vertex shader file", shader::pointcloud))
            return false;

        // Read frag shader file
        std::string frag_source;
        if (!errorState.check(utility::readFileToString(fragment_shader_path, frag_source, errorState), "Unable to read %s fragment shader file", shader::pointcloud))
            return false;

        // Compile shader
        return this->load(shader::pointcloud, {}, vert_source.data(), vert_source.size(), frag_source.data(), frag_source.size(), errorState);
    }
}
