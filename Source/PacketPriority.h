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
 * 此文件包含数据包优先级与可靠性的枚举定义。
 */



#pragma once
/* 这些枚举用于描述数据包的传送时机与优先级。 */
enum PacketPriority
{
	/* 最高优先级。此类消息立即触发发送，通常不会缓冲或合并到单个数据报中。 */
	IMMEDIATE_PRIORITY,

	/*
	 * 每发送 2 条 IMMEDIATE_PRIORITY 消息，发送 1 条 HIGH_PRIORITY 消息。
	 * 此优先级及更低优先级的消息会被缓冲，以 10 毫秒为间隔批量发送，
	 * 从而减少 UDP 开销并更好地进行拥塞控制测量。
	 */
	HIGH_PRIORITY,

	/*
	 * 每发送 2 条 HIGH_PRIORITY 消息，发送 1 条 MEDIUM_PRIORITY 消息。
	 * 此优先级及更低优先级的消息会被缓冲，以 10 毫秒为间隔批量发送，
	 * 从而减少 UDP 开销并更好地进行拥塞控制测量。
	 */
	MEDIUM_PRIORITY,

	/*
	 * 每发送 2 条 MEDIUM_PRIORITY 消息，发送 1 条 LOW_PRIORITY 消息。
	 * 此优先级及更低优先级的消息会被缓冲，以 10 毫秒为间隔批量发送，
	 * 从而减少 UDP 开销并更好地进行拥塞控制测量。
	 */
	LOW_PRIORITY,

	/* 内部使用 */
	NUMBER_OF_PRIORITIES
};

/*
 * 这些枚举用于描述数据包的传输可靠性。
 * 注意: 此枚举在流中占 3 位，若新增枚举值请同步修改对应代码。
 * 注意: 在 ReliabilityLayer::WriteToBitStreamFromInternalPacket 中，假定存在 5 种主要类型。
 * 注意: 请勿重新排序，代码中对 >= UNRELIABLE_WITH_ACK_RECEIPT 有比较判断。
 */
enum PacketReliability
{
	/*
	 * 与普通 UDP 相同，但会丢弃重复数据报。
	 * RakNet 附加 (6~17) + 21 比特的开销：其中 16 位用于检测重复包，6~17 位用于记录消息长度。
	 */
	UNRELIABLE,

	/*
	 * 带序列计数器的普通 UDP，乱序消息将被丢弃。
	 * 在同一通道上发送的有序和排序消息将按发送顺序到达。
	 */
	UNRELIABLE_SEQUENCED,

	/* 消息以可靠方式发送，但不保证顺序。开销与 UNRELIABLE 相同。 */
	RELIABLE,

	/*
	 * 消息可靠传输且按发送顺序到达，乱序消息将等待重传。开销与 UNRELIABLE_SEQUENCED 相同。
	 * 在同一通道上发送的有序和排序消息将按发送顺序到达。
	 */
	RELIABLE_ORDERED,

	/*
	 * 消息可靠传输且按发送序列到达，乱序消息将被丢弃。开销与 UNRELIABLE_SEQUENCED 相同。
	 * 在同一通道上发送的有序和排序消息将按发送顺序到达。
	 */
	RELIABLE_SEQUENCED,

	/*
	 * 与 UNRELIABLE 相同，但当调用 RakPeerInterface::Receive() 时，
	 * 用户会根据发送结果收到 ID_SND_RECEIPT_ACKED 或 ID_SND_RECEIPT_LOSS。
	 * 字节 1-4 包含 Send() 函数返回的消息编号。
	 * 断开连接或关闭时，所有未收到确认的消息应视为已丢失。
	 */
	UNRELIABLE_WITH_ACK_RECEIPT,

	/*
	 * 与 UNRELIABLE_SEQUENCED 相同，但当调用 RakPeerInterface::Receive() 时，
	 * 用户会根据发送结果收到 ID_SND_RECEIPT_ACKED 或 ID_SND_RECEIPT_LOSS。
	 * 字节 1-4 包含 Send() 函数返回的消息编号。断开连接或关闭时，所有未确认消息应视为丢失。
	 * 05/04/10 排序模式与 ACK 回执无法同时使用，因为无法得知对方是否丢弃了消息，即无从判断消息是否被处理。
	 */
	/* UNRELIABLE_SEQUENCED_WITH_ACK_RECEIPT, */

	/*
	 * 与 RELIABLE 相同。消息成功送达后，调用 RakPeerInterface::Receive() 时用户还会收到 ID_SND_RECEIPT_ACKED。
	 * ID_SND_RECEIPT_ACKED 在消息到达时返回，不一定按发送顺序。
	 * 字节 1-4 包含 Send() 函数返回的消息编号。
	 * 断开连接或关闭时，所有未确认消息应视为丢失。此模式不会返回 ID_SND_RECEIPT_LOSS。
	 */
	RELIABLE_WITH_ACK_RECEIPT,

	/*
	 * 与 RELIABLE_ORDERED 相同。消息成功送达后，调用 RakPeerInterface::Receive() 时用户还会收到 ID_SND_RECEIPT_ACKED。
	 * ID_SND_RECEIPT_ACKED 在消息到达时返回，不一定按发送顺序。
	 * 字节 1-4 包含 Send() 函数返回的消息编号。
	 * 断开连接或关闭时，所有未确认消息应视为丢失。此模式不会返回 ID_SND_RECEIPT_LOSS。
	 */
	RELIABLE_ORDERED_WITH_ACK_RECEIPT,

	/*
	 * 与 RELIABLE_SEQUENCED 相同。消息成功送达后，调用 RakPeerInterface::Receive() 时用户还会收到 ID_SND_RECEIPT_ACKED。
	 * 字节 1-4 包含 Send() 函数返回的消息编号。断开连接或关闭时，所有未确认消息应视为丢失。
	 * 05/04/10 排序模式与 ACK 回执无法同时使用，因为无法得知对方是否丢弃了消息，即无从判断消息是否被处理。
	 */
	/* RELIABLE_SEQUENCED_WITH_ACK_RECEIPT, */

	/* 内部使用 */
	NUMBER_OF_RELIABILITIES
};
