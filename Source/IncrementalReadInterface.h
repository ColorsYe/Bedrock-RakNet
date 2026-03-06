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
#include "FileListNodeContext.h"
#include "Export.h"

namespace RakNet
{

class RAK_DLL_EXPORT IncrementalReadInterface
{
public:
	IncrementalReadInterface() {}
	virtual ~IncrementalReadInterface() {}

	/*
	 * Read part of a file into destination
	 * 返回 number of bytes written. Return 0 when file is done
	 * 参数[输入] filename Filename to read
	 * 参数[输入] startReadBytes What offset from the start of the file to read from
	 * 参数[输入] numBytesToRead How many bytes to read. This is also how many bytes have been allocated to preallocatedDestination
	 * 参数[输出] preallocatedDestination 写入 your data here
	 * 返回值: The number of bytes read, or 0 if 无
	 */
	virtual unsigned int GetFilePart( const char *filename, unsigned int startReadBytes, unsigned int numBytesToRead, void *preallocatedDestination, FileListNodeContext context);
};

} /* RakNet 命名空间 */
