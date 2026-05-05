#include <renderservice.h>
#include <renderglobals.h>

#include "pointcloudmesh.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PointCloudMesh)
    RTTI_CONSTRUCTOR(nap::Core&)
    RTTI_PROPERTY("Rows", &nap::PointCloudMesh::mRows, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Colums", &nap::PointCloudMesh::mColums, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Size", &nap::PointCloudMesh::mSize, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Usage", &nap::PointCloudMesh::mUsage, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("CullMode", &nap::PointCloudMesh::mCullMode, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
    PointCloudMesh::PointCloudMesh(Core& core) : mRenderService(core.getService<RenderService>()) {}

    PointCloudMesh::~PointCloudMesh(){}

    bool PointCloudMesh::init(utility::ErrorState& errorState)
    {
        // Setup plane
        if (!setup(errorState))
            return false;

        // Initialize instance
        return mMeshInstance->init(errorState);
    }

    bool PointCloudMesh::setup(utility::ErrorState& error)
    {
        assert(mRenderService != nullptr);
        mMeshInstance = std::make_unique<MeshInstance>(*mRenderService);

        constructPointCloud(*mMeshInstance);

        return true;
    }


    void PointCloudMesh::constructPointCloud(nap::MeshInstance& mesh)
    {
        size_t vert_count = mRows * mColums;

        // Get incremental stepping values
        std::vector<glm::vec3> vertices(vert_count, {0.0f, 0.0f, 0.0f});
        std::vector<glm::vec3> normals(vert_count, {0.0f, 0.0f, 1.0f});
        std::vector<glm::vec3> uvs(vert_count, {0.0f, 0.0f, 0.0f});

        Vec3VertexAttribute& position_attribute = mesh.getOrCreateAttribute<glm::vec3>(vertexid::position);
        Vec3VertexAttribute& normal_attribute = mesh.getOrCreateAttribute<glm::vec3>(vertexid::normal);
        Vec3VertexAttribute& uv_attribute = mesh.getOrCreateAttribute<glm::vec3>(vertexid::getUVName(0));
        Vec4VertexAttribute& color_attribute = mesh.getOrCreateAttribute<glm::vec4>(vertexid::getColorName(0));

        // Set the number of vertices to use
        mesh.setNumVertices(vert_count);
        mesh.setDrawMode(EDrawMode::Points);
        mesh.setUsage(mUsage);
        mesh.setCullMode(mCullMode);
        mesh.setPolygonMode(mPolygonMode);

        // Push vertex data
        size_t width = mColums;
        size_t height = mRows;
        for(size_t y = 0; y < height; y++)
        {
            for(size_t x = 0; x < width; x++)
            {
                size_t idx = y * width + x;
                float x_part = static_cast<float>(x) / static_cast<float>(width);
                float y_part = static_cast<float>(y) / static_cast<float>(height);
                uvs[idx] = { x_part, y_part, 0.0f };

                float r = 0.0f;
                if(width >= height)
                {
                    r = (static_cast<float>(height) / static_cast<float>(width)) * mSize;
                }else
                {
                    r = (static_cast<float>(width) / static_cast<float>(height)) * mSize;
                }
                vertices[idx] = { x_part * r, y_part * mSize, 0.0f };
            }
        }

        std::vector<uint32> indices(vert_count, 0);
        for (size_t i = 0; i < vert_count; i++)
            indices[i] = i;

        position_attribute.setData(vertices.data(), vert_count);
        normal_attribute.setData(normals.data(), vert_count);
        uv_attribute.setData(uvs.data(), vert_count);
        color_attribute.setData({vert_count, {1, 1, 1, 1}});

        MeshShape& shape = mesh.createShape();
        shape.setIndices(indices.data(), indices.size());
    }
}
