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
 * 保存 RakNet 返回的所有统计数据的结构体。
 *
 */



#pragma once
#include "PacketPriority.h"
#include "Export.h"
#include "RakNetTypes.h"

namespace RakNet
{

enum RNSPerSecondMetrics
{
	/* 通过调用 RakPeerInterface::Send() 推送的字节数 */
	USER_MESSAGE_BYTES_PUSHED,

	/*
	 * 通过调用 RakPeerInterface::Send() 实际发送的用户消息字节数。该值小于或等于 USER_MESSAGE_BYTES_PUSHED。
	 * 消息可能已被推送，但由于拥塞控制尚未发送
	 */
	USER_MESSAGE_BYTES_SENT,

	/* 重发的用户消息字节数。若消息被标记为可靠但未到达，或确认未到达，则会重发该消息。 */
	USER_MESSAGE_BYTES_RESENT,

	/* 已接收并成功返回给用户的用户消息字节数。 */
	USER_MESSAGE_BYTES_RECEIVED_PROCESSED,

	/* 已接收但因数据格式错误而被忽略的用户消息字节数。通常为 0。 */
	USER_MESSAGE_BYTES_RECEIVED_IGNORED,

	/* 实际发送的总字节数，包括每条消息和每个数据报的开销，以及可靠消息的确认（ACK） */
	ACTUAL_BYTES_SENT,

	/* 实际接收的总字节数，包括开销和 ACK。 */
	ACTUAL_BYTES_RECEIVED,

	/* 内部使用 */
	RNS_PER_SECOND_METRICS_COUNT
};

/*
 * 网络统计信息用法
 *
 * 存储与网络使用情况相关的统计数据
 */
struct RAK_DLL_EXPORT RakNetStatistics
{
	/* 对于 RNSPerSecondMetrics 中的每种类型，过去 1 秒内的值是多少？ */
	uint64_t valueOverLastSecond[RNS_PER_SECOND_METRICS_COUNT];

	/* 对于 RNSPerSecondMetrics 中的每种类型，连接生命周期内的总值是多少？ */
	uint64_t runningTotal[RNS_PER_SECOND_METRICS_COUNT];

	/*
	 * 连接何时开始？
	 * 另见 RakNet::GetTimeUS()
	 */
	RakNet::TimeUS connectionStartTime;

	/*
	 * 当前发送速率是否受到拥塞控制的限制？
	 * 若每秒发送的数据量超过带宽容量，此值应为 true
	 */
	bool isLimitedByCongestionControl;

	/* 若 isLimitedByCongestionControl 为 true，限制值（字节/秒）是多少？ */
	uint64_t BPSLimitByCongestionControl;

	/* 当前发送速率是否受到 RakPeer::SetPerConnectionOutgoingBandwidthLimit() 调用的限制？ */
	bool isLimitedByOutgoingBandwidthLimit;

	/* 若 isLimitedByOutgoingBandwidthLimit 为 true，限制值（字节/秒）是多少？ */
	uint64_t BPSLimitByOutgoingBandwidthLimit;

	/* 每个优先级级别有多少消息正在等待发送？ */
	unsigned int messageInSendBuffer[NUMBER_OF_PRIORITIES];

	/* 每个优先级级别有多少字节正在等待发送？ */
	double bytesInSendBuffer[NUMBER_OF_PRIORITIES];

	/*
	 * 重发缓冲区中有多少消息正在等待？这包括等待确认的消息，因此通常应为较小的值
	 * 若该值随时间增长，说明超过了带宽容量。请参见 BPSLimitByCongestionControl
	 */
	unsigned int messagesInResendBuffer;

	/* 重发缓冲区中有多少字节正在等待。另见 messagesInResendBuffer */
	uint64_t bytesInResendBuffer;

	/* 过去一秒内的丢包率？该数值范围从 0.0（无丢包）到 1.0（100% 丢包） */
	float packetlossLastSecond;

	/* 连接生命周期内的平均总丢包率是多少？ */
	float packetlossTotal;

	RakNetStatistics& operator +=(const RakNetStatistics& other)
	{
		unsigned i;
		for (i=0; i < NUMBER_OF_PRIORITIES; i++)
		{
			messageInSendBuffer[i]+=other.messageInSendBuffer[i];
			bytesInSendBuffer[i]+=other.bytesInSendBuffer[i];
		}

		for (i=0; i < RNS_PER_SECOND_METRICS_COUNT; i++)
		{
			valueOverLastSecond[i]+=other.valueOverLastSecond[i];
			runningTotal[i]+=other.runningTotal[i];
		}

		return *this;
	}
};

/*
 * 详细程度目前支持 0（低）、1（中）、2（高）
 * 参数[输入] s 要格式化输出的统计信息
 * 参数[输入] buffer 包含格式化报告的缓冲区
 * 参数[输入] verbosityLevel
 * 0 低
 * 1 中
 * 2 高
 * 3 调试拥塞控制
 */
void RAK_DLL_EXPORT StatisticsToString( RakNetStatistics *s, char *buffer, int verbosityLevel );

} /* RakNet 命名空间 */
