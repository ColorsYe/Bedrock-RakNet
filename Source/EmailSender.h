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
 *  EmailSender.h
 * Rudimentary class to send email from code.  Don't expect anything fancy.
 *
 */

#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_EmailSender==1 && _RAKNET_SUPPORT_TCPInterface==1 && _RAKNET_SUPPORT_FileOperations==1

#pragma once
#include "RakNetTypes.h"
#include "RakMemoryOverride.h"
#include "Export.h"
#include "Rand.h"
#include "TCPInterface.h"

namespace RakNet
{
/* 前向声明 */
class FileList;
class TCPInterface;

/* Rudimentary class to send email from code. */
class RAK_DLL_EXPORT EmailSender
{
public:
	/* 获取单例 GetInstance() 和销毁单例 DestroyInstance(instance*) */
	STATIC_FACTORY_DECLARATIONS(EmailSender)

	/*
	 * 发送 an email
	 * 参数[输入] hostAddress The address of the email server.
	 * 参数[输入] hostPort The port of the email server (usually 25)
	 * 参数[输入] sender The email address you are sending from.
	 * 参数[输入] recipient The email address you are sending to.
	 * 参数[输入] senderName The email address you claim to be sending from
	 * 参数[输入] recipientName The email address you claim to be sending to
	 * 参数[输入] subject Email subject
	 * 参数[输入] body Email body
	 * 参数[输入] attachedFiles List of files to attach to the email. (Can be 0 to send 无).
	 * 参数[输入] doPrintf true to output SMTP info to console(for debugging?)
	 * 参数[输入] password Used if the server uses AUTHENTICATE PLAIN over TLS (such as gmail)
	 * 返回值: 0 on success, otherwise a string indicating the error message
	 */
	const char *Send(const char *hostAddress, unsigned short hostPort, const char *sender, const char *recipient, const char *senderName, const char *recipientName, const char *subject, const char *body, FileList *attachedFiles, bool doPrintf, const char *password);

protected:
	const char *GetResponse(TCPInterface *tcpInterface, const SystemAddress &emailServer, bool doPrintf);
	RakNetRandom rakNetRandom;
};

} /* RakNet 命名空间 */

#endif
