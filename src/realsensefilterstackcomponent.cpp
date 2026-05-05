#include "realsensefilterstackcomponent.h"
#include "realsenseframesetfilter.h"

#include <rs.hpp>

RTTI_BEGIN_CLASS(nap::RealSenseFilterStackComponent)
        RTTI_PROPERTY("Filters", &nap::RealSenseFilterStackComponent::mFilters, nap::rtti::EPropertyMetaData::Embedded)
        RTTI_PROPERTY("Device", &nap::RealSenseFilterStackComponent::mDevice, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseFilterStackComponentInstance)
        RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    // RealSenseFilterStackComponent
    //////////////////////////////////////////////////////////////////////////


    RealSenseFilterStackComponent::RealSenseFilterStackComponent()
    {
        mInstance = nullptr;
    }


    RealSenseFilterStackComponent::~RealSenseFilterStackComponent(){}


    RealSenseFilterStackComponentInstance* RealSenseFilterStackComponent::getInstance()
    {
        return mInstance;
    }

    //////////////////////////////////////////////////////////////////////////
    // RealSenseFilterStackComponentInstance
    //////////////////////////////////////////////////////////////////////////


    RealSenseFilterStackComponentInstance::RealSenseFilterStackComponentInstance(EntityInstance& entity, Component& resource) :
            RealSenseFrameSetListenerComponentInstance(entity, resource)
    {

    }


    RealSenseFilterStackComponentInstance::~RealSenseFilterStackComponentInstance(){}


    bool RealSenseFilterStackComponentInstance::onInit(utility::ErrorState &errorState)
    {
        mResource = getComponent<RealSenseFilterStackComponent>();
        mResource->mInstance = this;
        mDevice = mResource->mDevice.get();
        mDevice->addFrameSetListener(this);

        for(auto& filter : mResource->mFilters)
        {
            mFilters.emplace_back(filter.get());
        }

        return true;
    }


    void RealSenseFilterStackComponentInstance::destroy()
    {
        mDevice->removeFrameSetListener(this);
    }


    void RealSenseFilterStackComponentInstance::trigger(RealSenseDevice* device, const rs2::frameset &frameset)
    {
        rs2::frameset processed_frameset = frameset;
        for(auto* filter : mFilters)
        {
            processed_frameset = filter->process(device, processed_frameset);
        }

        std::lock_guard l(mMutex);
        for(auto* listener : mFrameSetListeners)
        {
            listener->trigger(device, processed_frameset);
        }
    }


    void RealSenseFilterStackComponentInstance::addFrameSetListener(RealSenseFrameSetListenerComponentInstance* frameSetListener)
    {
        std::lock_guard l(mMutex);
        auto it = std::find(mFrameSetListeners.begin(), mFrameSetListeners.end(), frameSetListener);
        assert(it == mFrameSetListeners.end()); // device already exists
        mFrameSetListeners.emplace_back(frameSetListener);
    }


    void RealSenseFilterStackComponentInstance::removeFrameSetListener(RealSenseFrameSetListenerComponentInstance* frameSetListener)
    {
        std::lock_guard l(mMutex);
        auto it = std::find(mFrameSetListeners.begin(), mFrameSetListeners.end(), frameSetListener);
        assert(it != mFrameSetListeners.end()); // device does not exist
        mFrameSetListeners.erase(it);
    }


    void RealSenseFilterStackComponentInstance::clear()
    {
        for(auto& frameset_listener : mFrameSetListeners)
        {
            frameset_listener->clear();
        }
    }
}
