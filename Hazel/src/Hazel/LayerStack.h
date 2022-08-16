#pragma once
#include "Core.h"
#include"Layer.h"

namespace Hazel {
	class HAZEL_API LayerStack
	{
	public:
		LayerStack();
		~LayerStack();
		void PushLayer(Layer* Layer);
		void PushOverlay(Layer* Overlay);
		void PopLayer(Layer* Layer);
		void PopOverlay(Layer*);

		inline std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
		inline std::vector<Layer*>::iterator end() { return m_Layers.end(); }
	private:
		std::vector<Layer*> m_Layers;
		unsigned int m_LayerInsertIndex=0;//iterator for consequtively storing the layers before the ovelay
	};
}

