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
http://www.ssfnet.org/Exchange/tcp/tcpTutorialNotes.html

cwnd=一次允许的最大字节数

Start:
cwnd=mtu
ssthresh=无限制

Slow start:
On ack cwnd*=2

congestion avoidance:
On ack during new period
cwnd+=mtu*mtu/cwnd

on loss or duplicate ack during period:
sshtresh=cwnd/2
cwnd=MTU
这将重新进入慢启动阶段

如果cwnd < ssthresh，则then use slow start
else use congestion avoidance


*/

#include "RakNetDefines.h"

#if USE_SLIDING_WINDOW_CONGESTION_CONTROL==1

#pragma once
#include "NativeTypes.h"
#include "RakNetTime.h"
#include "RakNetTypes.h"
#include "DS_Queue.h"

/* UDP 头部大小（字节） */
static constexpr unsigned int UDP_HEADER_SIZE = 28;

#define CC_DEBUG_PRINTF_1(x)
#define CC_DEBUG_PRINTF_2(x,y)
#define CC_DEBUG_PRINTF_3(x,y,z)
#define CC_DEBUG_PRINTF_4(x,y,z,a)
#define CC_DEBUG_PRINTF_5(x,y,z,a,b)
/* #define CC_DEBUG_PRINTF_1(x) printf(x) */
/* #define CC_DEBUG_PRINTF_2(x,y) printf(x,y) */
/* #define CC_DEBUG_PRINTF_3(x,y,z) printf(x,y,z) */
/* #define CC_DEBUG_PRINTF_4(x,y,z,a) printf(x,y,z,a) */
/* #define CC_DEBUG_PRINTF_5(x,y,z,a,b) printf(x,y,z,a,b) */

/* 若使用 iPod Touch TG，请将此值设为 4。*/
#define CC_TIME_TYPE_BYTES 8

#if CC_TIME_TYPE_BYTES==8
using CCTimeType = RakNet::TimeUS;
#else
using CCTimeType = RakNet::TimeMS;
#endif

using DatagramSequenceNumberType = RakNet::uint24_t;
using BytesPerMicrosecond = double;
using BytesPerSecond = double;
using MicrosecondsPerByte = double;

namespace RakNet
{

class CCRakNetSlidingWindow
{
	public:
	
	CCRakNetSlidingWindow();
	~CCRakNetSlidingWindow() noexcept;

	/* 将所有变量重置为新连接的初始状态 */
	void Init(CCTimeType curTime, uint32_t maxDatagramPayload);

	/* 按时间推进更新 */
	void Update(CCTimeType curTime, bool hasDataToSendOrResend);

	int GetRetransmissionBandwidth(CCTimeType curTime, CCTimeType timeSinceLastTick, uint32_t unacknowledgedBytes, bool isContinuousSend);
	int GetTransmissionBandwidth(CCTimeType curTime, CCTimeType timeSinceLastTick, uint32_t unacknowledgedBytes, bool isContinuousSend);

	/*
	 * ACK 无需立即发送，可以缓冲后批量发送
	 * 这样可以减少总体带宽占用
	 * 缓冲时长取决于发送方的重传时间
	 * 每次更新心跳时调用一次，必要时发送
	 */
	bool ShouldSendACKs(CCTimeType curTime, CCTimeType estimatedTimeToNextTick);

	/*
	 * 每个发送的数据包都必须包含序列号
	 * 调用此函数获取序列号，该序列号将传入 OnGotPacketPair()
	 */
	DatagramSequenceNumberType GetAndIncrementNextDatagramSequenceNumber();
	DatagramSequenceNumberType GetNextDatagramSequenceNumber();

	/*
	 * 发送数据包时调用
	 * 尽可能将第 15 和第 16 个数据包作为数据包对发送
	 * 当标记为数据包对的包到达时，传递给 OnGotPacketPair()
	 * 所有数据包到达时（额外地）传递给 OnGotPacket
	 * 数据包应包含本机系统时间，以便将 RTT 传递给 OnNonDuplicateAck()
	 */
	void OnSendBytes(CCTimeType curTime, uint32_t numBytes);

	/* 收到数据包对时调用 */
	void OnGotPacketPair(DatagramSequenceNumberType datagramSequenceNumber, uint32_t sizeInBytes, CCTimeType curTime);

	/*
	 * 收到数据包（包括数据包对）时调用
	 * 若数据报序列号乱序，skippedMessageCount 将非零
	 * 此时，对每个缺失的序列号发送 NAK
	 */
	bool OnGotPacket(DatagramSequenceNumberType datagramSequenceNumber, bool isContinuousSend, CCTimeType curTime, uint32_t sizeInBytes, uint32_t *skippedMessageCount);

	/*
	 * 收到 NAK 时调用，参数为丢失消息的序列号
	 * 影响拥塞控制
	 */
	void OnResend(CCTimeType curTime, RakNet::TimeUS nextActionTime);
	void OnNAK(CCTimeType curTime, DatagramSequenceNumberType nakSequenceNumber);

	/*
	 * 收到 ACK 时调用。
	 * hasBAndAS 可能随 ACK 一起写入，参见 OnSendAck()
	 * B 和 AS 用于 UpdateWindowSizeAndAckOnAckPerSyn 中的计算
	 * B 和 AS 每个 SYN 周期最多更新一次
	 */
	void OnAck(CCTimeType curTime, CCTimeType rtt, bool hasBAndAS, BytesPerMicrosecond _B, BytesPerMicrosecond _AS, double totalUserDataBytesAcked, bool isContinuousSend, DatagramSequenceNumberType sequenceNumber );
	void OnDuplicateAck( CCTimeType curTime, DatagramSequenceNumberType sequenceNumber );
	
	/*
	 * 发送 ACK 时调用，以判断 ACK 是否需要携带 B 和 AS 参数
	 * 请在调用 OnSendAck() 之前调用此函数
	 */
	void OnSendAckGetBAndAS(CCTimeType curTime, bool *hasBAndAS, BytesPerMicrosecond *_B, BytesPerMicrosecond *_AS);

	/*
	 * 发送 ACK 时调用，必要时写入 B 和 AS
	 * B 和 AS 每个 SYN 周期仅写入一次，以避免频繁计算
	 * 同时更新 SND（发送间隔），因为数据已写出
	 * 请确保在调用 OnSendAck() 之前先调用 OnSendAckGetBAndAS()，因为是否写入会影响 numBytes 的值
	 */
	void OnSendAck(CCTimeType curTime, uint32_t numBytes);

	/*
	 * 发送 NACK 时调用
	 * 同时更新 SND（发送间隔），因为数据已写出
	 */
	void OnSendNACK(CCTimeType curTime, uint32_t numBytes);
	
	/*
	 * 发送方的重传超时时间（RTO）
	 * 若消息最后一次传输时间与当前时间之差超过 RTO，则该包满足重传条件（仍需等待拥塞控制许可）
	 * RTO = (RTT + 4 * RTTVar) + SYN
	 * 若在过去 RTO 时间内持续发送但未收到任何 ACK 或 NAK，则 SND 翻倍
	 * 这是按消息计算的，与 UDT 不同；RakNet 支持持续数据流中的丢包重传，而 UDT 仅支持 RELIABLE_ORDERED 模式
	 * 最小值为 100 毫秒
	 */
	CCTimeType GetRTOForRetransmission(unsigned char timesSent) const;

	/*
	 * 设置单个数据报中可发送的最大数据量
	 * 默认值为 MAXIMUM_MTU_SIZE - UDP_HEADER_SIZE
	 */
	void SetMTU(uint32_t bytes);

	/* 返回由 SetMTU() 设置的值 */
	uint32_t GetMTU(void) const;

	/* 查询统计信息 */
	BytesPerMicrosecond GetLocalSendRate(void) const {return 0;}
	BytesPerMicrosecond GetLocalReceiveRate(CCTimeType currentTime) const;
	BytesPerMicrosecond GetRemoveReceiveRate(void) const {return 0;}
	/* BytesPerMicrosecond GetEstimatedBandwidth(void) const {return B;} */
	BytesPerMicrosecond GetEstimatedBandwidth(void) const {return GetLinkCapacityBytesPerSecond()*1000000.0;}
	double GetLinkCapacityBytesPerSecond(void) const {return 0;}

	/* 查询统计信息 */
	double GetRTT(void) const;

	bool GetIsInSlowStart(void) const {return IsInSlowStart();}
	uint32_t GetCWNDLimit(void) const {return static_cast<uint32_t>(0);}


	/* 考虑变量溢出情况下，a 是否大于 b？ */
	static bool GreaterThan(DatagramSequenceNumberType a, DatagramSequenceNumberType b);
	/* 考虑变量溢出情况下，a 是否小于 b？ */
	static bool LessThan(DatagramSequenceNumberType a, DatagramSequenceNumberType b);
/* void SetTimeBetweenSendsLimit(unsigned int bitsPerSecond); */
	uint64_t GetBytesPerSecondLimitByCongestionControl(void) const;
	  
	protected:

	/* 用户可发送的最大字节数，即一个完整数据报的大小 */
	uint32_t MAXIMUM_MTU_INCLUDING_UDP_HEADER;

	double cwnd; /* 网络中允许的最大字节数 */
	double ssThresh; /* 慢启动与拥塞避免之间的阈值 */

	/*
	 * 收到 ACK 时，若 oldestUnsentAck == 0，将其设置为当前时间
	 * 发出 ACK 后，将 oldestUnsentAck 重置为 0
	 */
	CCTimeType oldestUnsentAck;

	CCTimeType GetSenderRTOForACK(void) const;

	/* 每个发出的数据报都被分配一个序列号，每次分配后递增 1 */
	DatagramSequenceNumberType nextDatagramSequenceNumber;
	DatagramSequenceNumberType nextCongestionControlBlock;
	bool backoffThisBlock, speedUpThisBlock;
	/*
	 * 跟踪已到达的数据报序列号。
	 * 若序列号出现跳跃，则对所有缺失的序列号发送 NAK
	 */
	DatagramSequenceNumberType expectedNextSequenceNumber;

	bool _isContinuousSend;

	bool IsInSlowStart(void) const;

	double lastRtt, estimatedRTT, deviationRtt;

};

}

#endif
