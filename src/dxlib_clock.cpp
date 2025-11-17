

#include "dxlib_clock.h"

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>

CDxLibClock::CDxLibClock()
{
	Restart();
}

CDxLibClock::~CDxLibClock()
{

}

float CDxLibClock::GetElapsedTime()
{
	unsigned long long nFrequency = DxLib::GetSysPerformanceFrequency();
	unsigned long long nNow = GetNowCounter();
	return static_cast<float>(nNow - m_nLastCounter) / nFrequency;
}

void CDxLibClock::Restart()
{
	m_nLastCounter = GetNowCounter();
}

unsigned long long CDxLibClock::GetNowCounter()
{
	return DxLib::GetNowSysPerformanceCount();
}
