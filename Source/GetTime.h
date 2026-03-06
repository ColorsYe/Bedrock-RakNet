/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/*
 *  GetTime.h
 * 返回 value from QueryPerformanceCounter.  This is the function RakNet uses to represent time. This time won't match the time returned by GetTimeCount(). See http://www.jenkinssoftware.com/forum/index.php?topic=2798.0
 *
 */


#pragma once
#include "Export.h"
#include "RakNetTime.h" /* For RakNet::TimeMS */

namespace RakNet
{
	/*
	 * 与 GetTimeMS 相同
	 * Holds the time in either a 32 or 64 bit variable, depending on __GET_TIME_64BIT
	 */
	RakNet::Time RAK_DLL_EXPORT GetTime( void );

	/*
	 * 返回 time as 32 bit
	 * 注意: The maximum delta between returned calls is 1 second - however, RakNet calls this constantly anyway. See NormalizeTime() in the cpp.
	 */
	RakNet::TimeMS RAK_DLL_EXPORT GetTimeMS( void );
	
	/*
	 * 返回 time as 64 bit
	 * 注意: The maximum delta between returned calls is 1 second - however, RakNet calls this constantly anyway. See NormalizeTime() in the cpp.
	 */
	RakNet::TimeUS RAK_DLL_EXPORT GetTimeUS( void );

	/* a > b? */
	extern RAK_DLL_EXPORT bool GreaterThan(RakNet::Time a, RakNet::Time b);
	/* a < b? */
	extern RAK_DLL_EXPORT bool LessThan(RakNet::Time a, RakNet::Time b);
}
