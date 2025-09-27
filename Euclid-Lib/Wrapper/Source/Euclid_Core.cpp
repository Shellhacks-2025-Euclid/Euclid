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
