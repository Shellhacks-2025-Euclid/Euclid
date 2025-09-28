#pragma once
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <vector>    

#include "Euclid_Types.h"  // EuclidObjectID, EuclidShapeType, EuclidTransform (ensure it has CONE, CYLINDER, PRISM, CIRCLE)
#include "Math.h"          // TRS(tf)

namespace Euclid {

struct Object {
    EuclidObjectID id = 0;
    EuclidShapeType type = EUCLID_SHAPE_CUBE;
    EuclidTransform tf{};
    glm::mat4 Model() const { return TRS(tf); }

    int  customIndex = -1;                 // <— index into ObjectStore::mCustom
    glm::vec3 localMin{-0.5f}, localMax{0.5f}; // <— local AABB for picking
};

struct SharedMesh {
    unsigned vao = 0, vbo = 0, ebo = 0;
    int      indexCount = 0;   // for glDrawArrays or glDrawElements
    bool     indexed = false;

    void Release();
};

class ObjectStore {
public:
    // GPU primitives
    void InitPrimitives();
    void ReleasePrimitives();

    // CRUD
    Object* Create(EuclidShapeType t, const void* params, const EuclidTransform& xform, EuclidObjectID id);
    void    DestroyGPU(EuclidObjectID id);  // currently no per-object GPU, kept for future
    void    Clear();
    
    
    const SharedMesh* GetCustomMesh(int customIndex) const {
        if (customIndex < 0 || customIndex >= (int)mCustom.size()) return nullptr;
        return &mCustom[customIndex].mesh;
    }
    // (optional) bounds if you ever need them at render-time
    bool GetCustomBounds(int customIndex, glm::vec3& mn, glm::vec3& mx) const {
        if (customIndex < 0 || customIndex >= (int)mCustom.size()) return false;
        mn = mCustom[customIndex].localMin; mx = mCustom[customIndex].localMax; return true;
    }

    // Access
    Object*       Get(EuclidObjectID id);
    const Object* Get(EuclidObjectID id) const;

    const std::unordered_map<EuclidObjectID, std::unique_ptr<Object>>& All() const { return mObjects; }

    // Selection
    void            SetSelection(EuclidObjectID id) { mSelected = id; }
    EuclidObjectID  GetSelection() const { return mSelected; }

    // Transforms
    EuclidResult GetTransform(EuclidObjectID id, EuclidTransform& out) const;
    EuclidResult SetTransform(EuclidObjectID id, const EuclidTransform& in);

    // Picking (ray in world from screen)
    EuclidObjectID RayPick(float screenX, float screenY,
                           const glm::mat4& invViewProj,
                           int viewportW, int viewportH) const;
    
    EuclidResult LoadOBJ(const char* path, EuclidObjectID* outID, bool normalize);
    EuclidResult CreateFromRawMesh(const float* positions, size_t vertexCount,
                                       const unsigned* indices, size_t indexCount,
                                       EuclidObjectID* outID, bool normalize);
    
    // Mesh routing for drawing
    const SharedMesh& MeshFor(EuclidShapeType t) const;

    // Bounds (local space, unit primitives)
    void ShapeLocalBounds(EuclidShapeType t, glm::vec3& bmin, glm::vec3& bmax) const;

private:
    std::unordered_map<EuclidObjectID, std::unique_ptr<Object>> mObjects;
    EuclidObjectID mSelected = 0;

    // Primitives
    SharedMesh mCube, mPlane, mSphere, mTorus,
               mCone, mCylinder, mPrism, mCircle;

    struct Ray { glm::vec3 o; glm::vec3 d; };
    static Ray  ScreenRay(float x, float y, int w, int h, const glm::mat4& invViewProj);
    static bool IntersectAABB(const Ray& ray, const glm::vec3& bmin, const glm::vec3& bmax, float& tHit);
    
    struct CustomEntry {
        SharedMesh mesh;
        glm::vec3  localMin{-0.5f}, localMax{0.5f};
    };
    std::vector<CustomEntry> mCustom;
};

} // namespace Euclid

