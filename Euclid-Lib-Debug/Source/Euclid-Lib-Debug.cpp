#include <GLFW/glfw3.h>
#include "Euclid.h"

static EuclidHandle H = nullptr;
static void* Loader(const char* n){ return (void*)glfwGetProcAddress(n); }
static uint8_t Mods(int m) {
    uint8_t r=0;
    if(m&GLFW_MOD_SHIFT) r|=1;
    if(m&GLFW_MOD_CONTROL)r|=2;
    if(m&GLFW_MOD_ALT)r|=4;
    if(m&GLFW_MOD_SUPER)r|=8;
    return r;
}

int main(){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GL_TRUE);
#endif
    GLFWwindow* win=glfwCreateWindow(800,600,"Euclid Test",nullptr,nullptr);
    glfwMakeContextCurrent(win);
    glfwSwapInterval(1);

    EuclidConfig cfg{800, 600, 3, 3};
    if (Euclid_Create(&cfg, Loader, &H) != EUCLID_OK) return -1;

    glfwSetCursorPosCallback(win, [](GLFWwindow*,double x,double y){ Euclid_OnMouseMove(H,x,y); });
    glfwSetMouseButtonCallback(win, [](GLFWwindow*,int b,int a,int m){ Euclid_OnMouseButton(H,(EuclidMouseButton)b,a!=GLFW_RELEASE,Mods(m)); });
    glfwSetScrollCallback(win, [](GLFWwindow*,double dx,double dy){ Euclid_OnScroll(H,dx,dy); });
    glfwSetFramebufferSizeCallback(win, [](GLFWwindow*,int w,int h){ Euclid_Resize(H,w,h); });

    double last=glfwGetTime();
    while(!glfwWindowShouldClose(win)){
        double now=glfwGetTime(); float dt=float(now-last); last=now;
        Euclid_Update(H, dt);
        Euclid_Render(H);
        glfwSwapBuffers(win);
        glfwPollEvents();
    }
    Euclid_Destroy(H);
    glfwTerminate();
}
