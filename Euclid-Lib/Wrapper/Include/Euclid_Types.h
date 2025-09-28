#pragma once
#include <stdint.h>
#include "Euclid_Export.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* EuclidHandle;

// Host's GL loader
typedef void* (EUCLID_CALL* Euclid_GetProcAddr)(const char* name);

typedef struct {
    int width, height;
    int gl_major, gl_minor;
} EuclidConfig;

// ---- Modifiers ----
typedef uint8_t EuclidMods;
enum {
    EUCLID_MOD_NONE  = 0,
    EUCLID_MOD_SHIFT = 1 << 0,
    EUCLID_MOD_CTRL  = 1 << 1,
    EUCLID_MOD_ALT   = 1 << 2,
    EUCLID_MOD_SUPER = 1 << 3
};

// ---- Mouse ----
typedef enum {
    EUCLID_MOUSE_LEFT   = 0,
    EUCLID_MOUSE_RIGHT  = 1,
    EUCLID_MOUSE_MIDDLE = 2
} EuclidMouseButton;

// ---- Results ----
typedef enum {
    EUCLID_OK        =  0,
    EUCLID_ERR_INIT  = -1,
    EUCLID_ERR_BAD_PARAM = -2,
    EUCLID_ERR_GLAD  = -3
} EuclidResult;

// =======================
// == Objects & Gizmos ==
// =======================
typedef uint64_t EuclidObjectID;

// Keep existing IDs stable; append new ones
typedef enum {
    EUCLID_SHAPE_CUBE     = 0,
    EUCLID_SHAPE_SPHERE   = 1,
    EUCLID_SHAPE_TORUS    = 2,
    EUCLID_SHAPE_PLANE    = 3,
    EUCLID_SHAPE_CONE     = 4,
    EUCLID_SHAPE_CYLINDER = 5,
    EUCLID_SHAPE_PRISM    = 6,  // regular n-gon prism (n>=3)
    EUCLID_SHAPE_CIRCLE   = 7   // filled disc in XZ
} EuclidShapeType;

typedef enum {
    EUCLID_GIZMO_NONE      = 0,
    EUCLID_GIZMO_TRANSLATE = 1,
    EUCLID_GIZMO_ROTATE    = 2,
    EUCLID_GIZMO_SCALE     = 3
} EuclidGizmoMode;

typedef struct {
    float position[3];  // x,y,z
    float rotation[3];  // euler degrees (your convention)
    float scale[3];     // sx,sy,sz
} EuclidTransform;

// ==========================
// == Per-shape parameters ==
// ==========================
// All POD & C-ABI friendly. Many engines ignore some fields and use transform scale;
// these give callers a semantic way to request specific tessellation/sizes.

// Cube: edge length
typedef struct { float size; } EuclidCubeParams;

// Sphere (UV): radius + tessellation
typedef struct { float radius; int slices; int stacks; } EuclidSphereParams;

// Torus: major/minor radii + tessellation
typedef struct {
    float majorRadius;   // ring radius (center to tube center)
    float minorRadius;   // tube radius
    int   majorSeg;      // around ring
    int   minorSeg;      // around tube
} EuclidTorusParams;

// Plane (XZ): dimensions
typedef struct { float width; float height; } EuclidPlaneParams;

// Cone (axis Y): base radius, height, segments (>=3)
typedef struct { float radius; float height; int segments; } EuclidConeParams;

// Cylinder (axis Y): radius, height, segments (>=3)
typedef struct { float radius; float height; int segments; } EuclidCylinderParams;

// Regular n-gon prism (axis Y): sides (>=3), circumscribed radius, height
typedef struct { int sides; float radius; float height; } EuclidPrismParams;

// Circle (filled disc in XZ): radius, segments (>=3)
typedef struct { float radius; int segments; } EuclidCircleParams;

// Tagged param “blob” (caller passes pointer to matching struct)
// Note: Implementations may ignore params and use unit meshes + transform scale.
typedef struct {
    EuclidShapeType type;
    const void*     params; // points to one of the structs above matching 'type'
    EuclidTransform xform;  // initial transform
} EuclidCreateShapeDesc;

#ifdef __cplusplus
} // extern "C"
#endif
