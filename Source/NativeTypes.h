/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#pragma once
#if defined(__GNUC__) || defined(__GCCXML__) || defined(__SNC__) || defined(__S3E__)
#include <stdint.h>
#elif !defined(_STDINT_H) && !defined(_SN_STDINT_H) && !defined(_SYS_STDINT_H_) && !defined(_STDINT) && !defined(_MACHTYPES_H_) && !defined(_STDINT_H_)
	using uint8_t = unsigned char;
	using uint16_t = unsigned short;
	using uint32_t = unsigned __int32;
	using int8_t = signed char;
	using int16_t = signed short;
	using int32_t = __int32;
	#if defined(_MSC_VER) && _MSC_VER < 1300
		using uint64_t = unsigned __int64;
		using int64_t = signed __int64;
	#else
		using uint64_t = unsigned long long int;
		using int64_t = signed long long;
	#endif
#endif
