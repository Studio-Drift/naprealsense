#include "realsenserenderframescomponent.h"
#include "realsensedevice.h"
#include "../../napimgui/src/imguiservice.h"

#include <rs.hpp>

RTTI_BEGIN_STRUCT(nap::RealSenseRenderFrameDescription)
        RTTI_PROPERTY("StreamType", &nap::RealSenseRenderFrameDescription::mStreamType, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("Format", &nap::RealSenseRenderFrameDescription::mFormat, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_CLASS(nap::RealSenseRenderFramesComponent)
    RTTI_PROPERTY("RenderDescriptions", &nap::RealSenseRenderFramesComponent::mRenderDescriptions, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("Enabled", &nap::RealSenseRenderFramesComponent::mEnabled, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseRenderFramesComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    // RealSenseRenderFramesComponent
    //////////////////////////////////////////////////////////////////////////


    RealSenseRenderFramesComponent::RealSenseRenderFramesComponent()
    {
        mInstance = nullptr;
    }


    RealSenseRenderFramesComponent::~RealSenseRenderFramesComponent(){}


    RealSenseRenderFramesComponentInstance* RealSenseRenderFramesComponent::getInstance()
    {
        return mInstance;
    }


    //////////////////////////////////////////////////////////////////////////
    // RealSenseRenderFramesComponentInstance::Impl
    //////////////////////////////////////////////////////////////////////////


    struct RealSenseRenderFramesComponentInstance::Impl
    {
    public:
        // Frame queue
        rs2::frame_queue mFrameQueue;
    };


    //////////////////////////////////////////////////////////////////////////
    // RealSenseRenderFramesComponentInstance
    //////////////////////////////////////////////////////////////////////////


    RealSenseRenderFramesComponentInstance::RealSenseRenderFramesComponentInstance(EntityInstance& entity, Component& resource) :
        RealSenseFrameSetListenerComponentInstance(entity, resource)
    {

    }


    RealSenseRenderFramesComponentInstance::~RealSenseRenderFramesComponentInstance(){}


    void RealSenseRenderFramesComponent::getDependentComponents(std::vector<rtti::TypeInfo> &components) const
    {
        components.emplace_back(RTTI_OF(RealSenseFilterStackComponent));
    }


    bool RealSenseRenderFramesComponentInstance::onInit(utility::ErrorState &errorState)
    {
        mImplementation = std::make_unique<Impl>();

        mResource = getComponent<RealSenseRenderFramesComponent>();
        mResource->mInstance = this;
        mRenderDescriptions = mResource->mRenderDescriptions;
        mEnabled.store(mResource->mEnabled);

        for(const auto& description : mRenderDescriptions)
        {
            auto render_texture = std::make_unique<RenderTexture2D>(*getEntityInstance()->getCore());
            render_texture->mWidth = 0;
            render_texture->mHeight = 0;
            render_texture->mClearColor = { 0, 0, 0 , 0 };
            render_texture->mColorSpace = EColorSpace::Linear;
            render_texture->mFormat = description.mFormat;
            if(!mRenderTextures.emplace(description.mStreamType, std::move(render_texture)).second)
            {
                errorState.fail(utility::stringFormat("%s encountered duplicate streamtype",
                                                      get_type().get_name().to_string().c_str()));
                return false;
            }
            mInitializationMap.emplace(description.mStreamType, false);
        }

        if(!errorState.check(getEntityInstance()->hasComponent<RealSenseFilterStackComponentInstance>(), "Entity has no RealSenseFilterStackComponentInstance"))
            return false;

        auto& filter_stack = getEntityInstance()->getComponent<RealSenseFilterStackComponentInstance>();
        filter_stack.addFrameSetListener(this);

        return true;
    }


    void RealSenseRenderFramesComponentInstance::destroy()
    {
        auto& filter_stack = getEntityInstance()->getComponent<RealSenseFilterStackComponentInstance>();
        filter_stack.removeFrameSetListener(this);

        auto* imgui_service = getEntityInstance()->getCore()->getService<IMGuiService>();
        for(auto& render_texture : mRenderTextures)
        {
            imgui_service->removeTextureHandle(*render_texture.second);
        }
    }


    void RealSenseRenderFramesComponentInstance::update(double deltaTime)
    {
        if(mEnabled.load())
        {
            rs2::frameset frameset;
            if(mImplementation->mFrameQueue.poll_for_frame(&frameset))
            {
                for(auto& description : mRenderDescriptions)
                {
                    auto frame = frameset.first(static_cast<rs2_stream>(description.mStreamType));
                    assert(frame.is<rs2::video_frame>());

                    ERealSenseStreamType stream_type = static_cast<ERealSenseStreamType>(frame.get_profile().stream_type());
                    assert(mRenderTextures.find(stream_type)!=mRenderTextures.end());
                    auto& render_texture = mRenderTextures.find(stream_type)->second;
                    const auto &video_frame = frame.as<rs2::video_frame>();

                    // Ensure dimensions are the same
                    glm::vec2 tex_size = render_texture->getSize();

                    if(video_frame.get_width() != tex_size.x || video_frame.get_height() != tex_size.y)
                    {
                        render_texture->mWidth = video_frame.get_width();
                        render_texture->mHeight = video_frame.get_height();
                        render_texture->mClearColor = { 0, 0, 0 , 0 };
                        render_texture->mColorSpace = EColorSpace::Linear;
                        render_texture->mFormat = description.mFormat;
                        render_texture->mUsage = ETextureUsage::DynamicWrite;

                        utility::ErrorState error_state;
                        mInitializationMap[stream_type] = render_texture->init(error_state);
                    }

                    if(mInitializationMap[stream_type])
                    {
                        // Update textures on GPU
                        mRenderTextures[stream_type]->update(video_frame.get_data(), mRenderTextures[stream_type]->getDescriptor());
                    }
                }
            }
        }
    }


    void RealSenseRenderFramesComponentInstance::trigger(RealSenseDevice* device, const rs2::frameset &frameset)
    {
        if(mEnabled.load())
        {
            mImplementation->mFrameQueue.enqueue(frameset);
        }
    }


    void RealSenseRenderFramesComponentInstance::clear()
    {
    }


    RenderTexture2D& RealSenseRenderFramesComponentInstance::getRenderTexture(ERealSenseStreamType streamType) const
    {
        assert(mRenderTextures.find(streamType)!=mRenderTextures.end());
        return *mRenderTextures.find(streamType)->second;
    }


    bool RealSenseRenderFramesComponentInstance::isRenderTextureInitialized(ERealSenseStreamType streamType) const
    {
        if(mInitializationMap.find(streamType)!=mInitializationMap.end())
        {
            return mInitializationMap.find(streamType)->second;
        }
        return false;
    }


    void RealSenseRenderFramesComponentInstance::setEnabled(bool enable)
    {
        mEnabled.store(enable);
    }


    bool RealSenseRenderFramesComponentInstance::getEnabled()
    {
        return mEnabled.load();
    }
}
