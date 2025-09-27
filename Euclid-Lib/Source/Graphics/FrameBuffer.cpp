#include "Graphics.hpp"

namespace Euclid
{
FrameBuffer::FrameBuffer(glm::vec2 size)
    : mFBO(0)
    , mTextureID(0)
    , mSize(size)
{
    // Create Frame Buffer
    glGenFramebuffers(1, &mFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    // Create Color Texture
    GenTexture();
    // Check FBO for completness
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Frame Buffer Is Not compiled";
}
FrameBuffer::~FrameBuffer() {
    
}
void FrameBuffer::SetSize(glm::vec2 size) {
    mSize = size;
}
uint32_t FrameBuffer::GetFBO() {
    return mFBO;
}
glm::vec2 FrameBuffer::GetSize() {
    return mSize;
}
uint32_t FrameBuffer::GetTextureID(){
    return mTextureID;
}
void FrameBuffer::GenTexture() {
    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_2D, mTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mSize.x, mSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureID, 0);
}
void FrameBuffer::BindBuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glClear(GL_COLOR_BUFFER_BIT);
}
void FrameBuffer::UnBindBuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);
}
}
