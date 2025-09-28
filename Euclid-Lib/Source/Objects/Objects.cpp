#include "Objects.hpp"
#include <glad/glad.h>

#include <vector>
#include <algorithm>
#include <cmath>

namespace Euclid {

// -------- SharedMesh --------
void SharedMesh::Release() {
    if (ebo) { glDeleteBuffers(1, &ebo); ebo = 0; }
    if (vbo) { glDeleteBuffers(1, &vbo); vbo = 0; }
    if (vao) { glDeleteVertexArrays(1, &vao); vao = 0; }
}

static inline void MulScale(EuclidTransform& tf, float sx, float sy, float sz) {
    tf.scale[0] *= sx; tf.scale[1] *= sy; tf.scale[2] *= sz;
}

// Map per-shape params to transform scale for unit meshes.
// Assumes unit sizes built as in your meshes:
//  cube edge=1, sphere radius=0.5, plane 1x1 (XZ), cone r=0.5 h=1, cyl r=0.5 h=1,
//  prism radius~0.5 (equilateral tri), circle radius=0.5,
//  torus base (R=0.5, r=0.2) -> extents XZ ~ (R+r)=0.7, Y ~ (2r)=0.4.
static void ApplyParamsToScale(EuclidShapeType t, const void* params, EuclidTransform& tf) {
    if (!params) return;
    switch (t) {
        case EUCLID_SHAPE_CUBE: {
            const EuclidCubeParams& p = *reinterpret_cast<const EuclidCubeParams*>(params);
            MulScale(tf, p.size, p.size, p.size);
        } break;
        case EUCLID_SHAPE_SPHERE: {
            const EuclidSphereParams& p = *reinterpret_cast<const EuclidSphereParams*>(params);
            float s = (p.radius > 0.f) ? (p.radius / 0.5f) : 1.f; // base radius 0.5
            MulScale(tf, s, s, s);
        } break;
        case EUCLID_SHAPE_PLANE: {
            const EuclidPlaneParams& p = *reinterpret_cast<const EuclidPlaneParams*>(params);
            MulScale(tf, p.width, 1.f, p.height);
        } break;
        case EUCLID_SHAPE_CONE: {
            const EuclidConeParams& p = *reinterpret_cast<const EuclidConeParams*>(params);
            float sR = (p.radius > 0.f) ? (p.radius / 0.5f) : 1.f; // base r=0.5
            float sH = (p.height > 0.f) ? (p.height / 1.f) : 1.f;  // base h=1
            MulScale(tf, 2.f*sR*0.5f, sH, 2.f*sR*0.5f); // simplify to (sR, sH, sR)
            // equivalently: MulScale(tf, sR, sH, sR);
        } break;
        case EUCLID_SHAPE_CYLINDER: {
            const EuclidCylinderParams& p = *reinterpret_cast<const EuclidCylinderParams*>(params);
            float sR = (p.radius > 0.f) ? (p.radius / 0.5f) : 1.f;
            float sH = (p.height > 0.f) ? (p.height / 1.f) : 1.f;
            MulScale(tf, sR, sH, sR);
        } break;
        case EUCLID_SHAPE_PRISM: {
            const EuclidPrismParams& p = *reinterpret_cast<const EuclidPrismParams*>(params);
            float sR = (p.radius > 0.f) ? (p.radius / 0.5f) : 1.f; // our built tri radius~0.5
            float sH = (p.height > 0.f) ? (p.height / 1.f) : 1.f;
            MulScale(tf, sR, sH, sR);
            // NOTE: 'sides' is ignored with current fixed 3-sided mesh.
        } break;
        case EUCLID_SHAPE_CIRCLE: {
            const EuclidCircleParams& p = *reinterpret_cast<const EuclidCircleParams*>(params);
            float s = (p.radius > 0.f) ? (p.radius / 0.5f) : 1.f; // base r=0.5
            MulScale(tf, s, 1.f, s);
        } break;
        case EUCLID_SHAPE_TORUS: {
            const EuclidTorusParams& p = *reinterpret_cast<const EuclidTorusParams*>(params);
            // Approximate: scale XZ to match (R+r), scale Y to match r.
            float sxz = (p.majorRadius + p.minorRadius) / 0.7f; // base (R+r)=0.7
            float sy  = (p.minorRadius > 0.f) ? (p.minorRadius / 0.2f) : 1.f; // base r=0.2
            MulScale(tf, sxz, sy, sxz);
        } break;
        default: break;
    }
}



namespace {
    struct V { float p[3]; float c[3]; };
    constexpr float PI = 3.14159265358979323846f;

    inline V VC(float x, float y, float z, float r, float g, float b) {
        return {{x,y,z},{r,g,b}};
    }

    inline void UploadMesh(SharedMesh& dst,
                           const std::vector<V>& verts,
                           const std::vector<unsigned>& idx)
    {
        glGenVertexArrays(1, &dst.vao);
        glGenBuffers(1, &dst.vbo);

        glBindVertexArray(dst.vao);
        glBindBuffer(GL_ARRAY_BUFFER, dst.vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(V), verts.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(V), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(V), (void*)(3*sizeof(float)));

        if (!idx.empty()) {
            glGenBuffers(1, &dst.ebo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dst.ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size()*sizeof(unsigned), idx.data(), GL_STATIC_DRAW);
            dst.indexCount = (GLsizei)idx.size();
            dst.indexed = true;
        } else {
            dst.indexCount = (GLsizei)verts.size();
            dst.indexed = false;
        }

        glBindVertexArray(0);
    }

    // ---------- Primitive builders ----------

    // Non-indexed cube (same layout/colors you used)
    void BuildCube(SharedMesh& out) {
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
        std::vector<V> verts(std::begin(cubeVerts), std::end(cubeVerts));
        std::vector<unsigned> idx; // none
        UploadMesh(out, verts, idx);
    }

    // Non-indexed plane (1x1 in XZ at y=0)
    void BuildPlane(SharedMesh& out) {
        const V planeVerts[] = {
            {{-0.5f,0,-0.5f},{1,1,1}}, {{0.5f,0,-0.5f},{1,1,1}}, {{0.5f,0,0.5f},{1,1,1}},
            {{0.5f,0,0.5f},{1,1,1}}, {{-0.5f,0,0.5f},{1,1,1}}, {{-0.5f,0,-0.5f},{1,1,1}},
        };
        std::vector<V> verts(std::begin(planeVerts), std::end(planeVerts));
        std::vector<unsigned> idx; // none
        UploadMesh(out, verts, idx);
    }

    // Sphere (lat/long)
    void BuildSphere(SharedMesh& out, int stacks=24, int slices=36, float R=0.5f) {
        std::vector<V> v; v.reserve((stacks+1)*(slices+1));
        std::vector<unsigned> idx; idx.reserve(stacks*slices*6);

        for (int i=0;i<=stacks;i++){
            float t  = float(i)/stacks;
            float th = t*PI;               // [0..PI]
            float y  = R*std::cos(th);
            float r  = R*std::sin(th);
            for (int j=0;j<=slices;j++){
                float s  = float(j)/slices;
                float ph = s*2*PI;         // [0..2PI]
                float x  = r*std::cos(ph);
                float z  = r*std::sin(ph);
                float cr = 0.5f + 0.5f*(x/R);
                float cg = 0.5f + 0.5f*(y/R);
                float cb = 0.5f + 0.5f*(z/R);
                v.push_back(VC(x,y,z, cr,cg,cb));
            }
        }
        int stride = slices+1;
        for (int i=0;i<stacks;i++){
            for (int j=0;j<slices;j++){
                unsigned a = i*stride + j;
                unsigned b = a + stride;
                idx.push_back(a); idx.push_back(b); idx.push_back(a+1);
                idx.push_back(b); idx.push_back(b+1); idx.push_back(a+1);
            }
        }
        UploadMesh(out, v, idx);
    }

    // Torus
    void BuildTorus(SharedMesh& out, int segU=48, int segV=24, float R=0.5f, float r=0.2f) {
        std::vector<V> v; v.reserve((segU+1)*(segV+1));
        std::vector<unsigned> idx; idx.reserve(segU*segV*6);

        for (int i=0;i<=segU;i++){
            float u = (float)i/segU * 2*PI;
            float cu = std::cos(u), su = std::sin(u);
            for (int j=0;j<=segV;j++){
                float vv = (float)j/segV * 2*PI;
                float cv = std::cos(vv), sv = std::sin(vv);
                float x = (R + r*cv)*cu;
                float y = r*sv;
                float z = (R + r*cv)*su;
                float cr = 0.5f+0.5f*cv;
                float cg = 0.5f+0.5f*sv;
                float cb = 0.5f+0.5f*cu;
                v.push_back(VC(x,y,z, cr,cg,cb));
            }
        }
        int stride = segV+1;
        for (int i=0;i<segU;i++){
            for (int j=0;j<segV;j++){
                unsigned a = i*stride + j;
                unsigned b = a + stride;
                idx.push_back(a); idx.push_back(b); idx.push_back(a+1);
                idx.push_back(b); idx.push_back(b+1); idx.push_back(a+1);
            }
        }
        UploadMesh(out, v, idx);
    }

    // Cone (base at y=-h/2, apex at y=+h/2)
    void BuildCone(SharedMesh& out, int seg=32, float radius=0.5f, float h=1.0f) {
        float y0 = -0.5f*h, y1 = 0.5f*h;
        std::vector<V> v; v.reserve(seg + 1 + 1);
        std::vector<unsigned> idx;

        for (int i=0;i<seg;i++){
            float a = (float)i/seg * 2*PI;
            float x = radius*std::cos(a);
            float z = radius*std::sin(a);
            v.push_back(VC(x,y0,z, 0.9f,0.6f,0.2f));
        }
        unsigned baseCenter = (unsigned)v.size();
        v.push_back(VC(0,y0,0, 0.8f,0.5f,0.2f));

        unsigned apex = (unsigned)v.size();
        v.push_back(VC(0,y1,0, 0.95f,0.35f,0.35f));

        // base fan
        for (int i=0;i<seg;i++){
            unsigned a = (unsigned)i;
            unsigned b = (unsigned)((i+1)%seg);
            idx.push_back(baseCenter); idx.push_back(b); idx.push_back(a);
        }
        // sides
        for (int i=0;i<seg;i++){
            unsigned a = (unsigned)i;
            unsigned b = (unsigned)((i+1)%seg);
            idx.push_back(a); idx.push_back(b); idx.push_back(apex);
        }
        UploadMesh(out, v, idx);
    }

    // Cylinder (axis Y, height h, radius r)
    void BuildCylinder(SharedMesh& out, int seg=32, float radius=0.5f, float h=1.0f) {
        float y0 = -0.5f*h, y1 = 0.5f*h;
        std::vector<V> v; v.reserve(2*seg + 2);
        std::vector<unsigned> idx;

        // bottom ring
        for (int i=0;i<seg;i++){
            float a = (float)i/seg * 2*PI;
            float x = radius*std::cos(a);
            float z = radius*std::sin(a);
            v.push_back(VC(x,y0,z, 0.7f,0.7f,0.9f));
        }
        unsigned bottomCenter = (unsigned)v.size();
        v.push_back(VC(0,y0,0, 0.6f,0.6f,0.9f));

        // top ring
        unsigned topStart = (unsigned)v.size();
        for (int i=0;i<seg;i++){
            float a = (float)i/seg * 2*PI;
            float x = radius*std::cos(a);
            float z = radius*std::sin(a);
            v.push_back(VC(x,y1,z, 0.7f,0.9f,0.7f));
        }
        unsigned topCenter = (unsigned)v.size();
        v.push_back(VC(0,y1,0, 0.6f,0.9f,0.6f));

        // caps
        for (int i=0;i<seg;i++){
            unsigned a = (unsigned)i;
            unsigned b = (unsigned)((i+1)%seg);
            // bottom fan
            idx.push_back(bottomCenter); idx.push_back(b); idx.push_back(a);
            // top fan
            unsigned ta = topStart + a;
            unsigned tb = topStart + b;
            idx.push_back(topCenter); idx.push_back(ta); idx.push_back(tb);
        }

        // sides
        for (int i=0;i<seg;i++){
            unsigned a0 = (unsigned)i;
            unsigned b0 = (unsigned)((i+1)%seg);
            unsigned a1 = topStart + i;
            unsigned b1 = topStart + ((i+1)%seg);
            idx.push_back(a0); idx.push_back(b0); idx.push_back(a1);
            idx.push_back(b0); idx.push_back(b1); idx.push_back(a1);
        }
        UploadMesh(out, v, idx);
    }

    // Triangular prism (equilateral XZ, height along Y)
    void BuildTriPrism(SharedMesh& out, float height=1.0f, float radius=0.5f) {
        float y0 = -0.5f*height, y1 = 0.5f*height;
        std::vector<V> v; v.reserve(6);
        std::vector<unsigned> idx;

        for (int k=0;k<3;k++){
            float a = (PI/2.0f) + k*(2*PI/3.0f);
            float x = radius*std::cos(a);
            float z = radius*std::sin(a);
            v.push_back(VC(x,y0,z, 0.9f,0.9f,0.3f)); // bottom
        }
        for (int k=0;k<3;k++){
            float a = (PI/2.0f) + k*(2*PI/3.0f);
            float x = radius*std::cos(a);
            float z = radius*std::sin(a);
            v.push_back(VC(x,y1,z, 0.9f,0.6f,0.3f)); // top
        }

        // caps
        idx.push_back(0); idx.push_back(2); idx.push_back(1);     // bottom
        idx.push_back(3); idx.push_back(4); idx.push_back(5);     // top

        // sides (three quads -> two tris each)
        auto quad = [&](unsigned a, unsigned b, unsigned c, unsigned d){
            idx.push_back(a); idx.push_back(b); idx.push_back(c);
            idx.push_back(a); idx.push_back(c); idx.push_back(d);
        };
        quad(0,1,4,3);
        quad(1,2,5,4);
        quad(2,0,3,5);

        UploadMesh(out, v, idx);
    }

    // Circle (filled disc) in XZ at y=0
    void BuildCircle(SharedMesh& out, int seg=64, float radius=0.5f) {
        std::vector<V> v; v.reserve(seg+1);
        std::vector<unsigned> idx; idx.reserve(seg*3);
        v.push_back(VC(0,0,0, 0.95f,0.95f,0.95f)); // center
        for (int i=0;i<seg;i++){
            float a = (float)i/seg * 2*PI;
            float x = radius*std::cos(a);
            float z = radius*std::sin(a);
            float cr = 0.5f+0.5f*std::cos(a);
            float cg = 0.5f+0.5f*std::sin(a);
            v.push_back(VC(x,0,z, cr,cg,0.9f));
        }
        for (int i=0;i<seg;i++){
            unsigned a = 1 + (unsigned)i;
            unsigned b = 1 + (unsigned)((i+1)%seg);
            idx.push_back(0); idx.push_back(a); idx.push_back(b);
        }
        UploadMesh(out, v, idx);
    }
} // anon

// === Helpers (put next to UploadMesh in the same anonymous namespace) ===
static void ComputeAABB(const std::vector<V>& verts, glm::vec3& bmin, glm::vec3& bmax) {
    glm::vec3 mn( 1e9f), mx(-1e9f);
    for (auto& v : verts) {
        mn.x = std::min(mn.x, v.p[0]); mn.y = std::min(mn.y, v.p[1]); mn.z = std::min(mn.z, v.p[2]);
        mx.x = std::max(mx.x, v.p[0]); mx.y = std::max(mx.y, v.p[1]); mx.z = std::max(mx.z, v.p[2]);
    }
    bmin = mn; bmax = mx;
}

static void NormalizeToUnit(std::vector<V>& verts) {
    glm::vec3 mn, mx; ComputeAABB(verts, mn, mx);
    glm::vec3 size = mx - mn;
    float maxDim = std::max(size.x, std::max(size.y, size.z));
    if (maxDim <= 0.f) return;
    glm::vec3 center = 0.5f * (mn + mx);
    float s = 1.0f / maxDim;
    for (auto& v : verts) {
        v.p[0] = (v.p[0] - center.x) * s;
        v.p[1] = (v.p[1] - center.y) * s;
        v.p[2] = (v.p[2] - center.z) * s;
    }
}

// tiny OBJ reader: v, vn, f (triangulates fan)
struct Idx { int v=-1, vn=-1; };
static bool ParseOBJ(const char* path, std::vector<V>& outVerts, std::vector<unsigned>& outIdx) {
    FILE* fp = fopen(path, "rb");
    if (!fp) return false;

    std::vector<glm::vec3> pos, nrm;
    std::vector<Idx> face;
    char line[1024];

    auto emitTri = [&](const Idx& a, const Idx& b, const Idx& c){
        auto push = [&](const Idx& id)->unsigned{
            glm::vec3 P(0), N(0);
            if (id.v  >=0 && id.v  < (int)pos.size()) P = pos[id.v];
            if (id.vn >=0 && id.vn < (int)nrm.size()) N = nrm[id.vn];
            glm::vec3 C = (glm::length(N)>0) ? 0.5f*(glm::normalize(N)+glm::vec3(1)) : glm::vec3(0.9f);
            V v; v.p[0]=P.x; v.p[1]=P.y; v.p[2]=P.z; v.c[0]=C.x; v.c[1]=C.y; v.c[2]=C.z;
            outVerts.push_back(v); return (unsigned)outVerts.size()-1;
        };
        unsigned ia=push(a), ib=push(b), ic=push(c);
        outIdx.push_back(ia); outIdx.push_back(ib); outIdx.push_back(ic);
    };

    while (fgets(line, sizeof(line), fp)) {
        if (line[0]=='v' && line[1]==' ') {
            glm::vec3 p; if (sscanf(line,"v %f %f %f",&p.x,&p.y,&p.z)==3) pos.push_back(p);
        } else if (line[0]=='v' && line[1]=='n') {
            glm::vec3 n; if (sscanf(line,"vn %f %f %f",&n.x,&n.y,&n.z)==3) nrm.push_back(n);
        } else if (line[0]=='f' && line[1]==' ') {
            face.clear(); char* s=line+2;
            while (*s) {
                while (*s==' ') ++s; if (*s=='\n'||*s=='\0') break;
                int vi=-1, ti=-1, ni=-1; char* st=s;
                vi=(int)strtol(s,&s,10);
                if (*s=='/') { ++s; if (*s=='/') { ++s; ni=(int)strtol(s,&s,10);} else { (void)strtol(s,&s,10); if (*s=='/') { ++s; ni=(int)strtol(s,&s,10);} } }
                if (s==st) break;
                Idx id; if (vi>0) id.v=vi-1; if (ni>0) id.vn=ni-1; face.push_back(id);
                while (*s==' ') ++s;
            }
            if (face.size()>=3) for (size_t i=1;i+1<face.size();++i) emitTri(face[0], face[i], face[i+1]);
        }
    }
    fclose(fp);
    return !outVerts.empty();
}

// -------- ObjectStore: primitives --------
void ObjectStore::InitPrimitives() {
    // Cube & Plane (non-indexed to match your current shader path)
    BuildCube(mCube);
    BuildPlane(mPlane);

    // Parametric shapes
    BuildSphere   (mSphere,   /*stacks*/24, /*slices*/36, /*R*/0.5f);
    BuildTorus    (mTorus,    /*segU*/48,   /*segV*/24,   /*R*/0.5f, /*r*/0.2f);
    BuildCone     (mCone,     /*seg*/32,    /*radius*/0.5f, /*h*/1.0f);
    BuildCylinder (mCylinder, /*seg*/32,    /*radius*/0.5f, /*h*/1.0f);
    BuildTriPrism (mPrism,    /*height*/1.0f, /*radius*/0.5f);
    BuildCircle   (mCircle,   /*seg*/64,    /*radius*/0.5f);
}

void ObjectStore::ReleasePrimitives() {
    mCube.Release();
    mSphere.Release();
    mTorus.Release();
    mCone.Release();
    mCylinder.Release();
    mPrism.Release();
    mCircle.Release();
    mPlane.Release();
}

// -------- CRUD --------
Object* ObjectStore::Create(EuclidShapeType t, const void* params,
                            const EuclidTransform& xform, EuclidObjectID id) {
    auto obj = std::make_unique<Object>();
    // auto-generate a usable id when caller passes 0
    if (id == 0) id = NewID();

    obj->id   = id;
    obj->type = t;
    obj->tf   = xform;

    for (int i=0;i<3;++i) if (obj->tf.scale[i] == 0.0f) obj->tf.scale[i] = 1.0f;
    ApplyParamsToScale(t, params, obj->tf);

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
    mNextID   = 1; // (optional) start fresh so ids stay small between scenes
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
        case EUCLID_SHAPE_CUBE:     bmin={-0.5f,-0.5f,-0.5f}; bmax={0.5f,0.5f,0.5f}; break;
        case EUCLID_SHAPE_PLANE:    bmin={-0.5f, 0.0f,-0.5f}; bmax={0.5f,0.0f,0.5f}; break;
        case EUCLID_SHAPE_SPHERE:   bmin={-0.5f,-0.5f,-0.5f}; bmax={0.5f,0.5f,0.5f}; break;
        case EUCLID_SHAPE_TORUS:    bmin={-0.7f,-0.2f,-0.7f}; bmax={0.7f,0.2f,0.7f}; break; // R=0.5, r=0.2
        case EUCLID_SHAPE_CONE:     bmin={-0.5f,-0.5f,-0.5f}; bmax={0.5f,0.5f,0.5f}; break;
        case EUCLID_SHAPE_CYLINDER: bmin={-0.5f,-0.5f,-0.5f}; bmax={0.5f,0.5f,0.5f}; break;
        case EUCLID_SHAPE_PRISM:    bmin={-0.5f,-0.5f,-0.5f}; bmax={0.5f,0.5f,0.5f}; break;
        case EUCLID_SHAPE_CIRCLE:   bmin={-0.5f, 0.0f,-0.5f}; bmax={0.5f,0.0f,0.5f}; break;
        default:                    bmin={-0.5f,-0.5f,-0.5f}; bmax={0.5f,0.5f,0.5f}; break;
    }
}

// -------- Mesh routing --------
const SharedMesh& ObjectStore::MeshFor(EuclidShapeType t) const {
    switch (t) {
        case EUCLID_SHAPE_CUBE:     return mCube;
        case EUCLID_SHAPE_PLANE:    return mPlane;
        case EUCLID_SHAPE_SPHERE:   return mSphere;
        case EUCLID_SHAPE_TORUS:    return mTorus;
        case EUCLID_SHAPE_CONE:     return mCone;
        case EUCLID_SHAPE_CYLINDER: return mCylinder;
        case EUCLID_SHAPE_PRISM:    return mPrism;
        case EUCLID_SHAPE_CIRCLE:   return mCircle;
        case EUCLID_SHAPE_CUSTOM:
        default:                    return mCube;
    }
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
                                    int viewportW, int viewportH) const
{
    if (viewportW <= 0 || viewportH <= 0) return 0;
    Ray ray = ScreenRay(screenX, screenY, viewportW, viewportH, invViewProj);

    EuclidObjectID best = 0;
    float bestT = 1e30f;

    for (auto& kv : mObjects) {
        const Object& o = *kv.second;

        // 👇 use custom bounds if this is an imported mesh
        glm::vec3 bminL, bmaxL;
        if (o.type == EUCLID_SHAPE_CUSTOM) {
            bminL = o.localMin;
            bmaxL = o.localMax;
        } else {
            ShapeLocalBounds(o.type, bminL, bmaxL);
        }

        glm::mat4 M = o.Model();

        // local AABB -> world AABB via 8 corners (as you already do)
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

// === ObjectStore methods ===
EuclidResult ObjectStore::LoadOBJ(const char* path, EuclidObjectID* outID, bool normalize)
{
    if (!path || !outID) return EUCLID_ERR_BAD_PARAM;

    std::vector<V> verts;
    std::vector<unsigned> idx;
    if (!ParseOBJ(path, verts, idx))
        return EUCLID_ERR_BAD_PARAM;

    if (normalize) NormalizeToUnit(verts);

    glm::vec3 mn, mx;
    ComputeAABB(verts, mn, mx);

    // Upload GPU mesh
    CustomEntry ce;
    UploadMesh(ce.mesh, verts, idx);
    ce.localMin = mn;
    ce.localMax = mx;

    const int customIndex = (int)mCustom.size();
    mCustom.push_back(std::move(ce));

    // Create scene object and hook it up to the custom mesh
    EuclidTransform xform{};
    xform.scale[0] = xform.scale[1] = xform.scale[2] = 1.f;

    Object* o = Create(EUCLID_SHAPE_CUSTOM, nullptr, xform, 0);
    if (!o) return EUCLID_ERR_INIT;            // <-- was EUCLID_ERR_UNKNOWN

    o->customIndex = customIndex;
    o->localMin    = mn;
    o->localMax    = mx;

    *outID = o->id;
    return EUCLID_OK;
}

EuclidResult ObjectStore::CreateFromRawMesh(const float* positions, size_t vertexCount,
                                            const unsigned* indices, size_t indexCount,
                                            EuclidObjectID* outID, bool normalize)
{
    if (!positions || vertexCount == 0 || !indices || indexCount < 3 || !outID)
        return EUCLID_ERR_BAD_PARAM;

    // Build vertex array
    std::vector<V> verts;
    verts.reserve(vertexCount);
    for (size_t i = 0; i < vertexCount; ++i) {
        V v{};
        v.p[0] = positions[3*i+0];
        v.p[1] = positions[3*i+1];
        v.p[2] = positions[3*i+2];
        v.c[0] = v.c[1] = v.c[2] = 0.9f; // default gray
        verts.push_back(v);
    }

    std::vector<unsigned> idx(indices, indices + indexCount);

    if (normalize) NormalizeToUnit(verts);

    glm::vec3 mn, mx;
    ComputeAABB(verts, mn, mx);

    // Upload GPU mesh
    CustomEntry ce;
    UploadMesh(ce.mesh, verts, idx);
    ce.localMin = mn;
    ce.localMax = mx;

    const int customIndex = (int)mCustom.size();
    mCustom.push_back(std::move(ce));

    // Create scene object
    EuclidTransform xform{};
    xform.scale[0] = xform.scale[1] = xform.scale[2] = 1.f;

    Object* o = Create(EUCLID_SHAPE_CUSTOM, nullptr, xform, 0);
    if (!o) return EUCLID_ERR_INIT;            // <-- was EUCLID_ERR_UNKNOWN

    o->customIndex = customIndex;
    o->localMin    = mn;
    o->localMax    = mx;

    *outID = o->id;
    return EUCLID_OK;
}



} // namespace Euclid
