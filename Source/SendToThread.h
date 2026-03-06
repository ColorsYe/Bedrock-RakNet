/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  本源代码使用 BSD 风格许可证授权，
 *  许可证文件位于源码树根目录的 LICENSE 文件中。
 *  同目录下的 PATENTS 文件中还包含额外的专利授权。
 *
 */

/* 线程化发送支持 */
#pragma once
#include "RakNetDefines.h"

#ifdef USE_THREADED_SEND

#include "InternalPacket.h"
#include "SocketLayer.h"
#include "DS_ThreadsafeAllocatingQueue.h"
#include "ThreadPool.h"

namespace RakNet
{
class SendToThread
{
public:
	SendToThread();
	~SendToThread() noexcept;

	/* 发送线程的数据块结构 */
	struct SendToThreadBlock
	{
		SOCKET s;
		SystemAddress systemAddress;
		unsigned short remotePortRakNetWasStartedOn_PS3;
		unsigned int extraSocketOptions;
		char data[MAXIMUM_MTU_SIZE];
		unsigned short dataWriteOffset;
	};

	/* 从队列中分配一个数据块 */
	static SendToThreadBlock* AllocateBlock();
	/* 处理一个发送数据块 */
	static void ProcessBlock(SendToThreadBlock* threadedSend);

	/* 增加引用计数 */
	static void AddRef();
	/* 减少引用计数 */
	static void Deref();
	static DataStructures::ThreadsafeAllocatingQueue<SendToThreadBlock> objectQueue;
protected:
	static int refCount;
	static ThreadPool<SendToThreadBlock*,SendToThreadBlock*> threadPool;

};
}


#endif
