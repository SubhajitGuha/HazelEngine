#pragma once
#include "hzpch.h"
#include "Core.h"

namespace Hazel {

	class HAZEL_API Application { //set this class as dll export
	public:
		Application();

		virtual ~Application();
		
		void Run();

	};
	//define in client (not in engine dll)
	Application* CreateApplication();
	
}