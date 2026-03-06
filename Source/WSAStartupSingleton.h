/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  本源代码使用 BSD 风格许可证授权，
 *  许可证文件位于源码树根目录的 LICENSE 文件中。
 *  同目录下的 PATENTS 文件中还包含额外的专利授权。
 *
 */

/* WSAStartup 单例模式封装，确保 Winsock 只初始化和清理一次 */
#pragma once
class WSAStartupSingleton
{
public:
	WSAStartupSingleton();
	~WSAStartupSingleton() noexcept;
	/* 增加引用计数，首次调用时初始化 Winsock */
	static void AddRef();
	/* 减少引用计数，引用为零时清理 Winsock */
	static void Deref();

protected:
	static int refCount;
};
