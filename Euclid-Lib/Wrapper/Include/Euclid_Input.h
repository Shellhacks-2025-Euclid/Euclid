#pragma once
#include "Euclid_Export.h"
#include "Euclid_Types.h"

EUCLID_EXTERN_C EUCLID_API void EUCLID_CALL Euclid_OnMouseMove(EuclidHandle h, double x, double y);
EUCLID_EXTERN_C EUCLID_API void EUCLID_CALL Euclid_OnMouseButton(EuclidHandle h, EuclidMouseButton b, int down, EuclidMods mods);
EUCLID_EXTERN_C EUCLID_API void EUCLID_CALL Euclid_OnScroll(EuclidHandle h, double dx, double dy);
