#include "Hazel.h"

namespace Hazel {
	class Atmosphere {
	public:
		static void RenderAtmosphere(Camera& camera , const float& atmosphere_radius);
		static void InitilizeAtmosphere();
		static void Atmosphere::RenderQuad();
	private:
		static ref<Shader> atmosphere_shader;
	};
}