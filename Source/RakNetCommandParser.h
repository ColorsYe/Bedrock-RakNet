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
 * 
 * 包含 RakNetCommandParser，用于向 RakPeer 实例发送命令
 *
 */

#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_RakNetCommandParser==1

#pragma once
#include "CommandParserInterface.h"
#include "Export.h"

namespace RakNet
{
class RakPeerInterface;

/* This allows a console client to call most of the functions in RakPeer */
class RAK_DLL_EXPORT RakNetCommandParser : public CommandParserInterface
{
public:
	/* 获取单例 GetInstance() 和销毁单例 DestroyInstance(instance*) */
	STATIC_FACTORY_DECLARATIONS(RakNetCommandParser)

	RakNetCommandParser();
	~RakNetCommandParser() noexcept;

	/*
	 * 给定带有参数 parameterList 的命令，执行你需要的任何处理。
	 * 参数[输入] command The command to process
	 * 参数[输入] numParameters How many parameters were passed along with the command
	 * 参数[输入] parameterList The list of parameters.  parameterList[0] is the first parameter and so on.
	 * 参数[输入] transport The transport interface we can use to write to
	 * 参数[输入] systemAddress The player that sent this command.
	 * 参数[输入] originalString The string that was actually sent over the network, in case you want to do your own parsing
	 */
	bool OnCommand(const char *command, unsigned numParameters, char **parameterList, TransportInterface *transport, const SystemAddress &systemAddress, const char *originalString) override;

	/*
	 * 你需要重写此函数并返回一个静态字符串来标识你的解析器。
	 * 应返回一个静态字符串
	 * 返回值: The name that you return.
	 */
	const char *GetName(void) const override;

	/*
	 * 当需要向 systemAddress 发送解析器简要描述时的回调
	 * 参数[输入] transport The transport interface we can use to write to
	 * 参数[输入] systemAddress The player that requested help.
	 */
	void SendHelp(TransportInterface *transport, const SystemAddress &systemAddress) override;

	/*
	 * Records the instance of RakPeer to perform the desired commands on
	 * 参数[输入] rakPeer The RakPeer instance, or a derived class (e.g. RakPeer or RakPeer)
	 */
	void SetRakPeerInterface(RakNet::RakPeerInterface *rakPeer);
protected:

	/* Which instance of RakPeer we are working on.  Set from SetRakPeerInterface() */
	RakPeerInterface *peer;
};

} /* RakNet 命名空间 */

#endif
