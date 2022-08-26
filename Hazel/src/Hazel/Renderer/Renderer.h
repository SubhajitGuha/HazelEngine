#pragma once
#include "RenderCommand.h"
namespace Hazel {
	class Renderer {
	public:
		static void BeginScene(){}
		static void Submit(VertexArray& vertexarray) { RenderCommand::DrawIndex(vertexarray); }
		static void EndScene(){}

	};
}