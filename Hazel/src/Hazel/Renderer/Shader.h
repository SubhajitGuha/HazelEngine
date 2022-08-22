#pragma once
#include "Hazel/Log.h"
namespace Hazel {
	class Shader {
	public:
		Shader(std::string& vertexshader, std::string& fragmentshader);
		void init();
	};
}