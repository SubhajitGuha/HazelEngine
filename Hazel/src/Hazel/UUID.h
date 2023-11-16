#pragma once
#include "Hazel.h"

namespace Hazel
{
	class UUID
	{
	public:
		UUID()
		{
			std::uniform_int_distribution<int> random;
			std::random_device rd;//seed
			std::default_random_engine generator(rd());//default engine

			m_uuid = random(generator);
		}
		operator uint64_t() { return m_uuid; }
	private:
		uint64_t m_uuid;
	};
}