#include "Core.hpp"

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
    BuildTranslationGizmo({0, 0, 0}, 2.0);
    BuildScaleTips({0, 0, 0}, 2.0);
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

    return true;
}
void Core::CleanUp() {
    if (mVBO) { glDeleteBuffers(1, &mVBO); mVBO = 0; }
    if (mVAO) { glDeleteVertexArrays(1, &mVAO); mVAO = 0; }
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

    // --- MAIN SCENE (cube) ---
    mainShader.Use();
    glm::mat4 model = mModel;

    GLint locModel = glGetUniformLocation(mainShader.GetID(), "uModel");
    GLint locView  = glGetUniformLocation(mainShader.GetID(), "uView");
    GLint locProj  = glGetUniformLocation(mainShader.GetID(), "uProjection");
    glUniformMatrix4fv(locModel, 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(locView,  1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(locProj,  1, GL_FALSE, &projection[0][0]);

    glBindVertexArray(mVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

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
    
    // Gizmo
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    
    glBindVertexArray(mDummyVAO);   // or mVAO if you don't want a dummy
    glDepthMask(GL_FALSE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    DrawRotationGizmo(viewProj);
    DrawTranslationGizmo(viewProj);
    DrawTransformationGizmo(viewProj);
    
    
    // restore default
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

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
        glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);

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
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);

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
void Core::DrawTranslationGizmo(const glm::mat4& viewProj) {
    translationShader.Use();
    glUniformMatrix4fv(glGetUniformLocation(translationShader.GetID(),"uViewProj"), 1, GL_FALSE, &viewProj[0][0]);
    glm::vec2 viewportPx = { (float)mWidth, (float)mHeight };
    glUniform2fv(glGetUniformLocation(translationShader.GetID(),"uViewportPx"), 1, &viewportPx[0]);
    glUniform1f (glGetUniformLocation(translationShader.GetID(),"uLinePx"), 10.0f);

    glBindVertexArray(mTranslationVAO);     // contains 12 verts total: 4 per axis
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);   // X
    glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);   // Y
    glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);   // Z
    glBindVertexArray(0);
}
void Core::DrawRotationGizmo(const glm::mat4& viewProj) {
    rotationShader.Use();
    glUniformMatrix4fv(glGetUniformLocation(rotationShader.GetID(),"uViewProj"), 1, GL_FALSE, &viewProj[0][0]);
    glm::vec2 viewportPx = { (float)mWidth, (float)mHeight };
    glUniform2fv(glGetUniformLocation(rotationShader.GetID(),"uViewportPx"), 1, &viewportPx[0]);
    glUniformMatrix4fv(glGetUniformLocation(rotationShader.GetID(),"uModel"), 1, GL_FALSE, &mModel[0][0]);
    glUniform1f(glGetUniformLocation(rotationShader.GetID(),"uRadius"), 1.0f);
    glUniform1f(glGetUniformLocation(rotationShader.GetID(),"uRingPx"), 10.0f);
    glUniform1i(glGetUniformLocation(rotationShader.GetID(),"uSegments"), 64);

    // X ring
    glUniform3f(glGetUniformLocation(rotationShader.GetID(),"uAxis"), 1,0,0);
    glUniform3f(glGetUniformLocation(rotationShader.GetID(),"uColor"), 0.95f,0.35f,0.35f);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 2*(64+1));

    // Y ring
    glUniform3f(glGetUniformLocation(rotationShader.GetID(),"uAxis"), 0,1,0);
    glUniform3f(glGetUniformLocation(rotationShader.GetID(),"uColor"), 0.40f,0.90f,0.45f);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 2*(64+1));

    // Z ring
    glUniform3f(glGetUniformLocation(rotationShader.GetID(),"uAxis"), 0,0,1);
    glUniform3f(glGetUniformLocation(rotationShader.GetID(),"uColor"), 0.35f,0.65f,0.95f);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 2*(64+1));
}
void Core::DrawTransformationGizmo(const glm::mat4& viewProj) {
    // shafts
      DrawTranslationGizmo(viewProj);

      // tip squares
      transformationShader.Use();
      glUniformMatrix4fv(glGetUniformLocation(transformationShader.GetID(),"uViewProj"), 1, GL_FALSE, &viewProj[0][0]);
      glm::vec2 viewportPx = { (float)mWidth, (float)mHeight };
      glUniform2fv(glGetUniformLocation(transformationShader.GetID(),"uViewportPx"), 1, &viewportPx[0]);
      glUniform1f(glGetUniformLocation(transformationShader.GetID(),"uSizePx"), 30.0f);

      glBindVertexArray(mTransformationVAO);     // 12 verts total, 4 per axis
      glDrawArrays(GL_TRIANGLE_STRIP, 0,  4);   // X tip
      glDrawArrays(GL_TRIANGLE_STRIP, 4,  4);   // Y tip
      glDrawArrays(GL_TRIANGLE_STRIP, 8,  4);   // Z tip
      glBindVertexArray(0);
}
void Core::OnMouseMove(double x, double y) {
    const bool orbiting = sCtrlDown || sMMBDown;
    if (!orbiting) {
        sFirstMoveWhileOrbit = true;
        return;
    }

    if (sFirstMoveWhileOrbit) {
        sLastX = x; sLastY = y;
        sFirstMoveWhileOrbit = false;
        return;
    }

    float dx = static_cast<float>(x - sLastX);
    float dy = static_cast<float>(y - sLastY);
    sLastX = x; sLastY = y;

    mainCamera.ProcessMouseMovement(dx, dy, /*constrainPitch*/ true);
}
void Core::OnMouseButton(int button, bool down, unsigned mods) {
    OnMods(mods); // refresh modifier cache

    if (button == kMouseMiddle) {
        sMMBDown = down;
        if (down) sFirstMoveWhileOrbit = true; // avoid jump on press
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

