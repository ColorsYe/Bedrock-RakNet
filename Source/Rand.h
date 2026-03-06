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
 * 
 * \b [内部使用] Random number generator
 *
 */



#pragma once
#include "Export.h"

/*
 * Initialise seed for Random Generator
 * 注意: 非线程安全, use an instance of RakNetRandom if necessary per thread
 * 参数[输入] seed The seed value for the random number generator.
 */
extern void RAK_DLL_EXPORT seedMT( unsigned int seed );

/*
 * 内部使用
 * 注意: 非线程安全, use an instance of RakNetRandom if necessary per thread
 */
extern unsigned int RAK_DLL_EXPORT reloadMT( void );

/*
 * 获取 a random unsigned int
 * 注意: 非线程安全, use an instance of RakNetRandom if necessary per thread
 * 返回值: an integer random value.
 */
extern unsigned int RAK_DLL_EXPORT randomMT( void );

/*
 * 获取 a random float
 * 注意: 非线程安全, use an instance of RakNetRandom if necessary per thread
 * 返回值: 0 to 1.0f, inclusive
 */
extern float RAK_DLL_EXPORT frandomMT( void );

/*
 * Randomizes a buffer
 * 注意: 非线程安全, use an instance of RakNetRandom if necessary per thread
 */
extern void RAK_DLL_EXPORT fillBufferMT( void *buffer, unsigned int bytes );

namespace RakNet {

/* Same thing as above functions, but not global */
class RAK_DLL_EXPORT RakNetRandom
{
public:
	RakNetRandom();
	~RakNetRandom() noexcept;
	void SeedMT( unsigned int seed );
	unsigned int ReloadMT( void );
	unsigned int RandomMT( void );
	float FrandomMT( void );
	void FillBufferMT( void *buffer, unsigned int bytes );

protected:
	unsigned int state[ 624 + 1 ];
	unsigned int *next;
	int left;
};

} /* RakNet 命名空间 */
