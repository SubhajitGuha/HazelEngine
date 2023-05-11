#include "hzpch.h"
#include "Camera.h"
#include "EditorCamera.h"
#include "SceneCamera.h"
namespace Hazel {

	ref<Camera> Camera::GetCamera(CameraType type)
	{
		switch (type)
		{
		case CameraType::EDITOR_CAMERA:
			return std::make_shared<EditorCamera>();
		case CameraType::SCENE_CAMERA:
			return std::make_shared<SceneCamera>();
		default:
			HAZEL_CORE_ERROR("Select a valid Camera Type");
			return nullptr;
		}
	}
}
