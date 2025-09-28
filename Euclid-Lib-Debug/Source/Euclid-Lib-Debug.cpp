// debug.cpp  (GLFW + ImGui test app for Euclid DLL)
// Build with your ImGui backends: imgui_impl_glfw.cpp, imgui_impl_opengl3.cpp

#include <GLFW/glfw3.h>
#include "Euclid.h"

// ImGui
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <unordered_map>

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

// ---------------------
// Per-object param store
// ---------------------
struct ParamStore {
    EuclidShapeType type = EUCLID_SHAPE_CUBE;
    EuclidCubeParams      cube{1.0f};
    EuclidSphereParams    sphere{0.5f,24,24};
    EuclidTorusParams     torus{0.7f,0.25f,32,16};
    EuclidPlaneParams     plane{1.0f,1.0f};
    EuclidConeParams      cone{0.5f,1.0f,32};
    EuclidCylinderParams  cylinder{0.5f,1.0f,32};
    EuclidPrismParams     prism{6,0.5f,1.0f};
    EuclidCircleParams    circle{0.5f,64};
};

static void ApplyParamsToSelectedTransform(EuclidHandle H, EuclidObjectID id, const ParamStore& ps) {
    if (!id) return;

    EuclidTransform t;
    if (Euclid_GetObjectTransform(H, id, &t) != EUCLID_OK) return;

    auto setScale = [&](float sx, float sy, float sz){
        t.scale[0] = sx; t.scale[1] = sy; t.scale[2] = sz;
    };

    switch (ps.type) {
        case EUCLID_SHAPE_CUBE: {
            setScale(ps.cube.size, ps.cube.size, ps.cube.size);
        } break;
        case EUCLID_SHAPE_SPHERE: {
            float s = (ps.sphere.radius > 0.f) ? (ps.sphere.radius / 0.5f) : 1.f;
            setScale(s, s, s);
        } break;
        case EUCLID_SHAPE_PLANE: {
            setScale(ps.plane.width, 1.f, ps.plane.height);
        } break;
        case EUCLID_SHAPE_CONE: {
            float sR = (ps.cone.radius > 0.f) ? (ps.cone.radius / 0.5f) : 1.f;
            float sH = (ps.cone.height > 0.f) ? (ps.cone.height / 1.f) : 1.f;
            setScale(sR, sH, sR);
        } break;
        case EUCLID_SHAPE_CYLINDER: {
            float sR = (ps.cylinder.radius > 0.f) ? (ps.cylinder.radius / 0.5f) : 1.f;
            float sH = (ps.cylinder.height > 0.f) ? (ps.cylinder.height / 1.f) : 1.f;
            setScale(sR, sH, sR);
        } break;
        case EUCLID_SHAPE_PRISM: {
            float sR = (ps.prism.radius > 0.f) ? (ps.prism.radius / 0.5f) : 1.f;
            float sH = (ps.prism.height > 0.f) ? (ps.prism.height / 1.f) : 1.f;
            setScale(sR, sH, sR);
            // 'sides' ignored in current fixed-mesh build.
        } break;
        case EUCLID_SHAPE_CIRCLE: {
            float s = (ps.circle.radius > 0.f) ? (ps.circle.radius / 0.5f) : 1.f;
            setScale(s, 1.f, s);
        } break;
        case EUCLID_SHAPE_TORUS: {
            float sxz = (ps.torus.majorRadius + ps.torus.minorRadius) / 0.7f;
            float sy  = (ps.torus.minorRadius > 0.f) ? (ps.torus.minorRadius / 0.2f) : 1.f;
            setScale(sxz, sy, sxz);
        } break;
        default: break;
    }

    Euclid_SetObjectTransform(H, id, &t);
}


static std::unordered_map<EuclidObjectID, ParamStore> gParamsById;
static ParamStore gDefaults;                   // spawn defaults
static EuclidObjectID gLocalSelected = 0;      // <-- local selection cache

static void RememberParams(EuclidObjectID id, const ParamStore& ps) {
    gParamsById[id] = ps;
}
static void SetSelectionBoth(EuclidObjectID id) {
    Euclid_SetSelection(H, id);    // engine
    gLocalSelected = id;           // local cache
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
        ImGui_ImplGlfw_CursorPosCallback(w, x, y);

        const bool dragging = Euclid_IsDraggingGizmo(H) != 0;
        if (ImGui::GetIO().WantCaptureMouse && !dragging) return;

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

        double xpx, ypx; GetCursorPosPixels(w, &xpx, &ypx);
        Euclid_OnMouseMove(H, xpx, ypx);
    });

    glfwSetMouseButtonCallback(win, [](GLFWwindow* w,int b,int a,int m){
        ImGui_ImplGlfw_MouseButtonCallback(w, b, a, m);

        Euclid_OnMods(H, Mods(m));
        const bool want = ImGui::GetIO().WantCaptureMouse;
        const bool dragging = Euclid_IsDraggingGizmo(H) != 0;

        if (b == GLFW_MOUSE_BUTTON_LEFT) {
            if (a == GLFW_PRESS) {
                if (want) return;

                double xpx, ypx; GetCursorPosPixels(w, &xpx, &ypx);
                Euclid_OnMouseMove(H, xpx, ypx);
                Euclid_OnMouseButton(H, EUCLID_MOUSE_LEFT, 1, Mods(m));

                if (!Euclid_IsDraggingGizmo(H)) {
                    EuclidObjectID id = Euclid_RayPick(H, (float)xpx, (float)ypx);
                    SetSelectionBoth(id); // <-- set engine & local
                }
                return;
            } else if (a == GLFW_RELEASE) {
                if (dragging || !want) {
                    Euclid_OnMouseButton(H, EUCLID_MOUSE_LEFT, 0, Mods(m));
                }
                return;
            }
        }

        if (!want) Euclid_OnMouseButton(H, (EuclidMouseButton)b, a != GLFW_RELEASE, Mods(m));
    });

    glfwSetScrollCallback(win, [](GLFWwindow* w,double dx,double dy){
        ImGui_ImplGlfw_ScrollCallback(w, dx, dy);
        if (!ImGui::GetIO().WantCaptureMouse) Euclid_OnScroll(H, dx, dy);
    });

    glfwSetFramebufferSizeCallback(win, [](GLFWwindow*,int w,int h){
        Euclid_Resize(H,w,h); // framebuffer size (pixels)
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

        // Gizmo mode
        gizmo = Euclid_GetGizmoMode(H);
        int gm = (int)gizmo;
        ImGui::Text("Gizmo Mode  (1/2/3)");
        ImGui::RadioButton("Translate", &gm, EUCLID_GIZMO_TRANSLATE); ImGui::SameLine();
        ImGui::RadioButton("Rotate",    &gm, EUCLID_GIZMO_ROTATE);    ImGui::SameLine();
        ImGui::RadioButton("Scale",     &gm, EUCLID_GIZMO_SCALE);
        if ((EuclidGizmoMode)gm != gizmo) Euclid_SetGizmoMode(H, (EuclidGizmoMode)gm);

        ImGui::Separator();
        ImGui::Text("Add Primitives");

        // Add buttons (use defaults + remember params)
        if (ImGui::Button("Add Cube")) {
            EuclidCreateShapeDesc d{}; d.type=EUCLID_SHAPE_CUBE;
            EuclidCubeParams p = gDefaults.cube; d.params=&p; MakeDefaultTransform(d.xform, 0,0,0);
            EuclidObjectID id=0;
            if (Euclid_CreateShape(H,&d,&id)==EUCLID_OK && id) {
                SetSelectionBoth(id);
                ParamStore ps = gDefaults; ps.type = EUCLID_SHAPE_CUBE; RememberParams(id, ps);
            }
        } ImGui::SameLine();
        if (ImGui::Button("Add Sphere")) {
            EuclidCreateShapeDesc d{}; d.type=EUCLID_SHAPE_SPHERE;
            EuclidSphereParams p = gDefaults.sphere; d.params=&p; MakeDefaultTransform(d.xform, 1.5f,0,0);
            EuclidObjectID id=0;
            if (Euclid_CreateShape(H,&d,&id)==EUCLID_OK && id) {
                SetSelectionBoth(id);
                ParamStore ps = gDefaults; ps.type = EUCLID_SHAPE_SPHERE; RememberParams(id, ps);
            }
        } ImGui::SameLine();
        if (ImGui::Button("Add Torus")) {
            EuclidCreateShapeDesc d{}; d.type=EUCLID_SHAPE_TORUS;
            EuclidTorusParams p = gDefaults.torus; d.params=&p; MakeDefaultTransform(d.xform,-1.5f,0,0);
            EuclidObjectID id=0;
            if (Euclid_CreateShape(H,&d,&id)==EUCLID_OK && id) {
                SetSelectionBoth(id);
                ParamStore ps = gDefaults; ps.type = EUCLID_SHAPE_TORUS; RememberParams(id, ps);
            }
        } ImGui::SameLine();
        if (ImGui::Button("Add Plane")) {
            EuclidCreateShapeDesc d{}; d.type=EUCLID_SHAPE_PLANE;
            EuclidPlaneParams p = gDefaults.plane; d.params=&p; MakeDefaultTransform(d.xform, 0,-0.51f,0);
            EuclidObjectID id=0;
            if (Euclid_CreateShape(H,&d,&id)==EUCLID_OK && id) {
                SetSelectionBoth(id);
                ParamStore ps = gDefaults; ps.type = EUCLID_SHAPE_PLANE; RememberParams(id, ps);
            }
        }

        if (ImGui::Button("Add Cone")) {
            EuclidCreateShapeDesc d{}; d.type=EUCLID_SHAPE_CONE;
            EuclidConeParams p = gDefaults.cone; d.params=&p; MakeDefaultTransform(d.xform, -2.0f,0,1.5f);
            EuclidObjectID id=0;
            if (Euclid_CreateShape(H,&d,&id)==EUCLID_OK && id) {
                SetSelectionBoth(id);
                ParamStore ps = gDefaults; ps.type = EUCLID_SHAPE_CONE; RememberParams(id, ps);
            }
        } ImGui::SameLine();
        if (ImGui::Button("Add Cylinder")) {
            EuclidCreateShapeDesc d{}; d.type=EUCLID_SHAPE_CYLINDER;
            EuclidCylinderParams p = gDefaults.cylinder; d.params=&p; MakeDefaultTransform(d.xform, 2.0f,0,1.5f);
            EuclidObjectID id=0;
            if (Euclid_CreateShape(H,&d,&id)==EUCLID_OK && id) {
                SetSelectionBoth(id);
                ParamStore ps = gDefaults; ps.type = EUCLID_SHAPE_CYLINDER; RememberParams(id, ps);
            }
        } ImGui::SameLine();
        if (ImGui::Button("Add Prism")) {
            EuclidCreateShapeDesc d{}; d.type=EUCLID_SHAPE_PRISM;
            EuclidPrismParams p = gDefaults.prism; d.params=&p; MakeDefaultTransform(d.xform, 0,0,2.0f);
            EuclidObjectID id=0;
            if (Euclid_CreateShape(H,&d,&id)==EUCLID_OK && id) {
                SetSelectionBoth(id);
                ParamStore ps = gDefaults; ps.type = EUCLID_SHAPE_PRISM; RememberParams(id, ps);
            }
        } ImGui::SameLine();
        if (ImGui::Button("Add Circle")) {
            EuclidCreateShapeDesc d{}; d.type=EUCLID_SHAPE_CIRCLE;
            EuclidCircleParams p = gDefaults.circle; d.params=&p; MakeDefaultTransform(d.xform, -2.0f,0,-1.5f);
            EuclidObjectID id=0;
            if (Euclid_CreateShape(H,&d,&id)==EUCLID_OK && id) {
                SetSelectionBoth(id);
                ParamStore ps = gDefaults; ps.type = EUCLID_SHAPE_CIRCLE; RememberParams(id, ps);
            }
        }

        // Engine selection (may be 0) and fallback to local
        EuclidObjectID engSel=0; Euclid_GetSelection(H, &engSel);
        currentSelection = engSel ? engSel : gLocalSelected;

        ImGui::Separator();
        ImGui::Text("Selected (engine/local): %llu / %llu",
                    (unsigned long long)engSel,
                    (unsigned long long)gLocalSelected);

        if (currentSelection) {
            if (Euclid_GetObjectTransform(H, currentSelection, &tfCache)==EUCLID_OK) {
                bool changed=false;
                changed |= ImGui::DragFloat3("Pos",   tfCache.position, 0.01f);
                changed |= ImGui::DragFloat3("Rot",   tfCache.rotation, 0.2f);
                changed |= ImGui::DragFloat3("Scale", tfCache.scale,    0.01f);
                if (changed) Euclid_SetObjectTransform(H, currentSelection, &tfCache);
            }

            auto it = gParamsById.find(currentSelection);
            if (it != gParamsById.end()) {
                ParamStore& ps = it->second;
                ImGui::Separator();
                ImGui::Text("Selected Object Settings");
                bool edited = false;

                switch (ps.type) {
                    case EUCLID_SHAPE_CUBE:
                        edited |= ImGui::DragFloat("Edge", &ps.cube.size, 0.01f, 0.001f, 100.0f);
                        break;
                    case EUCLID_SHAPE_SPHERE:
                        edited |= ImGui::DragFloat("Radius", &ps.sphere.radius, 0.01f, 0.001f, 100.0f);
                        edited |= ImGui::DragInt("Slices", &ps.sphere.slices, 1, 3, 512);
                        edited |= ImGui::DragInt("Stacks", &ps.sphere.stacks, 1, 3, 512);
                        break;
                    case EUCLID_SHAPE_TORUS:
                        edited |= ImGui::DragFloat("Major R", &ps.torus.majorRadius, 0.01f, 0.001f, 100.0f);
                        edited |= ImGui::DragFloat("Minor r", &ps.torus.minorRadius, 0.01f, 0.001f, 100.0f);
                        edited |= ImGui::DragInt("Seg U", &ps.torus.majorSeg, 1, 3, 512);
                        edited |= ImGui::DragInt("Seg V", &ps.torus.minorSeg, 1, 3, 512);
                        break;
                    case EUCLID_SHAPE_PLANE:
                        edited |= ImGui::DragFloat("Width",  &ps.plane.width,  0.01f, 0.001f, 100.0f);
                        edited |= ImGui::DragFloat("Height", &ps.plane.height, 0.01f, 0.001f, 100.0f);
                        break;
                    case EUCLID_SHAPE_CONE:
                        edited |= ImGui::DragFloat("Radius",  &ps.cone.radius,  0.01f, 0.001f, 100.0f);
                        edited |= ImGui::DragFloat("Height",  &ps.cone.height,  0.01f, 0.001f, 100.0f);
                        edited |= ImGui::DragInt  ("Segments",&ps.cone.segments,1,  3,   1024);
                        break;
                    case EUCLID_SHAPE_CYLINDER:
                        edited |= ImGui::DragFloat("Radius",  &ps.cylinder.radius,  0.01f, 0.001f, 100.0f);
                        edited |= ImGui::DragFloat("Height",  &ps.cylinder.height,  0.01f, 0.001f, 100.0f);
                        edited |= ImGui::DragInt  ("Segments",&ps.cylinder.segments,1,  3,   1024);
                        break;
                    case EUCLID_SHAPE_PRISM:
                        edited |= ImGui::DragInt  ("Sides",  &ps.prism.sides, 1, 3, 128);
                        edited |= ImGui::DragFloat("Radius", &ps.prism.radius, 0.01f, 0.001f, 100.0f);
                        edited |= ImGui::DragFloat("Height", &ps.prism.height, 0.01f, 0.001f, 100.0f);
                        break;
                    case EUCLID_SHAPE_CIRCLE:
                        edited |= ImGui::DragFloat("Radius", &ps.circle.radius, 0.01f, 0.001f, 100.0f);
                        edited |= ImGui::DragInt  ("Segments",&ps.circle.segments,1, 3, 2048);
                        break;
                    default: break;
                }

                if (ImGui::Button("Apply (Recreate)")) {
                    ApplyParamsToSelectedTransform(H, currentSelection, ps);
                }

                ImGui::SameLine();
                if (ImGui::Button("Set as Defaults")) {
                    gDefaults = ps;
                }
            } else {
                ImGui::Separator();
                ImGui::TextDisabled("No stored params for this object.");
            }

            if (ImGui::Button("Delete Selected")) {
                Euclid_DeleteObject(H, currentSelection);
                gParamsById.erase(currentSelection);
                SetSelectionBoth(0);
            }
        } else {
            ImGui::TextDisabled("Click an object in the viewport to select.");
        }

        ImGui::Separator();
        if (ImGui::Button("Clear Scene")) {
            Euclid_ClearScene(H);
            gParamsById.clear();
            SetSelectionBoth(0);
        }
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
