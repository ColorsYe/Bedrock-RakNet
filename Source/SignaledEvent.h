/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  本源代码使用 BSD 风格许可证授权，
 *  许可证文件位于源码树根目录的 LICENSE 文件中。
 *  同目录下的 PATENTS 文件中还包含额外的专利授权。
 *
 */

/* 跨平台的信号事件封装 */
#pragma once
#if   defined(_WIN32)
#include "WindowsIncludes.h"



#else
	#include <pthread.h>
	#include <sys/types.h>
	#include "SimpleMutex.h"




#endif

#include "Export.h"

namespace RakNet
{

class RAK_DLL_EXPORT SignaledEvent
{
public:
	SignaledEvent();
	~SignaledEvent() noexcept;

	/* 初始化事件对象 */
	void InitEvent();
	/* 关闭事件对象 */
	void CloseEvent();
	/* 设置事件为已触发状态 */
	void SetEvent();
	/* 等待事件被触发，超时时间为毫秒 */
	void WaitOnEvent(int timeoutMs);

protected:
#ifdef _WIN32
	HANDLE eventList;




#else
	SimpleMutex isSignaledMutex;
	bool isSignaled;
#if !defined(ANDROID)
	pthread_condattr_t condAttr;
#endif
	pthread_cond_t eventList;
	pthread_mutex_t hMutex;
	pthread_mutexattr_t mutexAttr;
#endif
};

} /* RakNet 命名空间 */
