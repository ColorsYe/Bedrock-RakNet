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
 *  ConsoleServer.h
 * Contains ConsoleServer , used to plugin to your game to accept remote console-based connections
 *
 */


#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_ConsoleServer==1

#pragma once
#include "RakMemoryOverride.h"
#include "DS_List.h"
#include "RakNetTypes.h"
#include "Export.h"

namespace RakNet
{
/* 前向声明 */
class TransportInterface;
class CommandParserInterface;


/*
 * The main entry point for the server portion of your remote console application support.
 * ConsoleServer takes one TransportInterface and one or more CommandParserInterface (s)
 * The TransportInterface will be used to send data between the server and the client.  The connecting client must support the
 * protocol used by your derivation of TransportInterface . TelnetTransport and RakNetTransport are two such derivations .
 * 当远程控制台发送命令时，该命令将由你的 CommandParserInterface 实现来处理
 */
class RAK_DLL_EXPORT ConsoleServer
{
public:
	/* 获取单例 GetInstance() 和销毁单例 DestroyInstance(instance*) */
	STATIC_FACTORY_DECLARATIONS(ConsoleServer)

	ConsoleServer();
	~ConsoleServer() noexcept;

	/*
	 * 使用 TransportInterface 的派生类调用此函数，以便控制台服务器可以发送和接收命令
	 * 参数[输入] transportInterface Your interface to use.
	 * 参数[输入] port The port to host on.  Telnet uses port 23 by default.  RakNet can use whatever you want.
	 */
	void SetTransportProvider(TransportInterface *transportInterface, unsigned short port);

	/*
	 * Add an implementation of CommandParserInterface to the list of command parsers.
	 * 参数[输入] commandParserInterface The command parser referred to
	 */
	void AddCommandParser(CommandParserInterface *commandParserInterface);

	/*
	 * Remove an implementation of CommandParserInterface previously added with AddCommandParser().
	 * 参数[输入] commandParserInterface The command parser referred to
	 */
	void RemoveCommandParser(CommandParserInterface *commandParserInterface);

	/*
	 * 调用 update 以读取从 TransportInterface 发送的数据包。
	 * You should do this fairly frequently.
	 */
	void Update();

	/*
	 * 将a prompt设置为show when waiting for user input
	 * Pass an empty string to clear the prompt
	 * 默认无提示
	 * 参数[输入] _prompt 以空字符结尾的字符串 of the prompt to use. If you want a newline, be sure to use /r/n
	 */
	void SetPrompt(const char *_prompt);

protected:
	void ListParsers(SystemAddress systemAddress);
	void ShowPrompt(SystemAddress systemAddress);
	TransportInterface *transport;
	DataStructures::List<CommandParserInterface *> commandParserList;
	char* password[256];
	char *prompt;
};

} /* RakNet 命名空间 */

#endif
