#include "Euclid_Core.h"
#include "State.hpp"
#include <glad/glad.h>
#include <new>
#include <string>

static thread_local std::string g_last_error;
static void set_err(const char* msg) { g_last_error = msg ? msg : ""; }

EUCLID_EXTERN_C EUCLID_API const char* EUCLID_CALL Euclid_GetLastError() { return g_last_error.c_str(); }
EUCLID_EXTERN_C EUCLID_API const char* EUCLID_CALL Euclid_Version() { return "Euclid 0.1.0"; }

EUCLID_EXTERN_C EUCLID_API EuclidResult EUCLID_CALL
Euclid_Create(const EuclidConfig* cfg, Euclid_GetProcAddr loader, EuclidHandle* out)
{
    if (!cfg || !loader || !out) { set_err("bad params"); return EUCLID_ERR_BAD_PARAM; }

    if (!gladLoadGLLoader((GLADloadproc)loader)) {
        set_err("gladLoadGLLoader failed");
        return EUCLID_ERR_GLAD;
    }

    auto* s = new (std::nothrow) EuclidState();
    if (!s) { set_err("oom"); return EUCLID_ERR_INIT; }

    s->fbW = cfg->width; s->fbH = cfg->height;

    if (!s->core.Init(cfg->width, cfg->height, cfg->gl_major, cfg->gl_minor)) {
        delete s; set_err("Core.Init failed"); return EUCLID_ERR_INIT;
    }

    s->ready = true;
    s->targetFbo = 0; // ÔÓ ÛÏÓÎ˜‡ÌË˛ ó backbuffer
    *out = (EuclidHandle)s;
    set_err(nullptr);
    return EUCLID_OK;
}

EUCLID_EXTERN_C EUCLID_API void EUCLID_CALL Euclid_Destroy(EuclidHandle h)
{
    if (!h) return;
    auto* s = (EuclidState*)h;
    delete s;
}

EUCLID_EXTERN_C EUCLID_API void EUCLID_CALL Euclid_Resize(EuclidHandle h, int w, int hgt)
{
    if (auto* s = (EuclidState*)h) {
        s->fbW = w; s->fbH = hgt;
        s->core.Resize(w, hgt);
    }
}

// ÕŒ¬Œ≈
EUCLID_EXTERN_C EUCLID_API void EUCLID_CALL Euclid_SetFramebuffer(EuclidHandle h, unsigned int fb)
{
    if (auto* s = (EuclidState*)h) s->targetFbo = fb;
}

EUCLID_EXTERN_C EUCLID_API void EUCLID_CALL Euclid_Update(EuclidHandle h, float dt)
{
    if (auto* s = (EuclidState*)h) s->core.Update(dt);
}

EUCLID_EXTERN_C EUCLID_API void EUCLID_CALL Euclid_Render(EuclidHandle h)
{
    if (auto* s = (EuclidState*)h) {
        // –»—”≈Ã ¬ Õ”∆Õ€… FBO
        glBindFramebuffer(GL_FRAMEBUFFER, s->targetFbo);
        glViewport(0, 0, s->fbW, s->fbH);
        // ÔÓ ÊÂÎ‡ÌË˛: glDisable(GL_FRAMEBUFFER_SRGB); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        s->core.Render();
    }
}


// Shapes and gizmo calls
static inline EuclidResult ok_or(EuclidState* s){ return s ? EUCLID_OK : EUCLID_ERR_BAD_PARAM; }

EUCLID_EXTERN_C EUCLID_API EuclidResult EUCLID_CALL
Euclid_ClearScene(EuclidHandle h) {
    if (auto* s=(EuclidState*)h) {
        // s->ids.clear();  // if you keep a UI id set
        // s->selected = 0; // optional local cache
        return s->core.ClearScene();
    }
    return EUCLID_ERR_BAD_PARAM;
}

EUCLID_EXTERN_C EUCLID_API EuclidResult EUCLID_CALL
Euclid_CreateShape(EuclidHandle h, const EuclidCreateShapeDesc* desc, EuclidObjectID* out_id) {
    if (!h || !desc || !out_id) return EUCLID_ERR_BAD_PARAM;
    auto* s = (EuclidState*)h;
    auto id = s->nextID++; // if you assign ids in wrapper

    Euclid::Object* obj = s->core.CreateObject(desc->type, desc->params, desc->xform, id);
    if (!obj) return EUCLID_ERR_BAD_PARAM;

    // DO NOT store obj in s->objects as unique_ptr — Core owns it.
    *out_id = id;
    return EUCLID_OK;
}

EUCLID_EXTERN_C EUCLID_API EuclidResult EUCLID_CALL
Euclid_DeleteObject(EuclidHandle h, EuclidObjectID id) {
    if (!h) return EUCLID_ERR_BAD_PARAM;
    auto* s = (EuclidState*)h;
    const EuclidResult r = s->core.DeleteObject(id);
    if (r == EUCLID_OK) {
        // sync wrapper caches if you keep any
        // s->ids.erase(id);
        // s->selected = (s->selected == id ? 0 : s->selected);
    }
    return r;
}

EUCLID_EXTERN_C EUCLID_API EuclidResult EUCLID_CALL
Euclid_SelectObject(EuclidHandle h, EuclidObjectID id) {
    if (auto* s=(EuclidState*)h) {
        // optional: validate id by asking Core or your id set
        s->core.SetSelection(id);
        return EUCLID_OK;
    }
    return EUCLID_ERR_BAD_PARAM;
}

EUCLID_EXTERN_C EUCLID_API EuclidResult EUCLID_CALL
Euclid_GetSelection(EuclidHandle h, EuclidObjectID* out_id) {
    if (!out_id) return EUCLID_ERR_BAD_PARAM;
    if (auto* s=(EuclidState*)h) { *out_id = s->selected; return EUCLID_OK; }
    return EUCLID_ERR_BAD_PARAM;
}

EUCLID_EXTERN_C EUCLID_API EuclidResult EUCLID_CALL
Euclid_SetGizmoMode(EuclidHandle h, EuclidGizmoMode mode) {
    if (auto* s=(EuclidState*)h) {
        s->gizmoMode = mode;
        s->core.SetGizmoMode(mode);
        return EUCLID_OK;
    }
    return EUCLID_ERR_BAD_PARAM;
}

EUCLID_EXTERN_C EUCLID_API EuclidGizmoMode EUCLID_CALL
Euclid_GetGizmoMode(EuclidHandle h) {
    if (auto* s=(EuclidState*)h) return s->gizmoMode;
    return EUCLID_GIZMO_NONE;
}

EUCLID_EXTERN_C EUCLID_API EuclidResult EUCLID_CALL
Euclid_HitTestSelect(EuclidHandle h, double x, double y, EuclidObjectID* out_id) {
    if (!out_id) return EUCLID_ERR_BAD_PARAM;
    if (auto* s=(EuclidState*)h) {
        auto id = s->core.RayPick(static_cast<float>(x), static_cast<float>(y));
        *out_id = id;
        s->selected = id;
        s->core.SetSelection(id);
        return EUCLID_OK;
    }
    return EUCLID_ERR_BAD_PARAM;
}

EUCLID_EXTERN_C EUCLID_API EuclidResult EUCLID_CALL
Euclid_GetObjectTransform(EuclidHandle h, EuclidObjectID id, EuclidTransform* out_tf) {
    if (!out_tf) return EUCLID_ERR_BAD_PARAM;
    if (auto* s=(EuclidState*)h) return s->core.GetObjectTransform(id, *out_tf);
    return EUCLID_ERR_BAD_PARAM;
}
EUCLID_EXTERN_C EUCLID_API EuclidResult EUCLID_CALL
Euclid_SetObjectTransform(EuclidHandle h, EuclidObjectID id, const EuclidTransform* tf) {
    if (!tf) return EUCLID_ERR_BAD_PARAM;
    if (auto* s=(EuclidState*)h) return s->core.SetObjectTransform(id, *tf);
    return EUCLID_ERR_BAD_PARAM;
}
EUCLID_EXTERN_C EUCLID_API EuclidObjectID EUCLID_CALL Euclid_RayPick(EuclidHandle h, float x, float y){
    if (!h) return 0;
    auto* s = (EuclidState*)h;
    return s->core.RayPick(x, y);
}

EUCLID_EXTERN_C EUCLID_API void EUCLID_CALL Euclid_SetSelection(EuclidHandle h, EuclidObjectID id){
    if (!h) return;
    auto* s = (EuclidState*)h;
    s->core.SetSelection(id);
}

EUCLID_EXTERN_C EUCLID_API int EUCLID_CALL Euclid_IsDraggingGizmo(EuclidHandle h){
    if (!h) return 0;
    auto* s = (EuclidState*)h;
    return s->core.IsDraggingGizmo() ? 1 : 0;
}

// ---- Custom mesh import (OBJ / raw) ----
EUCLID_EXTERN_C EUCLID_API EuclidResult EUCLID_CALL
Euclid_LoadOBJ(EuclidHandle h, const char* path, EuclidObjectID* out_id, int normalize)
{
    if (!h || !path || !out_id) return EUCLID_ERR_BAD_PARAM;
    auto* s = (EuclidState*)h;
    // Core must provide these methods; see note below.
    return s->core.LoadOBJ(path, out_id, normalize != 0);
}

EUCLID_EXTERN_C EUCLID_API EuclidResult EUCLID_CALL
Euclid_CreateFromRawMesh(EuclidHandle h,
                         const float* positions, size_t vertexCount,
                         const unsigned* indices, size_t indexCount,
                         EuclidObjectID* out_id, int normalize)
{
    if (!h || !positions || vertexCount==0 || !indices || indexCount<3 || !out_id)
        return EUCLID_ERR_BAD_PARAM;

    auto* s = (EuclidState*)h;
    return s->core.CreateFromRawMesh(positions, vertexCount,
                                     indices, indexCount,
                                     out_id, normalize != 0);
}
