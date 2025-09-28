#pragma once

#include <stddef.h>
#include "Euclid_Export.h"
#include "Euclid_Types.h"

EUCLID_EXTERN_C EUCLID_API EuclidResult EUCLID_CALL Euclid_Create(const EuclidConfig* cfg, Euclid_GetProcAddr loader, EuclidHandle* out);
EUCLID_EXTERN_C EUCLID_API void         EUCLID_CALL Euclid_Destroy(EuclidHandle h);

EUCLID_EXTERN_C EUCLID_API void         EUCLID_CALL Euclid_Resize(EuclidHandle h, int w, int hgt);
EUCLID_EXTERN_C EUCLID_API void         EUCLID_CALL Euclid_Update(EuclidHandle h, float dt);
EUCLID_EXTERN_C EUCLID_API void         EUCLID_CALL Euclid_Render(EuclidHandle h);

EUCLID_EXTERN_C EUCLID_API void         EUCLID_CALL Euclid_SetFramebuffer(EuclidHandle h, unsigned int fb);

// diagnostics
EUCLID_EXTERN_C EUCLID_API const char* EUCLID_CALL Euclid_Version();
EUCLID_EXTERN_C EUCLID_API const char* EUCLID_CALL Euclid_GetLastError();   // thread-local last error string


// ---- Scene / Objects API ----
EUCLID_EXTERN_C EUCLID_API EuclidResult EUCLID_CALL Euclid_ClearScene(EuclidHandle h);

EUCLID_EXTERN_C EUCLID_API EuclidResult EUCLID_CALL
Euclid_CreateShape(EuclidHandle h, const EuclidCreateShapeDesc* desc, EuclidObjectID* out_id);

// ---- Custom mesh import ----
EUCLID_EXTERN_C EUCLID_API EuclidResult EUCLID_CALL
Euclid_LoadOBJ(EuclidHandle h, const char* path, EuclidObjectID* out_id, int normalize);

EUCLID_EXTERN_C EUCLID_API EuclidResult EUCLID_CALL
Euclid_CreateFromRawMesh(EuclidHandle h,
                         const float* positions, size_t vertexCount,
                         const unsigned* indices, size_t indexCount,
                         EuclidObjectID* out_id, int normalize);

EUCLID_EXTERN_C EUCLID_API EuclidResult EUCLID_CALL
Euclid_DeleteObject(EuclidHandle h, EuclidObjectID id);

EUCLID_EXTERN_C EUCLID_API EuclidResult EUCLID_CALL
Euclid_SelectObject(EuclidHandle h, EuclidObjectID id);

EUCLID_EXTERN_C EUCLID_API EuclidResult EUCLID_CALL
Euclid_GetSelection(EuclidHandle h, EuclidObjectID* out_id);

EUCLID_EXTERN_C EUCLID_API EuclidResult EUCLID_CALL
Euclid_SetGizmoMode(EuclidHandle h, EuclidGizmoMode mode);

EUCLID_EXTERN_C EUCLID_API EuclidGizmoMode EUCLID_CALL
Euclid_GetGizmoMode(EuclidHandle h);

// Viewport picking: x,y are pixel coords in the host window (same space you feed to OnMouseMove)
EUCLID_EXTERN_C EUCLID_API EuclidResult EUCLID_CALL
Euclid_HitTestSelect(EuclidHandle h, double x, double y, EuclidObjectID* out_id);

// Direct transform access (optional but handy)
EUCLID_EXTERN_C EUCLID_API EuclidResult EUCLID_CALL
Euclid_GetObjectTransform(EuclidHandle h, EuclidObjectID id, EuclidTransform* out_tf);

EUCLID_EXTERN_C EUCLID_API EuclidResult EUCLID_CALL
Euclid_SetObjectTransform(EuclidHandle h, EuclidObjectID id, const EuclidTransform* tf);

EUCLID_EXTERN_C EUCLID_API EuclidObjectID EUCLID_CALL Euclid_RayPick(EuclidHandle h, float x, float y); // returns 0 if none
EUCLID_EXTERN_C EUCLID_API void           EUCLID_CALL Euclid_SetSelection(EuclidHandle h, EuclidObjectID id);
EUCLID_EXTERN_C EUCLID_API int            EUCLID_CALL Euclid_IsDraggingGizmo(EuclidHandle h);           // 0/1

EUCLID_EXTERN_C EUCLID_API EuclidResult Euclid_DeleteObject(EuclidHandle h, EuclidObjectID id);

