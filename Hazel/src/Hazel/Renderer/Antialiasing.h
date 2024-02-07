#pragma once
#include "Hazel.h"

namespace Hazel
{
	class Antialiasing
	{
	public:
		Antialiasing();
		~Antialiasing();
		//default width = 1920 and height = 1080
		static ref<Antialiasing> Create(int width = 1920, int height=1080);
		static glm::vec2 GetJitterOffset();
		virtual void Update() = 0;
	private:
		static int frameCount;
	};
}