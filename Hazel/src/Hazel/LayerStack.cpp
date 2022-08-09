#include "hzpch.h"
#include "LayerStack.h"

namespace Hazel {
	LayerStack::LayerStack()
	{
		m_LayerInsert = m_Layers.begin();
	}

	LayerStack::~LayerStack()
	{
		for (auto i : m_Layers) {
			delete i;
		}
	}

	void LayerStack::PushLayer(Layer* Layer)
	{
		m_LayerInsert = m_Layers.emplace(m_LayerInsert, Layer);//Layers are pushed before the overlay
	}

	void LayerStack::PushOverlay(Layer* Overlay)
	{
		m_Layers.emplace_back(Overlay);//Overlays are always pushed back into a container
	}

	void LayerStack::PopLayer(Layer* Layer)
	{
		auto it = std::find(m_Layers.begin(), m_Layers.end(), Layer);
		if (it != m_Layers.end()) 
		{
			m_Layers.erase(it);
			m_LayerInsert--;
		}
	}

	void LayerStack::PopOverlay(Layer* Overlay)
	{
		auto it = std::find(m_Layers.begin(), m_Layers.end(), Overlay);
		m_Layers.erase(it);
	}
}