#include "Core.hpp"
#include <cmath>
#include <algorithm>
#include <glm/gtx/norm.hpp> 
#include <glm/gtx/euler_angles.hpp>

namespace Euclid {

namespace {
    constexpr unsigned kModCtrl      = 1u << 1; // matches EUCLID_MOD_CTRL
    constexpr unsigned kModSuper = 1u << 3; // Command on macOS
    constexpr int      kMouseMiddle  = 2;       // matches EUCLID_MOUSE_MIDDLE
}

struct MoveVert {
    glm::vec3 startWS;
    glm::vec3 endWS;
    float side;     // 0/1
    float corner;   // -1/+1
    glm::vec3 color;
};

struct TipVert {
    glm::vec3 centerWS;
    glm::vec3 color;
    glm::vec2 corner;
};

// ------ orbit gating state (DLL-internal) ------
static bool     sCtrlDown = false;
static bool     sMMBDown  = false;
static bool     sFirstMoveWhileOrbit = true;
static double   sLastX = 0.0, sLastY = 0.0;
static unsigned sMods   = 0; // last known modifiers
// ------------------------------------------------
static inline float WrapNearestDeg(float prevDeg, float currDeg) {
    // shift curr by +/-360 so it's closest to prev
    float d = currDeg - prevDeg;
    while (d >  180.f) { currDeg -= 360.f; d -= 360.f; }
    while (d < -180.f) { currDeg += 360.f; d += 360.f; }
    return currDeg;
}
static inline glm::mat3 MatFromEulerXYZ_Deg(float rx, float ry, float rz) {
    const float x = glm::radians(rx);
    const float y = glm::radians(ry);
    const float z = glm::radians(rz);
    // M = RotX(x) * RotY(y) * RotZ(z)  (XYZ intrinsic)
    return glm::mat3(glm::eulerAngleXYZ(x, y, z));
}

static inline glm::vec3 EulerXYZFromMatDeg(const glm::mat3& R) {
    float rx, ry, rz;
    glm::extractEulerAngleXYZ(glm::mat4(R), rx, ry, rz); // radians
    return glm::degrees(glm::vec3(rx, ry, rz));
}

void Init() {
    
}
bool Core::Init(int width, int height, int gl_major, int gl_minor) {
    mWidth = width;
    mHeight = height;

    InitShader();

    // Interleaved positions (xyz) + colors (rgb) for a cube (36 verts)
    const float tri[] = {
        // positions          // colors
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f, // back
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f, // front
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.5f, // left
        -0.5f,  0.5f, -0.5f,  0.0f, 0.5f, 1.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 0.5f, 0.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 0.5f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.5f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f, // right
         0.5f,  0.5f, -0.5f,  0.5f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 0.5f, 0.5f,
         0.5f, -0.5f, -0.5f,  0.0f, 0.5f, 0.5f,
         0.5f, -0.5f,  0.5f,  0.5f, 1.0f, 0.5f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,

        -0.5f, -0.5f, -0.5f,  0.3f, 0.7f, 0.5f, // bottom
         0.5f, -0.5f, -0.5f,  0.7f, 0.3f, 0.5f,
         0.5f, -0.5f,  0.5f,  0.5f, 0.7f, 0.3f,
         0.5f, -0.5f,  0.5f,  0.5f, 0.7f, 0.3f,
        -0.5f, -0.5f,  0.5f,  0.3f, 0.5f, 0.7f,
        -0.5f, -0.5f, -0.5f,  0.3f, 0.7f, 0.5f,

        -0.5f,  0.5f, -0.5f,  0.7f, 0.5f, 0.3f, // top
         0.5f,  0.5f, -0.5f,  0.5f, 0.3f, 0.7f,
         0.5f,  0.5f,  0.5f,  0.7f, 0.7f, 0.7f,
         0.5f,  0.5f,  0.5f,  0.7f, 0.7f, 0.7f,
        -0.5f,  0.5f,  0.5f,  0.2f, 0.8f, 0.4f,
        -0.5f,  0.5f, -0.5f,  0.7f, 0.5f, 0.3f
    };
    // VAO/VBO
    glGenVertexArrays(1, &mVAO);
    glGenBuffers(1, &mVBO);

    glBindVertexArray(mVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tri), tri, GL_STATIC_DRAW);

    // layout(location=0) vec3 aPos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    // layout(location=1) vec3 aColor
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Basic state
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);

    // --- Camera setup (orbiting around origin) ---
    // Start a bit above and to the side so we see cube edges nicely
    mainCamera = Camera({0.0f, 0.0f, 0.0f}, /*radius*/ 3.0f);
    mainCamera.SetYaw(-135.0f);
    mainCamera.SetPitch(-20.0f);
    mainCamera.SetZoom(45.0f);       // Projection FOV (independent from scroll)
    mainCamera.SetScrollSpeed(0.5f); // How fast radius changes per wheel notch

    // Model matrix (identity; let the camera do the orbiting)
    mModel = glm::mat4(1.0f);

    mObjs.InitPrimitives();
    
    mGizmoLength = 2.0f;
    BuildTranslationGizmo(glm::vec3(0), mGizmoLength);
    BuildScaleTips(glm::vec3(0), mGizmoLength);
    
    return true;
}
void Core::CleanUp() {
    if (mVBO) { glDeleteBuffers(1, &mVBO); mVBO = 0; }
    if (mVAO) { glDeleteVertexArrays(1, &mVAO); mVAO = 0; }
    mObjs.ReleasePrimitives();
}
void Core::Resize(int width, int height) {
    mWidth = width;
    mHeight = height;
    glViewport(0, 0, width, height);
}
void Core::Update(float dtSeconds) {
    // Keep identity so edges are visible due to the view angle.
    (void)dtSeconds;
    mModel = glm::mat4(1.0f);
}
void Core::Render() {
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // --- camera matrices ---
    float aspect = (mHeight > 0) ? (float)mWidth / (float)mHeight : 1.0f;
    glm::mat4 projection = glm::perspective(glm::radians(mainCamera.GetZoom()), aspect, 0.1f, 100.0f);
    glm::mat4 view = mainCamera.GetViewMatrix();
    glm::mat4 invVP = glm::inverse(projection * view);

    // If your Camera exposes position, use that:
    // glm::vec3 camPos = mainCamera.GetPosition();
    // Otherwise extract from view matrix:
    glm::mat4 invView = glm::inverse(view);
    glm::vec3 camPos = glm::vec3(invView[3]); // translation of inverse(view)
    glm::mat4 viewProj = projection * view;

    // --- MAIN SCENE ---
    DrawScene(view, projection);

    // --- GRID BACKGROUND PASS ---
    glBindVertexArray(mDummyVAO);   // or mVAO if you don't want a dummy
    glDepthMask(GL_FALSE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    gridShader.Use();
    GLint locInvVP = glGetUniformLocation(gridShader.GetID(), "uInvViewProj");
    GLint locCam   = glGetUniformLocation(gridShader.GetID(), "uCamPos");
    glUniformMatrix4fv(locInvVP, 1, GL_FALSE, &invVP[0][0]);
    glUniform3fv(locCam, 1, &camPos[0]);
    
    //glUseProgram(gridShader.GetID());
    glUniformMatrix4fv(glGetUniformLocation(gridShader.GetID(),"uInvViewProj"), 1, GL_FALSE, &invVP[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(gridShader.GetID(),"uViewProj"),    1, GL_FALSE, &viewProj[0][0]);
    glUniform3fv(glGetUniformLocation(gridShader.GetID(),"uCamPos"), 1, &camPos[0]);

    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    // --- SELECTED GIZMO RENDERING ---
    
    glDisable(GL_DEPTH_TEST);
    DrawGizmoForSelection(viewProj);

    glUseProgram(0);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
}
void Core::BuildTranslationGizmo(const glm::vec3 &origin, float L) {
    glm::vec3 X = origin + glm::vec3(L,0,0);
        glm::vec3 Y = origin + glm::vec3(0,L,0);
        glm::vec3 Z = origin + glm::vec3(0,0,L);

        const glm::vec3 cX(0.95f,0.35f,0.35f);
        const glm::vec3 cY(0.40f,0.90f,0.45f);
        const glm::vec3 cZ(0.35f,0.65f,0.95f);

        MoveVert v[12] = {
            // X shaft quad (triangle strip order)
            {origin, X, 0, -1, cX},
            {origin, X, 0, +1, cX},
            {origin, X, 1, -1, cX},
            {origin, X, 1, +1, cX},
            // Y
            {origin, Y, 0, -1, cY},
            {origin, Y, 0, +1, cY},
            {origin, Y, 1, -1, cY},
            {origin, Y, 1, +1, cY},
            // Z
            {origin, Z, 0, -1, cZ},
            {origin, Z, 0, +1, cZ},
            {origin, Z, 1, -1, cZ},
            {origin, Z, 1, +1, cZ},
        };

        glGenVertexArrays(1, &mTranslationVAO);
        glGenBuffers(1, &mTranslationVBO);
        glBindVertexArray(mTranslationVAO);
        glBindBuffer(GL_ARRAY_BUFFER, mTranslationVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_DYNAMIC_DRAW);

        GLsizei stride = sizeof(MoveVert);
        std::size_t off0 = offsetof(MoveVert, startWS);
        std::size_t off1 = offsetof(MoveVert, endWS);
        std::size_t off2 = offsetof(MoveVert, side);
        std::size_t off3 = offsetof(MoveVert, corner);
        std::size_t off4 = offsetof(MoveVert, color);

        glEnableVertexAttribArray(0); glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,stride,(void*)off0);
        glEnableVertexAttribArray(1); glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,stride,(void*)off1);
        glEnableVertexAttribArray(2); glVertexAttribPointer(2,1,GL_FLOAT,GL_FALSE,stride,(void*)off2);
        glEnableVertexAttribArray(3); glVertexAttribPointer(3,1,GL_FLOAT,GL_FALSE,stride,(void*)off3);
        glEnableVertexAttribArray(4); glVertexAttribPointer(4,3,GL_FLOAT,GL_FALSE,stride,(void*)off4);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void Core::BuildScaleTips(const glm::vec3& origin, float L) {
    glm::vec3 X = origin + glm::vec3(L,0,0);
    glm::vec3 Y = origin + glm::vec3(0,L,0);
    glm::vec3 Z = origin + glm::vec3(0,0,L);

    const glm::vec3 cX(0.95f,0.35f,0.35f);
    const glm::vec3 cY(0.40f,0.90f,0.45f);
    const glm::vec3 cZ(0.35f,0.65f,0.95f);

    const glm::vec2 corners[4] = { {-1,-1},{-1,1},{1,-1},{1,1} };

    TipVert v[12];
    for (int i=0;i<4;i++){ v[i+0] = {X,cX,corners[i]}; }
    for (int i=0;i<4;i++){ v[i+4] = {Y,cY,corners[i]}; }
    for (int i=0;i<4;i++){ v[i+8] = {Z,cZ,corners[i]}; }

    glGenVertexArrays(1, &mTransformationVAO);
    glGenBuffers(1, &mTransformationVBO);
    glBindVertexArray(mTransformationVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mTransformationVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_DYNAMIC_DRAW);

    GLsizei stride = sizeof(TipVert);
    std::size_t o0 = offsetof(TipVert, centerWS);
    std::size_t o1 = offsetof(TipVert, color);
    std::size_t o2 = offsetof(TipVert, corner);

    glEnableVertexAttribArray(0); glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,stride,(void*)o0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,stride,(void*)o1);
    glEnableVertexAttribArray(2); glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,stride,(void*)o2);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void Core::DrawTranslationGizmo(const glm::mat4& viewProj,
                                const glm::vec3& originWS,
                                float lengthWorld,
                                float linePx)
{
    translationShader.Use();
    glUniformMatrix4fv(glGetUniformLocation(translationShader.GetID(),"uViewProj"), 1, GL_FALSE, &viewProj[0][0]);

    glm::vec2 viewportPx = { (float)mWidth, (float)mHeight };
    glUniform2fv(glGetUniformLocation(translationShader.GetID(),"uViewportPx"), 1, &viewportPx[0]);
    glUniform1f (glGetUniformLocation(translationShader.GetID(),"uLinePx"), linePx);

    glBindVertexArray(mTranslationVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // X
    glDrawArrays(GL_TRIANGLE_STRIP, 4, 4); // Y
    glDrawArrays(GL_TRIANGLE_STRIP, 8, 4); // Z
    glBindVertexArray(0);
}
void Core::DrawRotationGizmo(const glm::mat4& viewProj,
                             const glm::vec3& originWS,
                             float radiusWorld,
                             float ringPx,
                             int   segments)
{
    rotationShader.Use();
    glUniformMatrix4fv(glGetUniformLocation(rotationShader.GetID(),"uViewProj"), 1, GL_FALSE, &viewProj[0][0]);

    glm::vec2 viewportPx = { (float)mWidth, (float)mHeight };
    glUniform2fv(glGetUniformLocation(rotationShader.GetID(),"uViewportPx"), 1, &viewportPx[0]);

    glUniform1f(glGetUniformLocation(rotationShader.GetID(),"uRadius"), radiusWorld);
    glUniform1f(glGetUniformLocation(rotationShader.GetID(),"uRingPx"), ringPx);
    glUniform1i(glGetUniformLocation(rotationShader.GetID(),"uSegments"), segments);
    glUniform3fv(glGetUniformLocation(rotationShader.GetID(),"uOrigin"), 1, &originWS[0]);

    // X ring
    glUniform3f(glGetUniformLocation(rotationShader.GetID(),"uAxis"), 1,0,0);
    glUniform3f(glGetUniformLocation(rotationShader.GetID(),"uColor"), 0.95f,0.35f,0.35f);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 2*(segments+1));

    // Y ring
    glUniform3f(glGetUniformLocation(rotationShader.GetID(),"uAxis"), 0,1,0);
    glUniform3f(glGetUniformLocation(rotationShader.GetID(),"uColor"), 0.40f,0.90f,0.45f);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 2*(segments+1));

    // Z ring
    glUniform3f(glGetUniformLocation(rotationShader.GetID(),"uAxis"), 0,0,1);
    glUniform3f(glGetUniformLocation(rotationShader.GetID(),"uColor"), 0.35f,0.65f,0.95f);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 2*(segments+1));
}
void Core::DrawTransformationGizmo(const glm::mat4& viewProj,
                                   const glm::vec3& originWS,
                                   float lengthWorld,
                                   float tipSizePx)
{
    // 1) shafts
    DrawTranslationGizmo(viewProj, originWS, lengthWorld, /*linePx*/10.0f);

    // 2) tips
    transformationShader.Use();
    glUniformMatrix4fv(glGetUniformLocation(transformationShader.GetID(),"uViewProj"), 1, GL_FALSE, &viewProj[0][0]);

    glm::vec2 viewportPx = { (float)mWidth, (float)mHeight };
    glUniform2fv(glGetUniformLocation(transformationShader.GetID(),"uViewportPx"), 1, &viewportPx[0]);
    glUniform1f(glGetUniformLocation(transformationShader.GetID(),"uSizePx"), tipSizePx);

    glBindVertexArray(mTransformationVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0,  4); // X tip
    glDrawArrays(GL_TRIANGLE_STRIP, 4,  4); // Y tip
    glDrawArrays(GL_TRIANGLE_STRIP, 8,  4); // Z tip
    glBindVertexArray(0);
}
void Core::OnMouseMove(double x, double y) {
    // 1) Gizmo drag takes priority (no Ctrl/MMB required)
    if (mDraggingGizmo) {
        UpdateGizmoDrag((float)x, (float)y);
        sLastX = x; sLastY = y;   // keep cache in sync
        return;
    }

    // 2) Orbit camera (same logic you had)
    const bool orbiting = sCtrlDown || sMMBDown;
    if (!orbiting) {                      // not orbiting -> just refresh last pos
        sFirstMoveWhileOrbit = true;
        sLastX = x; sLastY = y;
        return;
    }

    if (sFirstMoveWhileOrbit) {
        sLastX = x; sLastY = y;
        sFirstMoveWhileOrbit = false;
        return;
    }

    float dx = (float)(x - sLastX);
    float dy = (float)(y - sLastY);
    sLastX = x; sLastY = y;

    mainCamera.ProcessMouseMovement(dx, dy, /*constrainPitch*/ true);
}
void Core::OnMouseButton(int button, bool down, unsigned mods) {
    OnMods(mods);

    // MMB orbit (unchanged)
    if (button == kMouseMiddle) {
        sMMBDown = down;
        if (down) sFirstMoveWhileOrbit = true; // avoid jump
    }

    // LMB begins/ends gizmo drag
    const int kMouseLeft = 0; // matches EUCLID_MOUSE_LEFT
    if (button == kMouseLeft) {
        if (down) BeginGizmoDrag((float)sLastX, (float)sLastY);
        else      EndGizmoDrag();
    }
}
void Core::OnScroll(double dx, double dy) {
    (void)dx;
    // Scroll wheel changes ORBIT RADIUS
    mainCamera.ProcessMouseScroll(static_cast<float>(dy));
}
void Core::OnMods(unsigned mods) {
    sMods = mods;
    sCtrlDown = (sMods & kModCtrl) != 0 || (sMods & kModSuper) != 0;
}
}

// New namespace for Objects Logic to reduce the mess [BRUuuuuuuuuuH]
namespace Euclid
{

Euclid::Object* Core::CreateObject(EuclidShapeType t, const void* params,
                                   const EuclidTransform& xform, EuclidObjectID id) {
    return mObjs.Create(t, params, xform, id);
}

void Core::DestroyObjectGPU(EuclidObjectID id) {
    mObjs.DestroyGPU(id);
}

void Core::SetSelection(EuclidObjectID id) {
    mObjs.SetSelection(id);
    if (id != 0) {
        if (const Object* o = mObjs.Get(id)) {
            FocusOnObject(*o, /*adjustRadius=*/false); // keep current zoom, just change pivot
        }
    }
}

EuclidObjectID Core::RayPick(float x, float y) {
    float aspect = (mHeight>0)? float(mWidth)/float(mHeight) : 1.0f;
    glm::mat4 proj = glm::perspective(glm::radians(mainCamera.GetZoom()), aspect, 0.1f, 100.0f);
    glm::mat4 view = mainCamera.GetViewMatrix();
    glm::mat4 invVP = glm::inverse(proj * view);
    return mObjs.RayPick(x, y, invVP, mWidth, mHeight);
}

EuclidResult Core::GetObjectTransform(EuclidObjectID id, EuclidTransform& out) {
    return mObjs.GetTransform(id, out);
}

EuclidResult Core::SetObjectTransform(EuclidObjectID id, const EuclidTransform& in) {
    return mObjs.SetTransform(id, in);
}

void Core::RequestRebuildScene() {
    // kept for future (if you cache per-scene GPU data)
}

void Core::EndGizmoDrag() {
    mDraggingGizmo = false;
    mActiveGizmo   = GizmoPart::None;
}

// ---- PRIVATE FUNCTION CALLS ON OBJECTS ----
void Core::DrawObject(const Object& o, const glm::mat4& view, const glm::mat4& proj) {
    glm::mat4 model = o.Model();
    mainShader.Use();
    glUniformMatrix4fv(glGetUniformLocation(mainShader.GetID(),"uModel"),1,GL_FALSE,&model[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(mainShader.GetID(),"uView"),1,GL_FALSE,&view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(mainShader.GetID(),"uProjection"),1,GL_FALSE,&proj[0][0]);

    const SharedMesh& mesh = mObjs.MeshFor(o.type);
    glBindVertexArray(mesh.vao);
    if (mesh.indexed) glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
    else              glDrawArrays  (GL_TRIANGLES, 0, mesh.indexCount);
    glBindVertexArray(0);
}

void Core::DrawScene(const glm::mat4& view, const glm::mat4& proj) {
    for (auto& kv : mObjs.All()) DrawObject(*kv.second, view, proj);
}

void Core::DrawGizmoForSelection(const glm::mat4& viewProj) {
    EuclidObjectID sel = mObjs.GetSelection(); if (!sel) return;
    const Object* o = mObjs.Get(sel); if (!o) return;

    glm::vec3 pos(o->tf.position[0], o->tf.position[1], o->tf.position[2]);
    UpdateGizmoBasisFromObject(*o);
    UpdateGizmoGeometry(pos, mGizmoBasis, mGizmoLength);

    if (mGizmoMode==EUCLID_GIZMO_TRANSLATE || mGizmoMode==EUCLID_GIZMO_SCALE)
        DrawTransformationGizmo(viewProj, pos, mGizmoLength, 30.0f);
    else {
        // rotation rings oriented to object basis
        rotationShader.Use();
        glUniformMatrix4fv(glGetUniformLocation(rotationShader.GetID(),"uViewProj"), 1, GL_FALSE, &viewProj[0][0]);
        glUniform2f(glGetUniformLocation(rotationShader.GetID(),"uViewportPx"), (float)mWidth, (float)mHeight);
        glUniform1f(glGetUniformLocation(rotationShader.GetID(),"uRadius"), 1.0f);
        glUniform1f(glGetUniformLocation(rotationShader.GetID(),"uRingPx"), 10.0f);
        glUniform3fv(glGetUniformLocation(rotationShader.GetID(),"uOrigin"), 1, &pos[0]);
        glUniform1i(glGetUniformLocation(rotationShader.GetID(),"uSegments"), 64);

        // X ring in object space
        glm::vec3 ax;
        ax = glm::normalize(mGizmoBasis[0]); glUniform3f(glGetUniformLocation(rotationShader.GetID(),"uAxis"), ax.x,ax.y,ax.z);
        glUniform3f(glGetUniformLocation(rotationShader.GetID(),"uColor"), 0.95f,0.35f,0.35f);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 2*(64+1));

        // Y
        ax = glm::normalize(mGizmoBasis[1]); glUniform3f(glGetUniformLocation(rotationShader.GetID(),"uAxis"), ax.x,ax.y,ax.z);
        glUniform3f(glGetUniformLocation(rotationShader.GetID(),"uColor"), 0.40f,0.90f,0.45f);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 2*(64+1));

        // Z
        ax = glm::normalize(mGizmoBasis[2]); glUniform3f(glGetUniformLocation(rotationShader.GetID(),"uAxis"), ax.x,ax.y,ax.z);
        glUniform3f(glGetUniformLocation(rotationShader.GetID(),"uColor"), 0.35f,0.65f,0.95f);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 2*(64+1));
    }
}

void Core::FocusOnObject(const Object& o, bool adjustRadius) {
    // Set target to object's position, keep current yaw/pitch and (usually) the same radius
    glm::vec3 pos(o.tf.position[0], o.tf.position[1], o.tf.position[2]);
    mainCamera.SetTarget(pos);

    if (adjustRadius) {
        // Very conservative "fit" radius from object scale (unit primitives). Tweak as you like.
        glm::vec3 s(o.tf.scale[0], o.tf.scale[1], o.tf.scale[2]);
        float approxBound = 0.5f * glm::length(s);  // ~ half-diagonal of unit box scaled
        float minRadius   = glm::max(0.5f, approxBound * 2.2f);
        if (mainCamera.GetRadius() < minRadius)     // only zoom out if too close
            mainCamera.SetRadius(minRadius);
    }
}

Core::Ray Core::ScreenRay(float px, float py, const glm::mat4& invVP) const {
    float sx =  (2.0f * float(px) / float(mWidth)) - 1.0f;
    float sy = -(2.0f * float(py) / float(mHeight)) + 1.0f;
    glm::vec4 p0 = invVP * glm::vec4(sx, sy, -1.0f, 1.0f);
    glm::vec4 p1 = invVP * glm::vec4(sx, sy,  1.0f, 1.0f);
    p0 /= p0.w; p1 /= p1.w;
    return { glm::vec3(p0), glm::normalize(glm::vec3(p1 - p0)) };
}

bool Core::IntersectRayPlane(const Ray& r, const glm::vec3& p0, const glm::vec3& n, glm::vec3& hitWS) const {
    float denom = glm::dot(n, r.d);
    if (std::fabs(denom) < 1e-6f) return false;
    float t = glm::dot(p0 - r.o, n) / denom;
    if (t < 0.0f) return false;
    hitWS = r.o + t * r.d;
    return true;
}

float Core::WorldPerPixelAtViewZ(float absViewZ) const {
    float fovY = glm::radians(mainCamera.GetZoom());
    return 2.0f * std::tan(fovY * 0.5f) * absViewZ / float(std::max(1, mHeight));
}

bool Core::WorldToPixel(const glm::vec3& ws, const glm::mat4& VP, glm::vec2& outPx) const {
    glm::vec4 clip = VP * glm::vec4(ws,1);
    if (clip.w <= 0.0f) return false;
    glm::vec3 ndc = glm::vec3(clip) / clip.w;
    outPx.x = (ndc.x * 0.5f + 0.5f) * float(mWidth);
    outPx.y = (-ndc.y * 0.5f + 0.5f) * float(mHeight);
    return true;
}

float Core::DistPointToSegmentPx(const glm::vec2& p, const glm::vec2& a, const glm::vec2& b) const {
    glm::vec2 ab = b - a;
    float len2 = glm::dot(ab,ab);
    float t = (len2 > 0.0f) ? glm::clamp(glm::dot(p - a, ab) / len2, 0.0f, 1.0f) : 0.0f;
    glm::vec2 c = a + t*ab;
    return glm::length(p - c);
}

// --- pick translation/scale (shafts + tips) ---
Core::GizmoPart Core::PickTranslateScale(float mx, float my,
                                         const glm::mat4& view, const glm::mat4& proj,
                                         const glm::vec3& originWS,
                                         float lengthWorld, float linePx, float tipPx) const
{
    glm::mat4 VP = proj * view;

    glm::vec3 Xend = originWS + mGizmoBasis[0] * lengthWorld;
    glm::vec3 Yend = originWS + mGizmoBasis[1] * lengthWorld;
    glm::vec3 Zend = originWS + mGizmoBasis[2] * lengthWorld;

    glm::vec2 oPx, xPx, yPx, zPx;
    if (!WorldToPixel(originWS, VP, oPx)) return GizmoPart::None;
    bool vx = WorldToPixel(Xend, VP, xPx);
    bool vy = WorldToPixel(Yend, VP, yPx);
    bool vz = WorldToPixel(Zend, VP, zPx);

    glm::vec2 m{mx,my};
    const float halfLine = linePx * 0.5f + 3.0f; // +3px slack
    const float halfTip  = tipPx  * 0.5f + 3.0f;

    // billboarded tips
    if (vx && std::abs(m.x - xPx.x) <= halfTip && std::abs(m.y - xPx.y) <= halfTip) return GizmoPart::TipX;
    if (vy && std::abs(m.x - yPx.x) <= halfTip && std::abs(m.y - yPx.y) <= halfTip) return GizmoPart::TipY;
    if (vz && std::abs(m.x - zPx.x) <= halfTip && std::abs(m.y - zPx.y) <= halfTip) return GizmoPart::TipZ;

    // shafts as thick lines
    float dx = vx ? DistPointToSegmentPx(m, oPx, xPx) : 1e9f;
    float dy = vy ? DistPointToSegmentPx(m, oPx, yPx) : 1e9f;
    float dz = vz ? DistPointToSegmentPx(m, oPx, zPx) : 1e9f;

    float best = std::min({dx, dy, dz});
    if (best <= halfLine) {
        if (best == dx) return GizmoPart::MoveX;
        if (best == dy) return GizmoPart::MoveY;
        return GizmoPart::MoveZ;
    }
    return GizmoPart::None;
}

// --- pick rotation rings (screen-thickness aware) ---
Core::GizmoPart Core::PickRotate(float mx, float my,
                                 const glm::mat4& view, const glm::mat4& proj,
                                 const glm::vec3& originWS,
                                 float radiusWorld, float ringPx) const
{
    glm::mat4 VP = proj * view;
    glm::mat4 invVP = glm::inverse(VP);
    Ray ray = ScreenRay(mx, my, invVP);

    struct P { glm::vec3 n; GizmoPart part; } planes[3] = {
            { glm::normalize(mGizmoBasis[0]), GizmoPart::RotX },
            { glm::normalize(mGizmoBasis[1]), GizmoPart::RotY },
            { glm::normalize(mGizmoBasis[2]), GizmoPart::RotZ },
    };

    GizmoPart bestPart = GizmoPart::None;
    float bestErr = 1e9f;

    for (auto& pl : planes) {
        glm::vec3 hit;
        if (!IntersectRayPlane(ray, originWS, pl.n, hit)) continue;

        float r = glm::length(hit - originWS);
        float errWorld = std::abs(r - radiusWorld);

        // convert ringPx to world units at hit depth (add 3px slack)
        glm::vec4 hv = view * glm::vec4(hit,1);
        float wpp = WorldPerPixelAtViewZ(std::abs(hv.z));
        float tolWorld = (ringPx * 0.5f + 3.0f) * wpp;

        if (errWorld <= tolWorld && errWorld < bestErr) {
            bestErr = errWorld;
            bestPart = pl.part;
        }
    }
    return bestPart;
}

void Core::BeginGizmoDrag(float px, float py) {
    mActiveGizmo = GizmoPart::None;

    mDragObj = mObjs.GetSelection();
    const Object* o = mObjs.Get(mDragObj); if (!o) return;

    // freeze basis & origin for the entire drag
    mDragOriginWS = { o->tf.position[0], o->tf.position[1], o->tf.position[2] };
    UpdateGizmoBasisFromObject(*o);
    mDragBasis = mGizmoBasis;

    float aspect = (mHeight>0)? float(mWidth)/float(mHeight) : 1.0f;
    glm::mat4 proj = glm::perspective(glm::radians(mainCamera.GetZoom()), aspect, 0.1f, 100.0f);
    glm::mat4 view = mainCamera.GetViewMatrix();
    glm::mat4 invVP= glm::inverse(proj * view);

    const float linePx=10.f, tipPx=30.f, lenW=mGizmoLength, ringPx=10.f, radiusW=1.f;

    if (mGizmoMode == EUCLID_GIZMO_ROTATE) {
        mActiveGizmo = PickRotate(px, py, view, proj, mDragOriginWS, radiusW, ringPx);
        if (mActiveGizmo == GizmoPart::None) return;

        glm::vec3 n = (mActiveGizmo==GizmoPart::RotX)? mDragBasis[0]
                     : (mActiveGizmo==GizmoPart::RotY)? mDragBasis[1]
                                                      : mDragBasis[2];

        Ray ray = ScreenRay(px, py, invVP);
        glm::vec3 hit; if (!IntersectRayPlane(ray, mDragOriginWS, n, hit)) return;

        glm::vec3 t = glm::normalize(glm::abs(n.z) < 0.99f ? glm::cross(glm::vec3(0,0,1), n)
                                                            : glm::cross(glm::vec3(0,1,0), n));
        glm::vec3 b = glm::normalize(glm::cross(n, t));
        glm::vec3 v = glm::normalize(hit - mDragOriginWS);
        mAngle0 = std::atan2(glm::dot(v, b), glm::dot(v, t));
        mDraggingGizmo = true;
        return;
    }

    // Translate / Scale
    mActiveGizmo = PickTranslateScale(px, py, view, proj, mDragOriginWS, lenW, linePx, tipPx);
    if (mActiveGizmo == GizmoPart::None) return;

    // 1) freeze the world axis for this drag
    glm::vec3 axis(0);
    if (mActiveGizmo==GizmoPart::MoveX || mActiveGizmo==GizmoPart::TipX) axis = mDragBasis[0];
    if (mActiveGizmo==GizmoPart::MoveY || mActiveGizmo==GizmoPart::TipY) axis = mDragBasis[1];
    if (mActiveGizmo==GizmoPart::MoveZ || mActiveGizmo==GizmoPart::TipZ) axis = mDragBasis[2];
    mDragAxis = glm::normalize(axis);

    // 2) build a drag plane that contains the axis and is as perpendicular to the view as possible
    glm::mat4 invV = glm::inverse(view);
    glm::vec3 camF = -glm::normalize(glm::vec3(invV[2])); // camera forward (-Z); flip if you prefer
    glm::vec3 v = glm::cross(mDragAxis, camF);
    if (glm::dot(v, v) < 1e-8f) v = glm::vec3(0,1,0);   // fallback if nearly parallel
    mDragPlaneN = glm::normalize(glm::cross(v, mDragAxis));

    // 3) intersect current mouse ray with plane and cache scalar along the axis
    Ray ray = ScreenRay(px, py, invVP);
    glm::vec3 hit; if (!IntersectRayPlane(ray, mDragOriginWS, mDragPlaneN, hit)) return;
    mAxisT0 = glm::dot(hit - mDragOriginWS, mDragAxis);

    mDraggingGizmo = true;

}

// ---- UpdateGizmoDrag (apply deltas; NO re-pick, NO resetting) ----
void Core::UpdateGizmoDrag(float px, float py) {
    if (!mDraggingGizmo || !mDragObj) return;
    Object* o = mObjs.Get(mDragObj); if (!o) return;

    float aspect = (mHeight>0)? float(mWidth)/float(mHeight) : 1.0f;
    glm::mat4 proj = glm::perspective(glm::radians(mainCamera.GetZoom()), aspect, 0.1f, 100.0f);
    glm::mat4 view = mainCamera.GetViewMatrix();
    glm::mat4 invVP= glm::inverse(proj * view);

    // ---------------- ROTATE ----------------
    if (mGizmoMode == EUCLID_GIZMO_ROTATE) {
        glm::vec3 n(0); int idx=-1;
        if (mActiveGizmo==GizmoPart::RotX) { n=mDragBasis[0]; idx=0; }
        else if (mActiveGizmo==GizmoPart::RotY) { n=mDragBasis[1]; idx=1; }
        else if (mActiveGizmo==GizmoPart::RotZ) { n=mDragBasis[2]; idx=2; }
        else return;

        Ray ray = ScreenRay(px, py, invVP);
        glm::vec3 hit; if (!IntersectRayPlane(ray, mDragOriginWS, n, hit)) return;

        // tangent/binormal on the rotation plane for angle measurement
        glm::vec3 tVec = glm::normalize(glm::abs(n.z) < 0.99f ? glm::cross(glm::vec3(0,0,1), n)
                                                              : glm::cross(glm::vec3(0,1,0), n));
        glm::vec3 bVec = glm::normalize(glm::cross(n, tVec));
        glm::vec3 v    = glm::normalize(hit - mDragOriginWS);

        float a  = std::atan2(glm::dot(v, bVec), glm::dot(v, tVec)); // radians
        float da = a - mAngle0;                                      // radians (delta since last frame)
        mAngle0  = a;

        // Build current orientation from degrees (XYZ intrinsic)
        glm::mat3 Rcur = MatFromEulerXYZ_Deg(o->tf.rotation[0], o->tf.rotation[1], o->tf.rotation[2]);

        // Local axis for the chosen ring (X/Y/Z in local space)
        glm::vec3 axisLocal(0.0f); axisLocal[idx] = 1.0f;

        // Apply incremental rotation about LOCAL axis: R' = Rcur * Rot_local(da)
        glm::mat3 Rdelta = glm::mat3(glm::rotate(glm::mat4(1.0f), da, axisLocal));
        glm::mat3 Rnew   = Rcur * Rdelta;

        // Extract XYZ Euler (radians) and convert to degrees
        float rx, ry, rz;
        glm::extractEulerAngleXYZ(glm::mat4(Rnew), rx, ry, rz);
        glm::vec3 eDeg = EulerXYZFromMatDeg(Rnew);
        eDeg.x = WrapNearestDeg(o->tf.rotation[0], eDeg.x);
        eDeg.y = WrapNearestDeg(o->tf.rotation[1], eDeg.y);
        eDeg.z = WrapNearestDeg(o->tf.rotation[2], eDeg.z);

        o->tf.rotation[0] = eDeg.x;
        o->tf.rotation[1] = eDeg.y;
        o->tf.rotation[2] = eDeg.z;
        return;
    }

    // -------- TRANSLATE / SCALE (axis+plane mapping) --------
    // mDragAxis and mDragPlaneN were frozen in BeginGizmoDrag()
    Ray ray = ScreenRay(px, py, invVP);
    glm::vec3 hit; if (!IntersectRayPlane(ray, mDragOriginWS, mDragPlaneN, hit)) return;

    float t  = glm::dot(hit - mDragOriginWS, mDragAxis);  // scalar along frozen axis
    float dt = t - mAxisT0;

    if (mGizmoMode == EUCLID_GIZMO_TRANSLATE) {
        glm::vec3 p(o->tf.position[0], o->tf.position[1], o->tf.position[2]);
        p += mDragAxis * dt;                              // move along the visible axis
        o->tf.position[0] = p.x; o->tf.position[1] = p.y; o->tf.position[2] = p.z;
        mAxisT0 = t;
        return;
    }

    if (mGizmoMode == EUCLID_GIZMO_SCALE) {
        int idx = -1;
        if (mActiveGizmo==GizmoPart::MoveX || mActiveGizmo==GizmoPart::TipX) idx=0;
        else if (mActiveGizmo==GizmoPart::MoveY || mActiveGizmo==GizmoPart::TipY) idx=1;
        else if (mActiveGizmo==GizmoPart::MoveZ || mActiveGizmo==GizmoPart::TipZ) idx=2;
        if (idx<0) return;

        float s = 1.0f + dt;
        o->tf.scale[idx] = glm::max(0.001f, o->tf.scale[idx] * s);
        mAxisT0 = t;
        return;
    }
}


void Core::UpdateGizmoBasisFromObject(const Object& o) {
    glm::mat4 M = o.Model();
    glm::vec3 X = glm::normalize(glm::vec3(M[0]));
    glm::vec3 Y = glm::normalize(glm::vec3(M[1]));
    glm::vec3 Z = glm::normalize(glm::vec3(M[2]));
    mGizmoBasis = glm::mat3(X,Y,Z);
}

void Core::UpdateGizmoGeometry(const glm::vec3& origin, const glm::mat3& B, float L) {
    // endpoints in world space using object-local axes
    glm::vec3 Xend = origin + B[0] * L; // B[col] is a vec3 column
    glm::vec3 Yend = origin + B[1] * L;
    glm::vec3 Zend = origin + B[2] * L;

    const glm::vec3 cX(0.95f,0.35f,0.35f);
    const glm::vec3 cY(0.40f,0.90f,0.45f);
    const glm::vec3 cZ(0.35f,0.65f,0.95f);

    // --- shafts (triangle strips) ---
    MoveVert mv[12] = {
        {origin,Xend,0,-1,cX}, {origin,Xend,0,+1,cX}, {origin,Xend,1,-1,cX}, {origin,Xend,1,+1,cX},
        {origin,Yend,0,-1,cY}, {origin,Yend,0,+1,cY}, {origin,Yend,1,-1,cY}, {origin,Yend,1,+1,cY},
        {origin,Zend,0,-1,cZ}, {origin,Zend,0,+1,cZ}, {origin,Zend,1,-1,cZ}, {origin,Zend,1,+1,cZ},
    };
    glBindBuffer(GL_ARRAY_BUFFER, mTranslationVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(mv), mv);

    // --- tips (billboard quads) ---
    const glm::vec2 corners[4] = {{-1,-1},{-1,1},{1,-1},{1,1}};
    TipVert tv[12];
    for (int i=0;i<4;i++){ tv[i+0] = {Xend,cX,corners[i]}; }
    for (int i=0;i<4;i++){ tv[i+4] = {Yend,cY,corners[i]}; }
    for (int i=0;i<4;i++){ tv[i+8] = {Zend,cZ,corners[i]}; }
    glBindBuffer(GL_ARRAY_BUFFER, mTransformationVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(tv), tv);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

}
