/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  本源代码使用 BSD 风格许可证授权，
 *  许可证文件位于源码树根目录的 LICENSE 文件中。
 *  同目录下的 PATENTS 文件中还包含额外的专利授权。
 *
 */

/* 字节池 DS_BytePool.h */


#pragma once
#include "RakMemoryOverride.h"
#include "DS_MemoryPool.h"
#include "Export.h"
#include "SimpleMutex.h"
#include "RakAssert.h"

/* #define _DISABLE_BYTE_POOL */
/* #define _THREADSAFE_BYTE_POOL */

namespace DataStructures
{
	/* 从内存池中分配指定数量的字节。必要时使用堆内存。 */
	class RAK_DLL_EXPORT BytePool
	{
	public:
		BytePool();
		~BytePool() noexcept;
		/* 页面大小应至少为 8192 的 8 倍 */
		void SetPageSize(int size);
		unsigned char* Allocate(int bytesWanted, const char *file, unsigned int line);
		void Release(unsigned char *data, const char *file, unsigned int line);
		void Clear(const char *file, unsigned int line);
	protected:	
		MemoryPool<unsigned char[128]> pool128;
		MemoryPool<unsigned char[512]> pool512;
		MemoryPool<unsigned char[2048]> pool2048;
		MemoryPool<unsigned char[8192]> pool8192;
#ifdef _THREADSAFE_BYTE_POOL
		SimpleMutex mutex128;
		SimpleMutex mutex512;
		SimpleMutex mutex2048;
		SimpleMutex mutex8192;
#endif
	};
}
