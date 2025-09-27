#include "Graphics.hpp"
#include "Core.hpp"
#include <vector>

namespace Euclid
{
void Core::InitShader() {
    mainVertex.Init(GL_VERTEX_SHADER,
    R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aColor;

    uniform mat4 uModel;
    uniform mat4 uView;
    uniform mat4 uProjection;

    out vec3 vColor;

    void main()
    {
        gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
        vColor = aColor;
    }
    )");

    mainFragment.Init(GL_FRAGMENT_SHADER,
    R"(
    #version 330 core
    in vec3 vColor;
    out vec4 FragColor;

    void main()
    {
        FragColor = vec4(vColor, 1.0);
    }
    )");
    
    gridVertex.Init(GL_VERTEX_SHADER,
    R"(
    #version 330 core
    // Fullscreen triangle — no VAO data needed
    const vec2 verts[3] = vec2[3](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );

    out vec2 v_ndc; // NDC in [-1,1]

    void main() {
        gl_Position = vec4(verts[gl_VertexID], 0.0, 1.0);
        v_ndc = verts[gl_VertexID];
    }

    )");

    gridFragment.Init(GL_FRAGMENT_SHADER,
    R"(
    #version 330 core
    in vec2 v_ndc;
    out vec4 FragColor;

    // camera
    uniform mat4 uInvViewProj;   // inverse(Proj * View)
    uniform mat4 uViewProj;      // Proj * View
    uniform vec3 uCamPos;

    // grid controls
    uniform float uMinor;
    uniform int   uMajorEvery;
    uniform float uLineWidth;
    uniform float uMajorMul;
    uniform float uFadeStart;
    uniform float uFadeEnd;

    // colors
    uniform vec3 uMinorColor;
    uniform vec3 uMajorColor;
    uniform vec3 uAxisXColor;    // X axis (z=0)
    uniform vec3 uAxisZColor;    // Z axis (x=0)

    // ---- helpers ----
    vec3 ndcToWorld(vec3 ndc) {
        vec4 w = uInvViewProj * vec4(ndc, 1.0);
        return w.xyz / w.w;
    }

    float lineCoveragePx(float distPx, float halfWidthPx) {
        float aa = 1.0;
        return 1.0 - smoothstep(halfWidthPx - aa, halfWidthPx + aa, distPx);
    }

    float distToMinor(float coord, float s) {
        float t = coord / s;
        return abs(fract(t) - 0.5) * s;
    }

    float distToMajor(float coord, float S) {
        float m = abs(mod(coord, S));
        return min(m, S - m);
    }

    void main() {
        // World ray
        vec3 pNear = ndcToWorld(vec3(v_ndc, -1.0));
        vec3 pFar  = ndcToWorld(vec3(v_ndc,  1.0));
        vec3 dir   = normalize(pFar - pNear);

        // Plane y=0
        float denom = dir.y;
        if (abs(denom) < 1e-6) { discard; }
        float t = -uCamPos.y / denom;
        if (t <= 0.0) { discard; }

        vec3 hit = uCamPos + dir * t;
        float distCam = distance(uCamPos, hit);

        // Depth write
        vec4 clip = uViewProj * vec4(hit, 1.0);
        float ndcZ = clip.z / clip.w;
        gl_FragDepth = ndcZ * 0.5 + 0.5;

        // Fade
        float fade = 1.0;
        if (uFadeEnd > uFadeStart) {
            fade = 1.0 - clamp((distCam - uFadeStart) / (uFadeEnd - uFadeStart), 0.0, 1.0);
        }

        // Pixel scale
        float dWorldPerPxX = max(length(vec2(dFdx(hit.x), dFdy(hit.x))), 1e-6);
        float dWorldPerPxZ = max(length(vec2(dFdx(hit.z), dFdy(hit.z))), 1e-6);
        float pxPerUnitX = 1.0 / dWorldPerPxX;
        float pxPerUnitZ = 1.0 / dWorldPerPxZ;

        float minor = max(uMinor, 1e-6);
        float majorStep = minor * float(max(uMajorEvery, 1));

        // Distances
        float gxPx = distToMinor(hit.x, minor) * pxPerUnitX;
        float gzPx = distToMinor(hit.z, minor) * pxPerUnitZ;
        float GXPx = distToMajor(hit.x, majorStep) * pxPerUnitX;
        float GZPx = distToMajor(hit.z, majorStep) * pxPerUnitZ;

        float axPx = abs(hit.z) * pxPerUnitZ;
        float azPx = abs(hit.x) * pxPerUnitX;

        // Coverages
        float halfMinor = max(uLineWidth * 0.5, 0.25);
        float halfMajor = halfMinor * max(uMajorMul, 1.0);

        float covMinor = max(lineCoveragePx(gxPx, halfMinor),
                             lineCoveragePx(gzPx, halfMinor));
        float covMajor = max(lineCoveragePx(GXPx, halfMajor),
                             lineCoveragePx(GZPx, halfMajor));
        float covAxisX = lineCoveragePx(axPx, halfMinor * 2.0);
        float covAxisZ = lineCoveragePx(azPx, halfMinor * 2.0);

        // Compose color & alpha
        vec3 col = vec3(0.0);
        float alpha = 0.0;

        col   = mix(col, uMinorColor, covMinor * 0.6);
        alpha = max(alpha, covMinor * 0.6);

        col   = mix(col, uMajorColor, covMajor);
        alpha = max(alpha, covMajor);

        col   = mix(col, uAxisXColor, covAxisX);
        alpha = max(alpha, covAxisX);

        col   = mix(col, uAxisZColor, covAxisZ);
        alpha = max(alpha, covAxisZ);

        // Distance fade applies to alpha too
        alpha *= fade;

        if (alpha <= 0.001) discard;

        FragColor = vec4(col, alpha);
    }
    )");
    // link programs
    std::vector<unsigned int> mainShaderIDs = { mainVertex.GetID(), mainFragment.GetID() };
    mainShader.Init(mainShaderIDs);

    std::vector<unsigned int> gridShaderIDs = { gridVertex.GetID(), gridFragment.GetID() };
    gridShader.Init(gridShaderIDs);

    // create dummy VAO (core profile needs some VAO bound)
    glGenVertexArrays(1, &mDummyVAO);
    
    glUseProgram(gridShader.GetID());
    glUniform1f(glGetUniformLocation(gridShader.GetID(),"uMinor"), 0.25f);
    glUniform1i(glGetUniformLocation(gridShader.GetID(),"uMajorEvery"), 10);
    glUniform1f(glGetUniformLocation(gridShader.GetID(),"uLineWidth"), 1.0f);
    glUniform1f(glGetUniformLocation(gridShader.GetID(),"uMajorMul"), 1.8f);
    glUniform1f(glGetUniformLocation(gridShader.GetID(),"uFadeStart"), 50.0f);
    glUniform1f(glGetUniformLocation(gridShader.GetID(),"uFadeEnd"), 150.0f);
    glUniform3f(glGetUniformLocation(gridShader.GetID(),"uMinorColor"), 0.52f,0.56f,0.62f);
    glUniform3f(glGetUniformLocation(gridShader.GetID(),"uMajorColor"), 0.70f,0.74f,0.80f);
    glUniform3f(glGetUniformLocation(gridShader.GetID(),"uAxisXColor"), 0.95f,0.35f,0.35f);
    glUniform3f(glGetUniformLocation(gridShader.GetID(),"uAxisZColor"), 0.35f,0.65f,0.95f);
    glUniform3f(glGetUniformLocation(gridShader.GetID(),"uBgColor"),    0.06f,0.07f,0.08f);
    glUseProgram(0);
}
void Core::UseShader() {
    mainShader.Use();
    gridShader.Use();
}
}

