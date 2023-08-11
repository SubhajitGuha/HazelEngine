#include "Hazel.h"

namespace Hazel {
	class Atmosphere {
	public:
		static void RenderAtmosphere(Camera& camera , const float& atmosphere_radius= 6471e3);
		static void InitilizeAtmosphere();
		static void Atmosphere::RenderQuad(const glm::mat4& view, const glm::mat4& proj);
	private:
		static ref<Shader> atmosphere_shader;
	};
}