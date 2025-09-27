#pragma once

// EUCLID_STATIC
#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(EUCLID_STATIC)
#define EUCLID_API
#else
#if defined(EUCLID_BUILD_DLL)
#define EUCLID_API __declspec(dllexport)
#elif defined(EUCLID_USE_DLL)
#define EUCLID_API __declspec(dllimport)
#else
#define EUCLID_API
#endif
#endif
#define EUCLID_CALL __cdecl          // C/PInvoke
#else
#if __GNUC__ >= 4
#define EUCLID_API __attribute__((visibility("default")))
#else
#define EUCLID_API
#endif
#define EUCLID_CALL
#endif

#ifdef __cplusplus
#define EUCLID_EXTERN_C extern "C"
#else
#define EUCLID_EXTERN_C
#endif
