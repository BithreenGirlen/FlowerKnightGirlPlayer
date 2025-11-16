

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
	long long nNow = GetNowCounter();
	return (nNow - m_nLastCounter) / 1000.f;
}

void CDxLibClock::Restart()
{
	m_nLastCounter = GetNowCounter();
}

long long CDxLibClock::GetNowCounter()
{
	return DxLib::GetNowHiPerformanceCount();
}
