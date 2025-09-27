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
    std::vector<unsigned int> shaderIDs = { mainVertex.GetID(), mainFragment.GetID() };
    mainShader.Init(shaderIDs);
    
}
void Core::UseShader() {
    mainShader.Use();
}
}

