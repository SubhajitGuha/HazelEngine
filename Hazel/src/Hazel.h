#pragma once

//only used by hazel applications

#include "Hazel/Application.h"
#include "Hazel/Layer.h"
#include "Hazel/Log.h"
#include "Hazel/ImGui/ImGuiLayer.h"

#include "Hazel/Input.h"
#include "Hazel/HazelCodes.h"
#include "Hazel/ImGui/ImGuiLayer.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Component.h"

//-------Non Hazel api----------------
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "entt.hpp"

// ---Renderer------------------------
#include "Hazel/Renderer/Renderer.h"
#include "Hazel/Renderer/Renderer2D.h"
#include "Hazel/Renderer/RenderCommand.h"
#include "Hazel/Renderer/Buffer.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/OrthographicCamera.h"
#include "Hazel/OrthographicCameraController.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Renderer/SubTexture.h"
#include "Hazel/Renderer/FrameBuffer.h"
// -----------------------------------

//.....EntryPoint................................
//#include "Hazel/EntryPoint.h"
//...............................................