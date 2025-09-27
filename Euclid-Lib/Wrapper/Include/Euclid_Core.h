#pragma once
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
