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
 *  CommandParserInterface.h
 * Contains CommandParserInterface , from which you derive custom command parsers
 *
 */


#pragma once
#include "RakMemoryOverride.h"
#include "RakNetTypes.h"
#include "DS_OrderedList.h"
#include "Export.h"

namespace RakNet
{
/* 前向声明 */
class TransportInterface;

/*
 * 内部使用
 * Contains the information related to one command registered with RegisterCommand()
 * Implemented so I can have an automatic help system via SendCommandList()
 */
struct RAK_DLL_EXPORT RegisteredCommand
{
	const char *command;
	const char *commandHelp;
	unsigned char parameterCount;
};

/* List of commands registered with RegisterCommand() */
int RAK_DLL_EXPORT RegisteredCommandComp( const char* const & key, const RegisteredCommand &data );

/*
 * The interface used by command parsers.
 * CommandParserInterface provides a set of functions and interfaces that plug into the ConsoleServer class.
 * Each CommandParserInterface works at the same time as other interfaces in the system.
 */
class RAK_DLL_EXPORT CommandParserInterface
{
public:
	CommandParserInterface();
	virtual ~CommandParserInterface();

	/*
	 * 你需要重写此函数并返回一个静态字符串来标识你的解析器。
	 * 应返回一个静态字符串
	 * 返回值: The name that you return.
	 */
	virtual const char *GetName(void) const=0;

	/*
	 * A callback for when systemAddress has connected to us.
	 * 参数[输入] systemAddress The player that has connected.
	 * 参数[输入] transport The transport interface that sent us this information.  Can be used to send messages to this or other players.
	 */
	virtual void  OnNewIncomingConnection(const SystemAddress &systemAddress, TransportInterface *transport);

	/*
	 * A callback for when systemAddress has disconnected, either gracefully or forcefully
	 * 参数[输入] systemAddress The player that has disconnected.
	 * 参数[输入] transport The transport interface that sent us this information.
	 */
	virtual void OnConnectionLost(const SystemAddress &systemAddress, TransportInterface *transport);

	/*
	 * 当需要向 systemAddress 发送解析器简要描述时的回调
	 * 参数[输入] transport The transport interface we can use to write to
	 * 参数[输入] systemAddress The player that requested help.
	 */
	virtual void SendHelp(TransportInterface *transport, const SystemAddress &systemAddress)=0;

	/*
	 * 给定带有参数 parameterList 的命令，执行你需要的任何处理。
	 * 参数[输入] command The command to process
	 * 参数[输入] numParameters How many parameters were passed along with the command
	 * 参数[输入] parameterList The list of parameters.  parameterList[0] is the first parameter and so on.
	 * 参数[输入] transport The transport interface we can use to write to
	 * 参数[输入] systemAddress The player that sent this command.
	 * 参数[输入] originalString The string that was actually sent over the network, in case you want to do your own parsing
	 */
	virtual bool OnCommand(const char *command, unsigned numParameters, char **parameterList, TransportInterface *transport, const SystemAddress &systemAddress, const char *originalString)=0;

	/*
	 * This is called every time transport interface is registered.
	 * If you want to save a copy of the TransportInterface pointer
	 * 这是the place to do it
	 * 参数[输入] transport The new TransportInterface
	 */
	virtual void OnTransportChange(TransportInterface *transport);

	/*
	 * 内部使用
	 * Scan commandList and return the associated array
	 * 参数[输入] command The string to find
	 * 参数[输出] rc 包含 result of this operation
	 * 返回值: True if we found the command, false otherwise
	 */
	virtual bool GetRegisteredCommand(const char *command, RegisteredCommand *rc);

	/*
	 * 内部使用
	 * Goes through str, replacing the delineating character with 0's.
	 * 参数[输入] str The string sent by the transport interface
	 * 参数[输入] delineator The character to scan for to use as a delineator
	 * 参数[输入] delineatorToggle When encountered the delineator replacement is toggled on and off
	 * 参数[输出] numParameters How many pointers were written to parameterList
	 * 参数[输出] parameterList An array of pointers to characters.  Will hold pointers to locations inside str
	 * 参数[输入] parameterListLength How big the parameterList array is
	 */
	static void ParseConsoleString(char *str, const char delineator, unsigned char delineatorToggle, unsigned *numParameters, char **parameterList, unsigned parameterListLength);

	/*
	 * 内部使用
	 * Goes through the variable commandList and sends the command portion of each struct
	 * 参数[输入] transport The transport interface we can use to write to
	 * 参数[输入] systemAddress The player to write to
	 */
	virtual void SendCommandList(TransportInterface *transport, const SystemAddress &systemAddress);

	static constexpr unsigned char VARIABLE_NUMBER_OF_PARAMETERS = 255;

	/* Currently only takes static strings - doesn't make a copy of what you pass. */
	/* parameterCount is the number of parameters that the sender has to include with the command. */
	/* Pass 255 to parameterCount to indicate variable number of parameters */

	/*
	 * Registers a command.
	 * 参数[输入] parameterCount How many parameters your command requires.  If you want to accept a variable number of commands, pass CommandParserInterface::VARIABLE_NUMBER_OF_PARAMETERS
	 * 参数[输入] command A pointer to a STATIC string that has your command.  I keep a copy of the pointer here so don't deallocate the string.
	 * 参数[输入] commandHelp A pointer to a STATIC string that has the help information for your command.  I keep a copy of the pointer here so don't deallocate the string.
	 */
	virtual void RegisterCommand(unsigned char parameterCount, const char *command, const char *commandHelp);

	/*
	 * Just writes a string to the remote system based on the result ( res ) of your operation
	 * This is not necessary to call, but makes it easier to return results of function calls.
	 * 参数[输入] res The result to write
	 * 参数[输入] command The command that this result came from
	 * 参数[输入] transport The transport interface that will be written to
	 * 参数[输入] systemAddress The player this result will be sent to
	 */
	virtual void ReturnResult(bool res, const char *command, TransportInterface *transport, const SystemAddress &systemAddress);
	virtual void ReturnResult(char *res, const char *command, TransportInterface *transport, const SystemAddress &systemAddress);
	virtual void ReturnResult(SystemAddress res, const char *command, TransportInterface *transport, const SystemAddress &systemAddress);
	virtual void ReturnResult(int res, const char *command,TransportInterface *transport, const SystemAddress &systemAddress);

	/*
	 * Just writes a string to the remote system when you are calling a function that has no return value.
	 * This is not necessary to call, but makes it easier to return results of function calls.
	 * 参数[输入] res The result to write
	 * 参数[输入] command The command that this result came from
	 * 参数[输入] transport The transport interface that will be written to
	 * 参数[输入] systemAddress The player this result will be sent to
	 */
	virtual void ReturnResult(const char *command,TransportInterface *transport, const SystemAddress &systemAddress);

protected:
	DataStructures::OrderedList<const char*, RegisteredCommand, RegisteredCommandComp> commandList;
};

} /* RakNet 命名空间 */
