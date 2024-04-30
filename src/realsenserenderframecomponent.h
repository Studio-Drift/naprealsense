#pragma once

#include "realsenseframesetlistenercomponent.h"

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    // forward declares
    class RealSenseRenderFrameComponentInstance;
    class RealSenseFrameFilter;

    /**
     * RealSenseRenderFrameComponent component of a RealSenseRenderFrameComponentInstance
     * A RealSenseRenderFrameComponentInstance renders a frame from a frameset into a render texture
     */
    class NAPAPI RealSenseRenderFrameComponent : public RealSenseFrameSetListenerComponent
    {
    RTTI_ENABLE(RealSenseFrameSetListenerComponent)
    DECLARE_COMPONENT(RealSenseRenderFrameComponent, RealSenseRenderFrameComponentInstance)
    public:
        /**
         * Constructor
         */
        RealSenseRenderFrameComponent();

        /**
         * Destructor
         */
        virtual ~RealSenseRenderFrameComponent();

        // Properties
        ERealSenseStreamType mStreamType = ERealSenseStreamType::REALSENSE_STREAMTYPE_COLOR; ///< Property: 'StreamType' stream type of the stream to render
        RenderTexture2D::EFormat mFormat = RenderTexture2D::EFormat::RGBA8; ///< Property: 'Format' the render texture format
        std::vector<ResourcePtr<RealSenseFrameFilter>> mFilters; ///< Property: 'Filter' the filters to apply to the frame before rendering
    };

    /**
     * RealSenseRenderFrameComponentInstance renders a frame from a frameset into a render texture
     */
    class NAPAPI RealSenseRenderFrameComponentInstance : public RealSenseFrameSetListenerComponentInstance
    {
    RTTI_ENABLE(RealSenseFrameSetListenerComponentInstance)
    public:
        /**
         * Constructor
         * @param entity reference to entity instance
         * @param resource reference to component
         */
        RealSenseRenderFrameComponentInstance(EntityInstance& entity, Component& resource);

        /**
         * Destructor
         */
        virtual ~RealSenseRenderFrameComponentInstance();

        /**
         * Returns reference to render texture
         * @return reference to render texture
         */
        RenderTexture2D& getRenderTexture() const{ return *mRenderTexture; }

        /**
         * Returns true if render texture is initialized
         * @return true if render texture is initialized
         */
        bool isRenderTextureInitialized() const{ return mTextureInitialized; }

        /**
         * Called upon receiving a new frameset, called from RealSense device
         * @param frameset a new frameset
         */
         void trigger(nap::RealSenseDevice *device, const rs2::frameset &frameset) override;

    protected:
        /**
         * Internal init method
         * @param errorState contains any errors
         * @return true on success
         */
        bool onInit(utility::ErrorState& errorState) override;

        /**
         * Called before destruction
         */
        void destroy() override;

        /**
         * Update method
         * @param deltaTime time since last update
         */
        void update(double deltaTime) override;


    private:
        std::unique_ptr<RenderTexture2D> mRenderTexture;
        RealSenseRenderFrameComponent* mResource;
        ERealSenseStreamType mStreamType;
        RenderTexture2D::EFormat mFormat;
        bool mTextureInitialized = false;

        struct Impl;
        std::unique_ptr<Impl> mImplementation;

        std::vector<RealSenseFrameFilter*> mFilters;
    };
}
