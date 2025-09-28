#pragma once
#include <stdint.h>
#include "Euclid_Export.h"

typedef void* EuclidHandle;

// host's GL loader
typedef void* (EUCLID_CALL* Euclid_GetProcAddr)(const char* name);

typedef struct {
    int width, height;
    int gl_major, gl_minor;
} EuclidConfig;

typedef uint8_t EuclidMods;
enum {
    EUCLID_MOD_NONE = 0,
    EUCLID_MOD_SHIFT = 1 << 0,
    EUCLID_MOD_CTRL = 1 << 1,
    EUCLID_MOD_ALT = 1 << 2,
    EUCLID_MOD_SUPER = 1 << 3
};

typedef enum {
    EUCLID_MOUSE_LEFT = 0,
    EUCLID_MOUSE_RIGHT = 1,
    EUCLID_MOUSE_MIDDLE = 2
} EuclidMouseButton;

typedef enum {
    EUCLID_OK = 0,
    EUCLID_ERR_INIT = -1,
    EUCLID_ERR_BAD_PARAM = -2,
    EUCLID_ERR_GLAD = -3
} EuclidResult;


// == Object & Gizmo ==
typedef uint64_t EuclidObjectID;

typedef enum {
    EUCLID_SHAPE_CUBE = 0,
    EUCLID_SHAPE_SPHERE = 1,
    EUCLID_SHAPE_TORUS = 2,
    EUCLID_SHAPE_PLANE = 3
} EuclidShapeType;

typedef enum {
    EUCLID_GIZMO_NONE = 0,
    EUCLID_GIZMO_TRANSLATE = 1,
    EUCLID_GIZMO_ROTATE = 2,
    EUCLID_GIZMO_SCALE = 3
} EuclidGizmoMode;

typedef struct {
    float position[3];  // x,y,z
    float rotation[3];  // euler degrees yaw(Y), pitch(X), roll(Z) or any convention you use
    float scale[3];     // sx,sy,sz
} EuclidTransform;

// Per shape param structs (kept small & POD for C ABI)
typedef struct { float size; } EuclidCubeParams;             // edge length
typedef struct { float radius; int slices; int stacks; } EuclidSphereParams;
typedef struct { float majorRadius; float minorRadius; int majorSeg; int minorSeg; } EuclidTorusParams;
typedef struct { float width; float height; } EuclidPlaneParams;

// Tagged param “blob” (caller passes pointer to the matching struct)
typedef struct {
    EuclidShapeType type;
    const void* params; // points to one of the structs above, matching 'type'
    EuclidTransform xform; // initial transform
} EuclidCreateShapeDesc;
