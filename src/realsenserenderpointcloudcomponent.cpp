#include "realsenserenderpointcloudcomponent.h"
#include "realsensedevice.h"
#include "renderglobals.h"

#include <renderservice.h>
#include <meshutils.h>

// RealSense includes
#include <rs.hpp>

RTTI_BEGIN_CLASS(nap::RealSenseRenderPointCloudComponent)
        RTTI_PROPERTY("Device", &nap::RealSenseRenderPointCloudComponent::mDevice, nap::rtti::EPropertyMetaData::Required)
        RTTI_PROPERTY("CameraTransform", &nap::RealSenseRenderPointCloudComponent::mCameraTransform, nap::rtti::EPropertyMetaData::Required)
        RTTI_PROPERTY("PointSize", &nap::RealSenseRenderPointCloudComponent::mPointSize, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("MaxDistance", &nap::RealSenseRenderPointCloudComponent::mMaxDistance, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("FramesRenderer", &nap::RealSenseRenderPointCloudComponent::mFramesRenderer, nap::rtti::EPropertyMetaData::Required)
        RTTI_PROPERTY("IntrinsicsType", &nap::RealSenseRenderPointCloudComponent::mCameraIntrinsicsStreamType, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseRenderPointCloudComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    // RealSenseRenderPointCloudComponent
    //////////////////////////////////////////////////////////////////////////


    RealSenseRenderPointCloudComponent::RealSenseRenderPointCloudComponent(){}


    RealSenseRenderPointCloudComponent::~RealSenseRenderPointCloudComponent(){}


    //////////////////////////////////////////////////////////////////////////
    // RealSenseRenderPointCloudComponentInstance
    //////////////////////////////////////////////////////////////////////////


    RealSenseRenderPointCloudComponentInstance::RealSenseRenderPointCloudComponentInstance(EntityInstance& entity, Component& resource)
        : RenderableMeshComponentInstance(entity, resource)
    {
    }


    RealSenseRenderPointCloudComponentInstance::~RealSenseRenderPointCloudComponentInstance()
    {

    }


    bool RealSenseRenderPointCloudComponentInstance::init(utility::ErrorState& errorState)
    {
        if(!RenderableMeshComponentInstance::init(errorState))
            return false;

        auto* resource = getComponent<RealSenseRenderPointCloudComponent>();
        mDevice = resource->mDevice.get();
        mPointSize = resource->mPointSize;
        mMaxDistance = resource->mMaxDistance;
        mCameraIntrinsicsStreamType = resource->mCameraIntrinsicsStreamType;

        return true;
    }


    void RealSenseRenderPointCloudComponentInstance::onDraw(nap::IRenderTarget& renderTarget,
                                                            VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix,
                                                            const glm::mat4& projectionMatrix)
    {
        if(mReady)
            RenderableMeshComponentInstance::onDraw(renderTarget, commandBuffer, viewMatrix, projectionMatrix);
    }


    void RealSenseRenderPointCloudComponentInstance::update(double deltaTime)
    {
        mReady = mFramesRenderer->isRenderTextureInitialized(ERealSenseStreamType::REALSENSE_STREAMTYPE_DEPTH) &&
                mFramesRenderer->isRenderTextureInitialized(ERealSenseStreamType::REALSENSE_STREAMTYPE_COLOR) &&
                mDevice->getIsConnected();
        if(!mReady)
            return;

        auto& material_instance = getMaterialInstance();
        auto* depth_sampler = material_instance.getOrCreateSampler<Sampler2DInstance>("depth_texture");
        depth_sampler->setTexture(mFramesRenderer->getRenderTexture(ERealSenseStreamType::REALSENSE_STREAMTYPE_DEPTH));

        auto* color_sampler = material_instance.getOrCreateSampler<Sampler2DInstance>("color_texture");
        color_sampler->setTexture(mFramesRenderer->getRenderTexture(ERealSenseStreamType::REALSENSE_STREAMTYPE_COLOR));

        const auto& camera_intrinsics = mDevice->getIntrincicsMap();
        const auto& intrinsics = camera_intrinsics.find(mCameraIntrinsicsStreamType)->second;

        auto* ubo = material_instance.getOrCreateUniform("cam_intrinsics");
        float depth_scale = mDevice->getDepthScale();
        ubo->getOrCreateUniform<UniformFloatInstance>("width")->setValue(intrinsics.mWidth);
        ubo->getOrCreateUniform<UniformFloatInstance>("height")->setValue(intrinsics.mHeight);
        ubo->getOrCreateUniform<UniformFloatInstance>("ppx")->setValue(intrinsics.mPPX);
        ubo->getOrCreateUniform<UniformFloatInstance>("ppy")->setValue(intrinsics.mPPY);
        ubo->getOrCreateUniform<UniformFloatInstance>("fx")->setValue(intrinsics.mFX);
        ubo->getOrCreateUniform<UniformFloatInstance>("fy")->setValue(intrinsics.mFY);
        ubo->getOrCreateUniform<UniformIntInstance>("model")->setValue(static_cast<int>(intrinsics.mModel));
        ubo->getOrCreateUniform<UniformFloatArrayInstance>("coeffs")->setValue(intrinsics.mCoeffs[0], 0);
        ubo->getOrCreateUniform<UniformFloatArrayInstance>("coeffs")->setValue(intrinsics.mCoeffs[1], 1);
        ubo->getOrCreateUniform<UniformFloatArrayInstance>("coeffs")->setValue(intrinsics.mCoeffs[2], 2);
        ubo->getOrCreateUniform<UniformFloatArrayInstance>("coeffs")->setValue(intrinsics.mCoeffs[3], 3);
        ubo->getOrCreateUniform<UniformFloatArrayInstance>("coeffs")->setValue(intrinsics.mCoeffs[4], 4);

        //
        ubo = material_instance.getOrCreateUniform("UBO");
        ubo->getOrCreateUniform<UniformVec3Instance>("camera_world_position")->setValue(math::extractPosition(mCameraTransform->getGlobalTransform()));
        ubo->getOrCreateUniform<UniformFloatInstance>("realsense_depth_scale")->setValue(depth_scale);
        ubo->getOrCreateUniform<UniformFloatInstance>("point_size_scale")->setValue(mPointSize);
        ubo->getOrCreateUniform<UniformFloatInstance>("max_distance")->setValue(mMaxDistance);

    }
}
