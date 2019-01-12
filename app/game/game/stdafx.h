// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#define _USE_MATH_DEFINES
#include <math.h>

class Math
{
public:
    inline static double degToRad(double deg) { return deg * M_PI / 180; }
    inline static double radToDeg(double rad) { return rad * 180 / M_PI; }
};

// TODO: reference additional headers your program requires here
