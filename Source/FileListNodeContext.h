/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  本源代码使用 BSD 风格许可证授权，
 *  许可证文件位于源码树根目录的 LICENSE 文件中。
 *  同目录下的 PATENTS 文件中还包含额外的专利授权。
 *
 */

/* 文件列表节点上下文 */


#pragma once
#include "BitStream.h"

struct FileListNodeContext
{
	FileListNodeContext() {dataPtr=0; dataLength=0;}
	FileListNodeContext(unsigned char o, uint32_t f1, uint32_t f2, uint32_t f3) : op(o), flnc_extraData1(f1), flnc_extraData2(f2), flnc_extraData3(f3) {dataPtr=0; dataLength=0;}
	~FileListNodeContext() {}

	unsigned char op;
	uint32_t flnc_extraData1;
	uint32_t flnc_extraData2;
	uint32_t flnc_extraData3;
	void *dataPtr;
	unsigned int dataLength;
};

inline RakNet::BitStream& operator<<(RakNet::BitStream& out, FileListNodeContext& in)
{
	out.Write(in.op);
	out.Write(in.flnc_extraData1);
	out.Write(in.flnc_extraData2);
	out.Write(in.flnc_extraData3);
	return out;
}
inline RakNet::BitStream& operator>>(RakNet::BitStream& in, FileListNodeContext& out)
{
	in.Read(out.op);
	bool success = in.Read(out.flnc_extraData1);
	static_cast<void>(success);
	assert(success);
	success = in.Read(out.flnc_extraData2);
	static_cast<void>(success);
	assert(success);
	success = in.Read(out.flnc_extraData3);
	static_cast<void>(success);
	assert(success);
	return in;
}
