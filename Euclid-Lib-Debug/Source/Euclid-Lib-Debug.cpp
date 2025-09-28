// debug.cpp  (GLFW + ImGui test app for Euclid DLL)
// Build with your ImGui backends: imgui_impl_glfw.cpp, imgui_impl_opengl3.cpp

#include <GLFW/glfw3.h>
#include "Euclid.h"

// ImGui
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

static EuclidHandle H = nullptr;
static void* Loader(const char* n){ return (void*)glfwGetProcAddress(n); }
static uint8_t Mods(int m) {
    uint8_t r=0;
    if(m&GLFW_MOD_SHIFT)   r|=EUCLID_MOD_SHIFT;
    if(m&GLFW_MOD_CONTROL) r|=EUCLID_MOD_CTRL;
    if(m&GLFW_MOD_ALT)     r|=EUCLID_MOD_ALT;
    if(m&GLFW_MOD_SUPER)   r|=EUCLID_MOD_SUPER;
    return r;
}

// Convert cursor from window coords to framebuffer pixel coords (handles Retina/HiDPI)
static void GetCursorPosPixels(GLFWwindow* w, double* outXpx, double* outYpx) {
    double xw=0, yw=0; glfwGetCursorPos(w, &xw, &yw);
    int ww=1, wh=1, fbw=1, fbh=1;
    glfwGetWindowSize(w, &ww, &wh);
    glfwGetFramebufferSize(w, &fbw, &fbh);
    const double sx = (ww>0) ? double(fbw)/double(ww) : 1.0;
    const double sy = (wh>0) ? double(fbh)/double(wh) : 1.0;
    *outXpx = xw * sx;
    *outYpx = yw * sy;
}

static void MakeDefaultTransform(EuclidTransform& t, float px=0, float py=0, float pz=0) {
    t.position[0]=px; t.position[1]=py; t.position[2]=pz;
    t.rotation[0]=t.rotation[1]=t.rotation[2]=0.f;
    t.scale[0]=t.scale[1]=t.scale[2]=1.f;
}

int main(){
    // --- GLFW / GL ---
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GL_TRUE);
#endif
    GLFWwindow* win=glfwCreateWindow(1280,720,"Euclid Debug",nullptr,nullptr);
    glfwMakeContextCurrent(win);
    glfwSwapInterval(1);

    // --- Euclid init ---
    EuclidConfig cfg{1280, 720, 3, 3};
    if (Euclid_Create(&cfg, Loader, &H) != EUCLID_OK) return -1;

    // --- ImGui init (no auto-install callbacks) ---
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(win, /*install_callbacks=*/false);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // --- Callbacks ---
    glfwSetKeyCallback(win, [](GLFWwindow* w, int key, int sc, int action, int mods){
        ImGui_ImplGlfw_KeyCallback(w, key, sc, action, mods);
        Euclid_OnMods(H, Mods(mods));
        if (ImGui::GetIO().WantCaptureKeyboard) return;
        if (action==GLFW_PRESS || action==GLFW_REPEAT) {
            if (key==GLFW_KEY_1) Euclid_SetGizmoMode(H, EUCLID_GIZMO_TRANSLATE);
            if (key==GLFW_KEY_2) Euclid_SetGizmoMode(H, EUCLID_GIZMO_ROTATE);
            if (key==GLFW_KEY_3) Euclid_SetGizmoMode(H, EUCLID_GIZMO_SCALE);
        }
    });

    glfwSetCharCallback(win, [](GLFWwindow* w, unsigned int c){
        ImGui_ImplGlfw_CharCallback(w, c);
    });

    glfwSetCursorPosCallback(win, [](GLFWwindow* w,double x,double y){
        // Always forward to ImGui first
        ImGui_ImplGlfw_CursorPosCallback(w, x, y);

        const bool dragging = Euclid_IsDraggingGizmo(H) != 0;
        // If the UI wants the mouse AND we aren't dragging a gizmo, don't touch the engine
        if (ImGui::GetIO().WantCaptureMouse && !dragging) return;

        // keep Euclid's modifier cache fresh
        int mods = 0;
        if (glfwGetKey(w, GLFW_KEY_LEFT_CONTROL)  == GLFW_PRESS ||
            glfwGetKey(w, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)  mods |= GLFW_MOD_CONTROL;
        if (glfwGetKey(w, GLFW_KEY_LEFT_SUPER)    == GLFW_PRESS ||
            glfwGetKey(w, GLFW_KEY_RIGHT_SUPER)   == GLFW_PRESS)  mods |= GLFW_MOD_SUPER;
        if (glfwGetKey(w, GLFW_KEY_LEFT_SHIFT)    == GLFW_PRESS ||
            glfwGetKey(w, GLFW_KEY_RIGHT_SHIFT)   == GLFW_PRESS)  mods |= GLFW_MOD_SHIFT;
        if (glfwGetKey(w, GLFW_KEY_LEFT_ALT)      == GLFW_PRESS ||
            glfwGetKey(w, GLFW_KEY_RIGHT_ALT)     == GLFW_PRESS)  mods |= GLFW_MOD_ALT;
        Euclid_OnMods(H, Mods(mods));

        // send HiDPI-correct pixel coords to the DLL
        double xpx, ypx; GetCursorPosPixels(w, &xpx, &ypx);
        Euclid_OnMouseMove(H, xpx, ypx);
    });

    glfwSetMouseButtonCallback(win, [](GLFWwindow* w,int b,int a,int m){
        // Forward to ImGui first
        ImGui_ImplGlfw_MouseButtonCallback(w, b, a, m);

        Euclid_OnMods(H, Mods(m));
        const bool want = ImGui::GetIO().WantCaptureMouse;
        const bool dragging = Euclid_IsDraggingGizmo(H) != 0;

        if (b == GLFW_MOUSE_BUTTON_LEFT) {
            if (a == GLFW_PRESS) {
                if (want) return;  // UI consumes the press

                double xpx, ypx; GetCursorPosPixels(w, &xpx, &ypx);
                Euclid_OnMouseMove(H, xpx, ypx);                 // keep 'last' in sync
                Euclid_OnMouseButton(H, EUCLID_MOUSE_LEFT, 1, Mods(m));

                if (!Euclid_IsDraggingGizmo(H)) {
                    EuclidObjectID id = Euclid_RayPick(H, (float)xpx, (float)ypx);
                    Euclid_SetSelection(H, id);
                }
                return;
            } else if (a == GLFW_RELEASE) {
                // Always deliver release to the engine if it started a drag,
                // even if ImGui currently wants the mouse.
                if (dragging || !want) {
                    Euclid_OnMouseButton(H, EUCLID_MOUSE_LEFT, 0, Mods(m));
                }
                return;
            }
        }

        // Other buttons only if UI doesn't want them
        if (!want) Euclid_OnMouseButton(H, (EuclidMouseButton)b, a != GLFW_RELEASE, Mods(m));
    });

    glfwSetScrollCallback(win, [](GLFWwindow* w,double dx,double dy){
        ImGui_ImplGlfw_ScrollCallback(w, dx, dy);
        if (!ImGui::GetIO().WantCaptureMouse) Euclid_OnScroll(H, dx, dy);
    });

    glfwSetFramebufferSizeCallback(win, [](GLFWwindow*,int w,int h){
        Euclid_Resize(H,w,h); // framebuffer size (pixels) — correct for HiDPI
    });

    // UI state
    EuclidObjectID currentSelection = 0;
    EuclidGizmoMode gizmo = Euclid_GetGizmoMode(H);
    float fpsAvg = 0.f;
    EuclidTransform tfCache; MakeDefaultTransform(tfCache);

    // --- Main loop ---
    double last=glfwGetTime();
    while(!glfwWindowShouldClose(win)){
        double now=glfwGetTime(); float dt=float(now-last); last=now;
        fpsAvg = 0.9f*fpsAvg + 0.1f*(1.f/dt);

        Euclid_Update(H, dt);
        Euclid_Render(H);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Euclid Controls");
        ImGui::Text("FPS: %.1f", fpsAvg);
        ImGui::Separator();

        gizmo = Euclid_GetGizmoMode(H);
        int gm = (int)gizmo;
        ImGui::Text("Gizmo Mode  (1/2/3)");
        ImGui::RadioButton("Translate", &gm, EUCLID_GIZMO_TRANSLATE); ImGui::SameLine();
        ImGui::RadioButton("Rotate",    &gm, EUCLID_GIZMO_ROTATE);    ImGui::SameLine();
        ImGui::RadioButton("Scale",     &gm, EUCLID_GIZMO_SCALE);
        if ((EuclidGizmoMode)gm != gizmo) Euclid_SetGizmoMode(H, (EuclidGizmoMode)gm);

        ImGui::Separator();

        if (ImGui::Button("Add Cube"))   { EuclidCreateShapeDesc d{}; d.type=EUCLID_SHAPE_CUBE;   EuclidCubeParams p{1.0f}; d.params=&p; MakeDefaultTransform(d.xform, 0,0,0); EuclidObjectID id; Euclid_CreateShape(H,&d,&id); Euclid_SelectObject(H,id); }
        ImGui::SameLine();
        if (ImGui::Button("Add Sphere")) { EuclidCreateShapeDesc d{}; d.type=EUCLID_SHAPE_SPHERE; EuclidSphereParams p{0.5f,16,16}; d.params=&p; MakeDefaultTransform(d.xform, 1.5f,0,0); EuclidObjectID id; Euclid_CreateShape(H,&d,&id); Euclid_SelectObject(H,id); }
        ImGui::SameLine();
        if (ImGui::Button("Add Torus"))  { EuclidCreateShapeDesc d{}; d.type=EUCLID_SHAPE_TORUS;  EuclidTorusParams p{0.7f,0.25f,24,12}; d.params=&p; MakeDefaultTransform(d.xform,-1.5f,0,0); EuclidObjectID id; Euclid_CreateShape(H,&d,&id); Euclid_SelectObject(H,id); }
        ImGui::SameLine();
        if (ImGui::Button("Add Plane"))  { EuclidCreateShapeDesc d{}; d.type=EUCLID_SHAPE_PLANE;  EuclidPlaneParams p{1.0f,1.0f}; d.params=&p; MakeDefaultTransform(d.xform, 0,-0.5f,0); EuclidObjectID id; Euclid_CreateShape(H,&d,&id); Euclid_SelectObject(H,id); }

        Euclid_GetSelection(H, &currentSelection);
        ImGui::Separator();
        ImGui::Text("Selected ID: %llu", (unsigned long long)currentSelection);

        if (currentSelection) {
            if (Euclid_GetObjectTransform(H, currentSelection, &tfCache)==EUCLID_OK) {
                bool changed=false;
                changed |= ImGui::DragFloat3("Pos",   tfCache.position, 0.01f);
                changed |= ImGui::DragFloat3("Rot",   tfCache.rotation, 0.2f);
                changed |= ImGui::DragFloat3("Scale", tfCache.scale,    0.01f);
                if (changed) Euclid_SetObjectTransform(H, currentSelection, &tfCache);
            }
            if (ImGui::Button("Delete Selected")) Euclid_DeleteObject(H, currentSelection);
        } else {
            ImGui::TextDisabled("Click an object in the viewport to select.");
        }

        ImGui::Separator();
        if (ImGui::Button("Clear Scene")) Euclid_ClearScene(H);
        ImGui::SameLine();
        ImGui::TextDisabled("Orbit: Ctrl/MMB + drag   |   Zoom: wheel");

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    Euclid_Destroy(H);
    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
