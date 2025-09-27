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
