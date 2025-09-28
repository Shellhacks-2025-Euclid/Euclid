#pragma once
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

#include "Euclid_Types.h"  // EuclidObjectID, EuclidShapeType, EuclidTransform (ensure it has CONE, CYLINDER, PRISM, CIRCLE)
#include "Math.h"          // TRS(tf)

namespace Euclid {

struct Object {
    EuclidObjectID id = 0;
    EuclidShapeType type = EUCLID_SHAPE_CUBE;
    EuclidTransform tf{}; // position/rotation/scale
    glm::mat4 Model() const { return TRS(tf); }
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
};

} // namespace Euclid

