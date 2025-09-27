#include "Euclid_Input.h"
#include "State.hpp"

EUCLID_API void Euclid_OnMouseMove(EuclidHandle h, double x, double y)
{
    if (auto* s=(EuclidState*)h) s->core.OnMouseMove(x, y);
}
EUCLID_API void Euclid_OnMouseButton(EuclidHandle h, EuclidMouseButton b, int down, EuclidMods mods)
{
    if (auto* s=(EuclidState*)h) s->core.OnMouseButton((int)b, down!=0, (unsigned)mods);
}
EUCLID_API void Euclid_OnScroll(EuclidHandle h, double dx, double dy)
{
    if (auto* s=(EuclidState*)h) s->core.OnScroll(dx, dy);
}
EUCLID_API void Euclid_OnMods(EuclidHandle h, EuclidMods mods)
{
    if (auto* s = (EuclidState*)h) s->core.OnMods((unsigned)mods);
}
