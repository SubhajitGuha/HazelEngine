#pragma once

#ifdef HZ_PLATFORM_WINDOWS
#include <windows.h>
#endif // HZ_PLATFORM_WINDOWS

//only used by hazel applications

#include "Hazel/Core.h"
#include "Hazel/UUID.h"
#include "Hazel/Application.h"
#include "Hazel/Layer.h"
#include "Hazel/Log.h"
#include "Hazel/ImGui/ImGuiLayer.h"

#include "Hazel/Input.h"
#include "Hazel/HazelCodes.h"
#include "Hazel/ImGui/ImGuiLayer.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Entity.h"
#include "Hazel/Scene/Component.h"
#include "Hazel/Scene/ScriptableEntity.h"
#include "Hazel/LoadMesh.h"

//-------Non Hazel api----------------
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/matrix_major_storage.hpp"
#include "entt.hpp"

// ---Renderer------------------------
#include "Hazel/Renderer/Renderer.h"
#include "Hazel/Renderer/Renderer2D.h"
#include "Hazel/Renderer/Renderer3D.h"
#include "Hazel/Renderer/RenderCommand.h"
#include "Hazel/Renderer/Buffer.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/OrthographicCamera.h"
#include "Hazel/OrthographicCameraController.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Renderer/SubTexture.h"
#include "Hazel/Renderer/FrameBuffer.h"
#include "Hazel/Renderer/Camareas/SceneCamera.h"
#include "Hazel/Renderer/Camareas/EditorCamera.h"
#include "Hazel/Renderer/Bloom.h"

// -----------------------------------
// 
//----------features------------------
#include "Hazel/Renderer/CubeMapEnvironment.h"

//.....EntryPoint................................
//#include "Hazel/EntryPoint.h"
//...............................................