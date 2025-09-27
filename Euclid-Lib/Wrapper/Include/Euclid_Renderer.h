#pragma once
#include "Euclid_Export.h"
#include "Euclid_Types.h"

typedef struct {
    float fps;
    int   draw_calls;
    int   triangles;
} EuclidStats;

EUCLID_EXTERN_C EUCLID_API void EUCLID_CALL Euclid_GetStats(EuclidHandle h, EuclidStats* out_stats);
