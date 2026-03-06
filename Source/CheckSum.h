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
 *  CheckSum.cpp
 * [内部使用] CheckSum implementation from http://www.flounder.com/checksum.htm
 *
 */

#pragma once
#include "RakMemoryOverride.h"

/* 生成和验证校验和 */
class CheckSum
{

public:
	
 /* 默认构造函数 */
	
	CheckSum()
	{
		Clear();
	}
	
	void Clear()
	{
		sum = 0;
		r = 55665;
		c1 = 52845;
		c2 = 22719;
	}
	
	void Add ( unsigned int w );
	
	
	void Add ( unsigned short w );
	
	void Add ( unsigned char* b, unsigned int length );
	
	void Add ( unsigned char b );
	
	unsigned int Get ()
	{
		return sum;
	}
	
protected:
	unsigned short r;
	unsigned short c1;
	unsigned short c2;
	unsigned int sum;
};
