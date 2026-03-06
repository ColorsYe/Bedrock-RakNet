/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  本源代码使用 BSD 风格许可证授权，
 *  许可证文件位于源码树根目录的 LICENSE 文件中。
 *  同目录下的 PATENTS 文件中还包含额外的专利授权。
 *
 */

/*
 * 
 * 包含 ReplicaManager 系统使用的枚举定义。此文件是一个轻量级头文件，
 * 可以放心包含，无需担心引入大量其他依赖。
 *
 */



#pragma once
/*
 * 副本接口标志，用于启用和禁用 Replica 对象上的函数调用。
 * 传递给 ReplicaManager::EnableReplicaInterfaces 和 ReplicaManager::DisableReplicaInterfaces
 */
enum
{
	REPLICA_RECEIVE_DESTRUCTION=1<<0,    /* 接收销毁通知 */
	REPLICA_RECEIVE_SERIALIZE=1<<1,      /* 接收序列化数据 */
	REPLICA_RECEIVE_SCOPE_CHANGE=1<<2,   /* 接收作用域变更 */
	REPLICA_SEND_CONSTRUCTION=1<<3,      /* 发送构造通知 */
	REPLICA_SEND_DESTRUCTION=1<<4,       /* 发送销毁通知 */
	REPLICA_SEND_SCOPE_CHANGE=1<<5,      /* 发送作用域变更 */
	REPLICA_SEND_SERIALIZE=1<<6,         /* 发送序列化数据 */
	REPLICA_SET_ALL = 0xFF               /* 允许上述所有操作 */
};

enum ReplicaReturnResult
{
	/* 表示稍后用相同参数再次调用该函数 */
	REPLICA_PROCESS_LATER,
	/* 表示处理完成（正常的返回结果） */
	REPLICA_PROCESSING_DONE,
	/* 表示取消处理——不发送任何网络消息，也不改变当前状态 */
	REPLICA_CANCEL_PROCESS,
	/*
	 * 与 REPLICA_PROCESSING_DONE 相同，会发送消息，但不清除发送标志位。
	 * 适用于不同可靠性级别的多次分段发送。
	 * 目前仅被 Replica::Serialize 使用。
	 */
	REPLICA_PROCESS_AGAIN,
	/*
	 * 仅从 Replica::SendConstruction 接口返回，表示视为对方已拥有此对象，
	 * 但实际上不发送构造数据包。这样仍会向该系统发送作用域和序列化数据包。
	 */
	REPLICA_PROCESS_IMPLICIT
};
