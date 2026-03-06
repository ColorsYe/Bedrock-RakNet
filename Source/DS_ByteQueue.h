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
 *  DS_ByteQueue.h
 * 内部使用
 * Byte queue
 *
 */


#pragma once
#include "RakMemoryOverride.h"
#include "Export.h"

/*
 * DataStructures 命名空间的添加仅是为了避免常见数据结构名称导致的编译器错误
 * 由于这些数据结构是独立的，如果需要，你可以在 RakNet 之外将它们用于自己的项目。
 */
namespace DataStructures
{
	class ByteQueue
	{
	public:
		ByteQueue();
		~ByteQueue() noexcept;
		void WriteBytes(const char *in, unsigned length, const char *file, unsigned int line);
		bool ReadBytes(char *out, unsigned maxLengthToRead, bool peek);
		unsigned GetBytesWritten(void) const;
		char* PeekContiguousBytes(unsigned int *outLength) const;
		void IncrementReadOffset(unsigned length);
		void DecrementReadOffset(unsigned length);
		void Clear(const char *file, unsigned int line);
		void Print();

	protected:
		char *data;
		unsigned readOffset, writeOffset, lengthAllocated;
	};
}
