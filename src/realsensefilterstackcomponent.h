#pragma once

#include "realsenseframesetlistenercomponent.h"

namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    class RealSenseFrameSetFilter;
    class RealSenseFilterStackComponentInstance;
    class RealSenseDevice;

    /**
     * RealSenseRenderFrameComponent
     */
    class NAPAPI RealSenseFilterStackComponent : public RealSenseFrameSetListenerComponent
    {
        friend class RealSenseFilterStackComponentInstance;

    RTTI_ENABLE(RealSenseFrameSetListenerComponent)
    DECLARE_COMPONENT(RealSenseFilterStackComponent, RealSenseFilterStackComponentInstance)
    public:
        /**
         * Constructor
         */
        RealSenseFilterStackComponent();

        /**
         * Destructor
         */
        virtual ~RealSenseFilterStackComponent();

        /**
         * Returns component instance, nullptr if not inited
         * @return component instance, nullptr if not inited
         */
        RealSenseFilterStackComponentInstance* getInstance();

        std::vector<ResourcePtr<RealSenseFrameSetFilter>> mFilters;
        ResourcePtr<RealSenseDevice> mDevice;
    private:
        RealSenseFilterStackComponentInstance* mInstance;
    };

    /**
     * RealSenseFilterStackComponentInstance
     */
    class NAPAPI RealSenseFilterStackComponentInstance : public RealSenseFrameSetListenerComponentInstance
    {
    RTTI_ENABLE(RealSenseFrameSetListenerComponentInstance)
    public:
        /**
         * Constructor
         * @param entity
         * @param resource
         */
        RealSenseFilterStackComponentInstance(EntityInstance& entity, Component& resource);

        /**
         * Destructor
         */
        virtual ~RealSenseFilterStackComponentInstance();

        /**
         * Adds a frame set listener
         * @param framesetListener
         */
        void addFrameSetListener(RealSenseFrameSetListenerComponentInstance* framesetListener);

        /**
         * Removes a frame set listener
         * @param framesetListener
         */
        void removeFrameSetListener(RealSenseFrameSetListenerComponentInstance* framesetListener);

        /**
         * Called from RealSense processing thread
         * @param frame
         */
        virtual void trigger(RealSenseDevice* device, const rs2::frameset& frameset) override;

        /**
         * Removes all frame set listeners
         */
        void clear() override;
    protected:
        /**
         * internal initialization method called from init
         * Creates initial 1x1 render texture
         * @param errorState contains any errors
         * @return true on success
         */
        bool onInit(utility::ErrorState& errorState) override;

        /**
         * Called before deconstruction
         */
        void destroy() override;
    private:
        std::vector<RealSenseFrameSetListenerComponentInstance*> mFrameSetListeners;
        RealSenseFilterStackComponent* mResource;
        std::vector<RealSenseFrameSetFilter*> mFilters;
        std::mutex mMutex;
        RealSenseDevice* mDevice;
    };
}
