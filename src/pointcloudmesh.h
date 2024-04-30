#pragma once

#include <rtti/rtti.h>
#include <nap/core.h>
#include <mesh.h>
#include <rect.h>
#include <color.h>

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    /**
     * PointCloudMesh
     */
    class NAPAPI PointCloudMesh : public IMesh
    {
    RTTI_ENABLE(IMesh)
    public:
        /**
         * Constructor
         * @param core
         */
        PointCloudMesh(Core& core);

        /**
         * Destructor
         */
        virtual ~PointCloudMesh();

        /**
         * Initializes and uploads the mesh
         * @param errorState contains any error
         * @return true on success
         */
        virtual bool init(utility::ErrorState& errorState) override;

        /**
         * Creates the mesh
         * @param errorState contains any error
         * @return true on success
         */
        bool setup(utility::ErrorState& error);

        /**
         * Returns mesh instance
         * @return mesh instance
         */
        virtual MeshInstance& getMeshInstance() override					{ return *mMeshInstance; }

        /**
         * Returns const mesh instance
         * @return const mesh instance
         */
        virtual const MeshInstance& getMeshInstance() const override		{ return *mMeshInstance; }

        // property: the size of the plane
        int             mRows           = 100; ///< Property: 'Rows' amount of rows
        int             mColums         = 100; ///< Property: 'Columns' amount of columns
        float           mSize           = 1.0f; ///< Property: 'Size' size in units
        EMemoryUsage	mUsage			= EMemoryUsage::Static; ///< Property: 'Usage' memory usage
        ECullMode		mCullMode		= ECullMode::None; ///< Property: 'CullMode' cull mode


        void constructPointCloud(nap::MeshInstance& mesh);
    private:
        EPolygonMode	mPolygonMode	= EPolygonMode::Point;

        RenderService* mRenderService;
        std::unique_ptr<MeshInstance> mMeshInstance;
    };
}

