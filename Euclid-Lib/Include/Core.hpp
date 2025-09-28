#pragma once

#include "Graphics.hpp"
#include "Objects.hpp"

#include "Glad/glad.h"
#include <iostream>

namespace Euclid
{
class Core {
public:
    bool Init(int width, int height, int gl_major, int gl_minor);
    void CleanUp();

    void Resize(int width, int height);
    void Update(float dtSeconds);
    void Render();
    
    void InitShader();
    void UseShader();
    
    // Gizmo Draw (0_o)
    void BuildTranslationGizmo(const glm::vec3& origin, float L);
    void BuildScaleTips(const glm::vec3& origin, float L);
    void DrawTranslationGizmo(const glm::mat4& viewProj,
                              const glm::vec3& originWS,
                              float lengthWorld,
                              float linePx);
    void DrawRotationGizmo(const glm::mat4& viewProj,
                           const glm::vec3& originWS,
                           float radiusWorld,
                           float ringPx,
                           int   segments);
    void DrawTransformationGizmo(const glm::mat4& viewProj,
                                 const glm::vec3& originWS,
                                 float lengthWorld,
                                 float tipSizePx);
    
    // Mouse input coming from the host via wrapper
    void OnMouseMove(double x, double y);                    // absolute window coords
    void OnMouseButton(int button, bool down, unsigned mods);
    void OnScroll(double dx, double dy);
    void OnMods(unsigned mods);
    
    // Scene API (called from wrapper)
    Object* CreateObject(EuclidShapeType t, const void* params, const EuclidTransform& xform, EuclidObjectID id);
    void    DestroyObjectGPU(EuclidObjectID id);
    void    SetSelection(EuclidObjectID id);
    EuclidObjectID RayPick(float x, float y);
    EuclidResult GetObjectTransform(EuclidObjectID id, EuclidTransform& out);
    EuclidResult SetObjectTransform(EuclidObjectID id, const EuclidTransform& in);
    void SetGizmoMode(EuclidGizmoMode m) { mGizmoMode = m; }
    void RequestRebuildScene();
    bool IsDraggingGizmo() const { return mDraggingGizmo; }

    
private:
    void DrawObject(const Object& o, const glm::mat4& view, const glm::mat4& proj);
    void DrawScene (const glm::mat4& view, const glm::mat4& proj);
    void DrawGizmoForSelection(const glm::mat4& viewProj);

    void EndGizmoDrag();
    
    void FocusOnObject(const Object& o, bool adjustRadius=false);
    bool mPivotFollowsSelection = true; // keep pivot on the selected object while dragging
    
private:
    int mWidth;
    int mHeight;
    unsigned int mVAO = 0;
    unsigned int mVBO = 0;
    unsigned int mDummyVAO = 0;
    unsigned int mTranslationVAO = 0;
    unsigned int mTransformationVAO = 0;
    unsigned int mTranslationVBO = 0;
    unsigned int mTransformationVBO = 0;
    
    Camera mainCamera;
    float mLastMouseX = 0.f, mLastMouseY = 0.f;
    bool  mFirstMouse = true;
    float mAngleX = 0.0f, mAngleY = 0.0f;        // current
    float mTargetAngleX = 0.0f, mTargetAngleY = 0.0f; // targets
    glm::mat4 mModel = glm::mat4(1.0f);
    
    Shader mainVertex;
    Shader mainFragment;
    ShaderProgram mainShader;
    
    Shader gridVertex;
    Shader gridFragment;
    ShaderProgram gridShader;
    
    Shader translationVertex;
    Shader translationFragment;
    ShaderProgram translationShader;
    
    Shader rotationVertex;
    Shader rotationFragment;
    ShaderProgram rotationShader;
    
    Shader transformationVertex;
    Shader transformationFragment;
    ShaderProgram transformationShader;
    
    // Objects Logic Data
    ObjectStore mObjs;
    
    // gizmo state
    EuclidGizmoMode mGizmoMode = EUCLID_GIZMO_TRANSLATE;
    bool        mDraggingGizmo=false;
    glm::vec2   mDragStartPx{0};
    EuclidObjectID mDragObj=0;
    
    enum class GizmoPart { None, MoveX, MoveY, MoveZ, TipX, TipY, TipZ, RotX, RotY, RotZ };

    GizmoPart  mActiveGizmo = GizmoPart::None;
    glm::vec3  mGizmoOriginWS{0.0f}; // selected object position
    float      mAxisT0 = 0.0f;       // param along axis at drag-begin (translate/scale)
    float      mAngle0 = 0.0f;       // angle at drag-begin (rotate)

    // helpers
    struct Ray { glm::vec3 o; glm::vec3 d; };
    Ray  ScreenRay(float px, float py, const glm::mat4& invViewProj) const;
    bool IntersectRayPlane(const Ray& r, const glm::vec3& p0, const glm::vec3& n, glm::vec3& hitWS) const;
    float WorldPerPixelAtViewZ(float absViewZ) const; // pixel→world scale

    // screen-space helpers
    bool WorldToPixel(const glm::vec3& ws, const glm::mat4& viewProj, glm::vec2& outPx) const;
    float DistPointToSegmentPx(const glm::vec2& p, const glm::vec2& a, const glm::vec2& b) const;

    // pick tests (match shader visuals)
    GizmoPart PickTranslateScale(float mousePxX, float mousePxY,
                                 const glm::mat4& view, const glm::mat4& proj,
                                 const glm::vec3& originWS,
                                 float lengthWorld, float linePx, float tipPx) const;

    GizmoPart PickRotate(float mousePxX, float mousePxY,
                         const glm::mat4& view, const glm::mat4& proj,
                         const glm::vec3& originWS,
                         float radiusWorld, float ringPx) const;
    
    // Gizmo orientation
    glm::mat3 mGizmoBasis { 1.0f };   // columns = local X,Y,Z of the selected object (unit)
    glm::vec3 mDragOriginWS{0.0f}; 
    glm::mat3 mDragBasis  { 1.0f };   // frozen basis used for the current drag
    float     mGizmoLength = 2.0f;    // visual length (same as you draw)

    // Helpers
    void UpdateGizmoBasisFromObject(const Object& o);
    void UpdateGizmoGeometry(const glm::vec3& origin, const glm::mat3& basis, float L);

    // drag routines use mActiveGizmo
    void BeginGizmoDrag(float px, float py);
    void UpdateGizmoDrag(float px, float py);
    
    glm::vec3 mDragAxis{0.0f};      // world axis of the active gizmo
    glm::vec3 mDragPlaneN{0.0f};    // plane normal used for translate mapping
};
}

