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
 * 包含 LogCommandParser，用于向已连接的控制台发送日志
 *
 */

#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_LogCommandParser==1

#pragma once
#include "CommandParserInterface.h"
#include "Export.h"

namespace RakNet
{
/* 前向声明 */
class RakPeerInterface;

/* Adds the ability to send logging output to a remote console */
class RAK_DLL_EXPORT LogCommandParser : public CommandParserInterface
{
public:
	/* 获取单例 GetInstance() 和销毁单例 DestroyInstance(instance*) */
	STATIC_FACTORY_DECLARATIONS(LogCommandParser)

	LogCommandParser();
	~LogCommandParser() noexcept;

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

	/* All logs must be associated with a channel.  This is a filter so that remote clients only get logs for a system they care about. */
	/* If you call Log with a channel that is unknown, that channel will automatically be added */
	/* 参数[输入] channelName A persistent string naming the channel.  Don't deallocate this string. */
	void AddChannel(const char *channelName);

	/*
	 * Write a log to a channel.
	 * Logs are not buffered, so only remote consoles connected and subscribing at the time you write will get the output.
	 * 参数[输入] format 与 RAKNET_DEBUG_PRINTF() 相同
	 * 参数[输入] ... 与 RAKNET_DEBUG_PRINTF() 相同
	 */
	void WriteLog(const char *channelName, const char *format, ...);

	/*
	 * A callback for when systemAddress has connected to us.
	 * 参数[输入] systemAddress The player that has connected.
	 * 参数[输入] transport The transport interface that sent us this information.  Can be used to send messages to this or other players.
	 */
	void OnNewIncomingConnection(const SystemAddress &systemAddress, TransportInterface *transport) override;

	/*
	 * A callback for when systemAddress has disconnected, either gracefully or forcefully
	 * 参数[输入] systemAddress The player that has disconnected.
	 * 参数[输入] transport The transport interface that sent us this information.
	 */
	void OnConnectionLost(const SystemAddress &systemAddress, TransportInterface *transport) override;

	/*
	 * This is called every time transport interface is registered.  If you want to save a copy of the TransportInterface pointer
	 * 这是the place to do it
	 * 参数[输入] transport The new TransportInterface
	 */
	void OnTransportChange(TransportInterface *transport);
protected:
	/*
	 * Sends the currently active channels to the user
	 * 参数[输入] systemAddress The player to send to
	 * 参数[输入] transport The transport interface to use to send the channels
	 */
	void PrintChannels(const SystemAddress &systemAddress, TransportInterface *transport) const;

	/*
	 * Unsubscribe a user from a channel (or from all channels)
	 * 参数[输入] systemAddress The player to unsubscribe to
	 * 参数[输入] channelName If 0, then unsubscribe from all channels.  Otherwise unsubscribe from the named channel
	 */
	unsigned Unsubscribe(const SystemAddress &systemAddress, const char *channelName);

	/*
	 * Subscribe a user to a channel (or to all channels)
	 * 参数[输入] systemAddress The player to subscribe to
	 * 参数[输入] channelName If 0, then subscribe from all channels.  Otherwise subscribe to the named channel
	 */
	unsigned Subscribe(const SystemAddress &systemAddress, const char *channelName);

	/*
	 * Given the name of a channel, return the index into channelNames where it is located
	 * 参数[输入] channelName The name of the channel
	 */
	unsigned GetChannelIndexFromName(const char *channelName);

	/* One of these structures is created per player */
	struct SystemAddressAndChannel
	{
		/* The ID of the player */
		SystemAddress systemAddress;

		/* Bitwise representations of the channels subscribed to.  If bit 0 is set, then we subscribe to channelNames[0] and so on. */
		unsigned channels;
	};

	/* The list of remote users.  Added to when users subscribe, removed when they disconnect or unsubscribe */
	DataStructures::List<SystemAddressAndChannel> remoteUsers;

	/* Names of the channels at each bit, or 0 for an unused channel */
	const char *channelNames[32];

	/* This is so I can save the current transport provider, solely so I can use it without having the user pass it to Log */
	TransportInterface *trans;
};

} /* RakNet 命名空间 */

#endif
