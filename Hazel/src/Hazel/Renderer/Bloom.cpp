#include "hzpch.h"
#include "Bloom.h"
#include "Hazel/platform/Opengl/OpenGlBloom.h"

namespace Hazel {
    ref<Bloom> Bloom::Create()
    {
        switch (RendererAPI::GetAPI()) {
        case GraphicsAPI::OpenGL:
            return std::make_shared<OpenGlBloom>();
        case GraphicsAPI::None:
            return nullptr;
        default:
            return nullptr;
        }
    }
}