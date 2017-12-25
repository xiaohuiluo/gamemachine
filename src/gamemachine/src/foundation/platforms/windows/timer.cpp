﻿#include "stdafx.h"
#include "check.h"
#include "gamemachine.h"

extern "C"
{
	GMLargeInteger highResolutionTimerFrequency()
	{
		LARGE_INTEGER i;
		BOOL b = QueryPerformanceFrequency(&i);
		GM_ASSERT(b);
		return i.QuadPart;
	}

	GMLargeInteger highResolutionTimer()
	{
		LARGE_INTEGER i;
		BOOL b = QueryPerformanceCounter(&i);
		GM_ASSERT(b);
		return i.QuadPart;
	}
}