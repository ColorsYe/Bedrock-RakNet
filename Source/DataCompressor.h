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
 *  DataCompressor.h
 * DataCompressor does compression on a block of data.
 * Not very good compression, but it's small and fast so is something you can use per-message at runtime.
 *
 */


#pragma once
#include "RakMemoryOverride.h"
#include "DS_HuffmanEncodingTree.h"
#include "Export.h"

namespace RakNet
{

/* Does compression on a block of data.  Not very good compression, but it's small and fast so is something you can compute at runtime. */
class RAK_DLL_EXPORT DataCompressor
{
public:
	/* 获取单例 GetInstance() 和销毁单例 DestroyInstance(instance*) */
	STATIC_FACTORY_DECLARATIONS(DataCompressor)

	static void Compress( unsigned char *userData, unsigned sizeInBytes, RakNet::BitStream * output );
	static unsigned DecompressAndAllocate( RakNet::BitStream * input, unsigned char **output );
};

} /* RakNet 命名空间 */
