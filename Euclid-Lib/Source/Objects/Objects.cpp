#include "Objects.hpp"
#include <glad/glad.h>

namespace Euclid {

// -------- SharedMesh --------
void SharedMesh::Release() {
    if (ebo) { glDeleteBuffers(1, &ebo); ebo = 0; }
    if (vbo) { glDeleteBuffers(1, &vbo); vbo = 0; }
    if (vao) { glDeleteVertexArrays(1, &vao); vao = 0; }
}

// -------- ObjectStore: primitives --------
void ObjectStore::InitPrimitives() {
    // CUBE: 36-vertex, position+color like your existing cube (colors are placeholders)
    struct V { float p[3]; float c[3]; };
    const V cubeVerts[] = {
        // back
        {{-0.5f,-0.5f,-0.5f},{1,0,0}}, {{0.5f,-0.5f,-0.5f},{0,1,0}}, {{0.5f,0.5f,-0.5f},{0,0,1}},
        {{0.5f,0.5f,-0.5f},{0,0,1}}, {{-0.5f,0.5f,-0.5f},{1,1,0}}, {{-0.5f,-0.5f,-0.5f},{1,0,0}},
        // front
        {{-0.5f,-0.5f, 0.5f},{1,0,1}}, {{0.5f,-0.5f, 0.5f},{0,1,1}}, {{0.5f,0.5f, 0.5f},{1,1,1}},
        {{0.5f,0.5f, 0.5f},{1,1,1}}, {{-0.5f,0.5f, 0.5f},{.5,.5,.5}}, {{-0.5f,-0.5f, 0.5f},{1,0,1}},
        // left
        {{-0.5f, 0.5f, 0.5f},{0,1,.5}}, {{-0.5f, 0.5f,-0.5f},{0,.5,1}}, {{-0.5f,-0.5f,-0.5f},{1,.5,0}},
        {{-0.5f,-0.5f,-0.5f},{1,.5,0}}, {{-0.5f,-0.5f, 0.5f},{.5,1,0}}, {{-0.5f, 0.5f, 0.5f},{0,1,.5}},
        // right
        {{0.5f, 0.5f, 0.5f},{1,0,.5}}, {{0.5f, 0.5f,-0.5f},{.5,0,1}}, {{0.5f,-0.5f,-0.5f},{0,.5,.5}},
        {{0.5f,-0.5f,-0.5f},{0,.5,.5}}, {{0.5f,-0.5f, 0.5f},{.5,1,.5}}, {{0.5f, 0.5f, 0.5f},{1,0,.5}},
        // bottom
        {{-0.5f,-0.5f,-0.5f},{.3,.7,.5}}, {{0.5f,-0.5f,-0.5f},{.7,.3,.5}}, {{0.5f,-0.5f,0.5f},{.5,.7,.3}},
        {{0.5f,-0.5f,0.5f},{.5,.7,.3}}, {{-0.5f,-0.5f,0.5f},{.3,.5,.7}}, {{-0.5f,-0.5f,-0.5f},{.3,.7,.5}},
        // top
        {{-0.5f,0.5f,-0.5f},{.7,.5,.3}}, {{0.5f,0.5f,-0.5f},{.5,.3,.7}}, {{0.5f,0.5f,0.5f},{.7,.7,.7}},
        {{0.5f,0.5f,0.5f},{.7,.7,.7}}, {{-0.5f,0.5f,0.5f},{.2,.8,.4}}, {{-0.5f,0.5f,-0.5f},{.7,.5,.3}},
    };

    glGenVertexArrays(1, &mCube.vao);
    glGenBuffers(1, &mCube.vbo);
    glBindVertexArray(mCube.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mCube.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(V), (void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(V), (void*)(3 * sizeof(float)));
    mCube.indexCount = 36; mCube.indexed = false;
    glBindVertexArray(0);

    // PLANE: 1x1 in XZ
    const V planeVerts[] = {
        {{-0.5f,0,-0.5f},{1,1,1}}, {{0.5f,0,-0.5f},{1,1,1}}, {{0.5f,0,0.5f},{1,1,1}},
        {{0.5f,0,0.5f},{1,1,1}}, {{-0.5f,0,0.5f},{1,1,1}}, {{-0.5f,0,-0.5f},{1,1,1}},
    };
    glGenVertexArrays(1, &mPlane.vao);
    glGenBuffers(1, &mPlane.vbo);
    glBindVertexArray(mPlane.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mPlane.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVerts), planeVerts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(V), (void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(V), (void*)(3 * sizeof(float)));
    mPlane.indexCount = 6; mPlane.indexed = false;
    glBindVertexArray(0);

    // For now, sphere/torus: placeholders using cube mesh (safe to replace later)
    mSphere = mCube;
    mTorus  = mCube;
}

void ObjectStore::ReleasePrimitives() {
    mCube.Release();
    // mSphere/mTorus reuse cube buffers (no duplicate deletion)
    mPlane.Release();
}

// -------- CRUD --------
Object* ObjectStore::Create(EuclidShapeType t, const void* /*params*/,
                            const EuclidTransform& xform, EuclidObjectID id) {
    auto obj = std::make_unique<Object>();
    obj->id = id;
    obj->type = t;
    obj->tf = xform;
    for (int i = 0; i < 3; ++i) if (obj->tf.scale[i] == 0.0f) obj->tf.scale[i] = 1.0f;
    auto* raw = obj.get();
    mObjects.emplace(id, std::move(obj));
    return raw;
}

void ObjectStore::DestroyGPU(EuclidObjectID /*id*/) {
    // shared meshes only; nothing per-object yet
}

void ObjectStore::Clear() {
    mObjects.clear();
    mSelected = 0;
}

// -------- Access --------
Object* ObjectStore::Get(EuclidObjectID id) {
    auto it = mObjects.find(id); return (it == mObjects.end()) ? nullptr : it->second.get();
}
const Object* ObjectStore::Get(EuclidObjectID id) const {
    auto it = mObjects.find(id); return (it == mObjects.end()) ? nullptr : it->second.get();
}

// -------- Transforms --------
EuclidResult ObjectStore::GetTransform(EuclidObjectID id, EuclidTransform& out) const {
    auto it = mObjects.find(id); if (it == mObjects.end()) return EUCLID_ERR_BAD_PARAM;
    out = it->second->tf; return EUCLID_OK;
}
EuclidResult ObjectStore::SetTransform(EuclidObjectID id, const EuclidTransform& in) {
    auto it = mObjects.find(id); if (it == mObjects.end()) return EUCLID_ERR_BAD_PARAM;
    it->second->tf = in; return EUCLID_OK;
}

// -------- Bounds --------
void ObjectStore::ShapeLocalBounds(EuclidShapeType t, glm::vec3& bmin, glm::vec3& bmax) const {
    switch (t) {
        case EUCLID_SHAPE_CUBE:   bmin={-0.5f,-0.5f,-0.5f}; bmax={0.5f,0.5f,0.5f}; break;
        case EUCLID_SHAPE_PLANE:  bmin={-0.5f, 0.0f,-0.5f}; bmax={0.5f,0.0f,0.5f}; break;
        case EUCLID_SHAPE_SPHERE: bmin={-0.5f,-0.5f,-0.5f}; bmax={0.5f,0.5f,0.5f}; break; // unit sphere bounds
        case EUCLID_SHAPE_TORUS:  bmin={-0.7f,-0.3f,-0.7f}; bmax={0.7f,0.3f,0.7f}; break; // rough
    }
}

// -------- Mesh routing --------
const SharedMesh& ObjectStore::MeshFor(EuclidShapeType t) const {
    switch (t) {
        case EUCLID_SHAPE_CUBE:   return mCube;
        case EUCLID_SHAPE_PLANE:  return mPlane;
        case EUCLID_SHAPE_SPHERE: return mSphere;
        case EUCLID_SHAPE_TORUS:  return mTorus;
    }
    return mCube; // fallback
}

// -------- Picking helpers --------
ObjectStore::Ray ObjectStore::ScreenRay(float x, float y, int w, int h, const glm::mat4& invViewProj) {
    float sx =  (2.0f * float(x) / float(w)) - 1.0f;
    float sy = -(2.0f * float(y) / float(h)) + 1.0f;
    glm::vec4 pNear = invViewProj * glm::vec4(sx, sy, -1.0f, 1.0f);
    glm::vec4 pFar  = invViewProj * glm::vec4(sx, sy,  1.0f, 1.0f);
    pNear /= pNear.w; pFar /= pFar.w;
    glm::vec3 o = glm::vec3(pNear);
    glm::vec3 d = glm::normalize(glm::vec3(pFar - pNear));
    return { o, d };
}

bool ObjectStore::IntersectAABB(const Ray& ray, const glm::vec3& bmin, const glm::vec3& bmax, float& tHit) {
    glm::vec3 invD = 1.0f / ray.d;
    glm::vec3 t0s = (bmin - ray.o) * invD;
    glm::vec3 t1s = (bmax - ray.o) * invD;
    glm::vec3 tsm = glm::min(t0s, t1s);
    glm::vec3 tbg = glm::max(t0s, t1s);
    float tmin = glm::max(glm::max(tsm.x, tsm.y), tsm.z);
    float tmax = glm::min(glm::min(tbg.x, tbg.y), tbg.z);
    if (tmax >= glm::max(0.0f, tmin)) { tHit = tmin > 0 ? tmin : tmax; return true; }
    return false;
}

// -------- RayPick --------
EuclidObjectID ObjectStore::RayPick(float screenX, float screenY,
                                    const glm::mat4& invViewProj,
                                    int viewportW, int viewportH) const {
    if (viewportW <= 0 || viewportH <= 0) return 0;
    Ray ray = ScreenRay(screenX, screenY, viewportW, viewportH, invViewProj);

    EuclidObjectID best = 0;
    float bestT = 1e30f;

    for (auto& kv : mObjects) {
        const Object& o = *kv.second;
        glm::vec3 bminL, bmaxL; ShapeLocalBounds(o.type, bminL, bmaxL);
        glm::mat4 M = o.Model();

        // Transform local AABB to world AABB via 8 corners
        glm::vec3 corners[8] = {
            {bminL.x,bminL.y,bminL.z},{bmaxL.x,bminL.y,bminL.z},{bminL.x,bmaxL.y,bminL.z},{bmaxL.x,bmaxL.y,bminL.z},
            {bminL.x,bminL.y,bmaxL.z},{bmaxL.x,bminL.y,bmaxL.z},{bminL.x,bmaxL.y,bmaxL.z},{bmaxL.x,bmaxL.y,bmaxL.z}
        };
        glm::vec3 bminW( 1e9f), bmaxW(-1e9f);
        for (auto& c : corners) {
            glm::vec3 w = glm::vec3(M * glm::vec4(c, 1));
            bminW = glm::min(bminW, w);
            bmaxW = glm::max(bmaxW, w);
        }

        float t;
        if (IntersectAABB(ray, bminW, bmaxW, t) && t < bestT) { bestT = t; best = o.id; }
    }
    return best;
}

} // namespace Euclid
