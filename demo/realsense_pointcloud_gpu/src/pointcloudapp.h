#pragma once

// Core includes
#include <nap/resourcemanager.h>
#include <nap/resourceptr.h>


// Module includes
#include <renderservice.h>
#include <imguiservice.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <scene.h>
#include <renderwindow.h>
#include <entity.h>
#include <app.h>
#include <rendertexture2d.h>
#include <realsensedevice.h>

namespace nap 
{
	using namespace rtti;

    /**
     * Demo application to demonstrate the RealSense module
     * Application renders a pointcloud that is deformed using the PointCloud shader of the RealSense module
     * The PointCloud shader implements the de-projection methods from the realsense SDK, deforming the point cloud mesh completely on the GPU
     */
    class PointCloudApp : public App
	{
    public:
		/**
		 * Constructor
		 */
        PointCloudApp(nap::Core& core) : App(core) {}

        /**
         * Initialize all the services and app specific data structures
		 * @param error contains the error code when initialization fails
		 * @return if initialization succeeded
         */
        bool init(utility::ErrorState& error) override;

		/**
		 * Update is called every frame, before render.
		 * @param deltaTime the time in seconds between calls
		 */
		void update(double deltaTime) override;

        /**
         * Render is called after update. Use this call to render objects to a specific target
         */
        void render() override;

        /**
         * Called when the app receives a window message.
		 * @param windowEvent the window message that occurred
         */
        void windowMessageReceived(WindowEventPtr windowEvent) override;

        /**
         * Called when the app receives an input message (from a mouse, keyboard etc.)
		 * @param inputEvent the input event that occurred
         */
        void inputMessageReceived(InputEventPtr inputEvent) override;

        /**
		 * Called when the app is shutting down after quit() has been invoked
		 * @return the application exit code, this is returned when the main loop is exited
         */
        int shutdown() override;

    private:
        ResourceManager*			mResourceManager = nullptr;		///< Manages all the loaded data
		RenderService*				mRenderService = nullptr;		///< Render Service that handles render calls
		SceneService*				mSceneService = nullptr;		///< Manages all the objects in the scene
		InputService*				mInputService = nullptr;		///< Input service for processing input
		IMGuiService*				mGuiService = nullptr;			///< Manages GUI related update / draw calls
		ObjectPtr<RenderWindow>		mRenderWindow;					///< Pointer to the render window	
		ObjectPtr<Scene>			mScene = nullptr;				///< Pointer to the main scene
		ObjectPtr<EntityInstance>	mCameraEntity = nullptr;		///< Pointer to the entity that holds the perspective camera
        ObjectPtr<RealSenseDevice>  mRealSenseDevice = nullptr;		///< Pointer to the realsense device
        ObjectPtr<RenderTexture2D>  mColorTexture = nullptr;        ///< Pointer to the color render texture
        ObjectPtr<RenderTexture2D>  mDepthTexture = nullptr;        ///< Pointer to the depth render texture
        ObjectPtr<EntityInstance>   mRenderEntity = nullptr;        ///< Pointer to the render entity containing the renderable pointcloud
	};
}
