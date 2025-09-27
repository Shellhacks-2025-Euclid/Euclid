#include "Core.hpp"

namespace Euclid {

namespace {
    constexpr unsigned kModCtrl      = 1u << 1; // matches EUCLID_MOD_CTRL
    constexpr unsigned kModSuper = 1u << 3; // Command on macOS
    constexpr int      kMouseMiddle  = 2;       // matches EUCLID_MOUSE_MIDDLE
}

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
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mainShader.Use();

    // Projection (use camera FOV)
    float aspect = (mHeight > 0) ? (float)mWidth / (float)mHeight : 1.0f;
    glm::mat4 projection = glm::perspective(glm::radians(mainCamera.GetZoom()), aspect, 0.1f, 100.0f);

    // View (camera looks at its target)
    glm::mat4 view = mainCamera.GetViewMatrix();

    // Model
    glm::mat4 model = mModel;

    // Upload uniforms
    GLint locModel = glGetUniformLocation(mainShader.GetID(), "uModel");
    GLint locView  = glGetUniformLocation(mainShader.GetID(), "uView");
    GLint locProj  = glGetUniformLocation(mainShader.GetID(), "uProjection");

    glUniformMatrix4fv(locModel, 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(locView,  1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(locProj,  1, GL_FALSE, &projection[0][0]);

    glBindVertexArray(mVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36 + 3);
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

