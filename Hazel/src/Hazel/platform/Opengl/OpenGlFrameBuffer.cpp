#include "hzpch.h"
#include "OpenGlFrameBuffer.h"
#include "glad/glad.h"
#include "Hazel/Log.h"

namespace Hazel {
    OpenGlFrameBuffer::OpenGlFrameBuffer(const FrameBufferSpecification& spec)
    {
        invalidate(spec);
    }
    OpenGlFrameBuffer::~OpenGlFrameBuffer()
    {
        glDeleteFramebuffers(1, &m_RenderID);
        glDeleteTextures(1, &m_SceneTexture);
        glDeleteTextures(1, &m_DepthTexture);
    }
    void OpenGlFrameBuffer::Bind()
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_RenderID);
        HAZEL_CORE_ERROR(glGetError());

    }
    void OpenGlFrameBuffer::UnBind()
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
    void OpenGlFrameBuffer::Resize(unsigned int width, unsigned int height)
    {
        Specification.Width = width; 
        Specification.Height = height;
        invalidate(Specification);
        glViewport(0, 0, width, height);
    }
    void OpenGlFrameBuffer::ClearFrameBuffer()//this does nothing
    {
        float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        glClearBufferfv(GL_COLOR, 0, clearColor);
        glFlush();
    }
    void OpenGlFrameBuffer::BindFramebufferTexture(int slot)
    {
        glBindTextureUnit(slot, m_SceneTexture);
    }
    void OpenGlFrameBuffer::BindFramebufferDepthTexture(int slot)
    {
        glBindTextureUnit(slot, m_DepthTexture);
    }
    void OpenGlFrameBuffer::invalidate(const FrameBufferSpecification& spec)
    {
        if (m_RenderID)
        {
            glDeleteFramebuffers(1, &m_RenderID);
            glDeleteTextures(1, &m_SceneTexture);
            glDeleteTextures(1, &m_DepthTexture);
        }
        Specification = spec;

        glGenFramebuffers(1, &m_RenderID);
        glBindFramebuffer(GL_FRAMEBUFFER, m_RenderID);

        glGenTextures(1, &m_SceneTexture);//Create texture object
        glBindTexture(GL_TEXTURE_2D, m_SceneTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, spec.Width, spec.Height, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //glBindTextureUnit(SCENE_TEXTURE_SLOT, m_SceneTexture);
        glBindTexture(GL_TEXTURE_2D, 0);

        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_SceneTexture, 0);
        GLenum buffers[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, buffers);

        glGenTextures(1, &m_DepthTexture);
        glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, spec.Width, spec.Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);//unbind

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture, 0);


        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
            HAZEL_CORE_INFO("Scene FrameBuffer Compleate");
        else
            HAZEL_CORE_ERROR("Scene FrameBuffer inCompleate !!");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}