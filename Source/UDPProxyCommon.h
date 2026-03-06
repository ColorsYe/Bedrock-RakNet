/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  本源代码使用 BSD 风格许可证授权，
 *  许可证文件位于源码树根目录的 LICENSE 文件中。
 *  同目录下的 PATENTS 文件中还包含额外的专利授权。
 *
 */

#pragma once
/* 系统流程概述: */
/*
UDPProxyClient: 终端用户
UDPProxyServer: 公开服务器，使用 UDPForwarder 类为无法直接连接的终端用户路由消息。
UDPProxyCoordinator: 某处的服务器，由 RakNet 连接，用于维护 UDPProxyServer 列表。

UDPProxyServer
 启动时，登录到 UDPProxyCoordinator 并注册自身

UDPProxyClient
 希望打开到 X 的路由
 向 UDPProxyCoordinator 发送包含 X 和期望超时的消息
 等待成功或失败

UDPProxyCoordinator:
* 收到 openRouteRequest（打开路由请求）
 如果没有已注册的服务器，返回失败
 添加条目到内存
 chooseBestUDPProxyServer()（可重写，默认随机选择）
 查询该服务器以调用 StartForwarding()。返回成功或失败
 如果失败，从剩余列表中选择另一个服务器。如果没有剩余的，返回失败。否则返回成功。
* 断开连接:
 如果断开的系统是 openRouteRequest 上的待处理客户端，删除该请求
 如果断开的系统是 UDPProxyServer，从列表中移除。对于此服务器的每个待处理客户端，从剩余服务器中重新选择。
* 登录:
 添加到 UDPProxyServer 列表，如果设置了密码则验证密码
*/

/* 存储在 ID_UDP_PROXY_GENERAL 之后的第二个字节中 */
/* 这样做是为了避免 MessageIdentifiers.h 过于杂乱并超出单字节枚举的上限 */
enum UDPProxyMessages
{
	ID_UDP_PROXY_FORWARDING_SUCCEEDED,                              /* 转发成功 */
	ID_UDP_PROXY_FORWARDING_NOTIFICATION,                           /* 转发通知 */
	ID_UDP_PROXY_NO_SERVERS_ONLINE,                                 /* 没有在线服务器 */
	ID_UDP_PROXY_RECIPIENT_GUID_NOT_CONNECTED_TO_COORDINATOR,       /* 接收方 GUID 未连接到协调器 */
	ID_UDP_PROXY_ALL_SERVERS_BUSY,                                  /* 所有服务器繁忙 */
	ID_UDP_PROXY_IN_PROGRESS,                                       /* 正在处理中 */
	ID_UDP_PROXY_FORWARDING_REQUEST_FROM_CLIENT_TO_COORDINATOR,     /* 客户端到协调器的转发请求 */
	ID_UDP_PROXY_PING_SERVERS_FROM_COORDINATOR_TO_CLIENT,           /* 协调器到客户端的 ping 服务器请求 */
	ID_UDP_PROXY_PING_SERVERS_REPLY_FROM_CLIENT_TO_COORDINATOR,     /* 客户端到协调器的 ping 服务器回复 */
	ID_UDP_PROXY_FORWARDING_REQUEST_FROM_COORDINATOR_TO_SERVER,     /* 协调器到服务器的转发请求 */
	ID_UDP_PROXY_FORWARDING_REPLY_FROM_SERVER_TO_COORDINATOR,       /* 服务器到协调器的转发回复 */
	ID_UDP_PROXY_LOGIN_REQUEST_FROM_SERVER_TO_COORDINATOR,          /* 服务器到协调器的登录请求 */
	ID_UDP_PROXY_LOGIN_SUCCESS_FROM_COORDINATOR_TO_SERVER,          /* 协调器到服务器的登录成功 */
	ID_UDP_PROXY_ALREADY_LOGGED_IN_FROM_COORDINATOR_TO_SERVER,      /* 协调器到服务器：已登录 */
	ID_UDP_PROXY_NO_PASSWORD_SET_FROM_COORDINATOR_TO_SERVER,        /* 协调器到服务器：未设置密码 */
	ID_UDP_PROXY_WRONG_PASSWORD_FROM_COORDINATOR_TO_SERVER          /* 协调器到服务器：密码错误 */
};


/* UDP 转发器最大超时时间（10 分钟） */
#define UDP_FORWARDER_MAXIMUM_TIMEOUT (60000 * 10)
