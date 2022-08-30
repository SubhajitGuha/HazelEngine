#pragma once
namespace Hazel {
	class TimeStep {
	public:
		TimeStep(float Time)
			:m_Time(Time)
		{
		}

		operator float() { return m_Time; }
		float GetTime() { return m_Time; }
		float GetTimeInMilliseconds() { return m_Time; }
	private:
		float m_Time;
	};
}