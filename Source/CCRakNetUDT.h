/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "RakNetDefines.h"

#if USE_SLIDING_WINDOW_CONGESTION_CONTROL!=1

#pragma once
#include "NativeTypes.h"
#include "RakNetTime.h"
#include "RakNetTypes.h"
#include "DS_Queue.h"

/* 若使用 iPod Touch TG，请将此值设为 4。*/
#define CC_TIME_TYPE_BYTES 8

namespace RakNet
{

#if CC_TIME_TYPE_BYTES==8
using CCTimeType = uint64_t;
#else
using CCTimeType = uint32_t;
#endif

using DatagramSequenceNumberType = uint24_t;
using BytesPerMicrosecond = double;
using BytesPerSecond = double;
using MicrosecondsPerByte = double;

/* CC_RAKNET_UDT_PACKET_HISTORY_LENGTH 应为 2 的幂次，以确保写入索引能正确回绕 */
static constexpr int CC_RAKNET_UDT_PACKET_HISTORY_LENGTH = 64;
static constexpr int RTT_HISTORY_LENGTH = 64;

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

/*
 * Encapsulates UDT congestion control, as used by RakNet
 * Requirements:
 * <OL>
 * <LI>Each datagram is no more than MAXIMUM_MTU_SIZE, after accounting for the UDP header
 * <LI>Each datagram containing a user message has a sequence number which is set after calling OnSendBytes(). Set it by calling GetAndIncrementNextDatagramSequenceNumber()
 * <LI>System is designed to be used from a single thread.
 * <LI>Each packet should have a timeout time based on GetSenderRTOForACK(). If this time elapses, add the packet to the head of the send list for retransmission.
 * </OL>
 *
 * Recommended:
 * <OL>
 * <LI>Call sendto in its own thread. This takes a significant amount of time in high speed networks.
 * </OL>
 *
 * Algorithm:
 * <OL>
 * <LI>On a new connection, call Init()
 * <LI>On a periodic interval (SYN time is the best) call Update(). Also call ShouldSendACKs(), and send buffered ACKS if it returns true.
 * <LI>Call OnSendAck() when sending acks.
 * <LI>When you want to send or resend data, call GetNumberOfBytesToSend(). It will return you enough bytes to keep you busy for estimatedTimeToNextTick. You can send more than this to fill out a datagram, or to send packet pairs
 * <LI>Call OnSendBytes() when sending datagrams.
 * <LI>When data arrives, record the sequence number and buffer an ACK for it, to be sent from Update() if ShouldSendACKs() returns true
 * <LI>Every 16 packets that you send, send two of them back to back (a packet pair) as long as both packets are the same size. If you don't have two packets the same size, it is fine to defer this until you do.
 * <LI>When you get a packet, call OnGotPacket(). If the packet is also either of a packet pair, call OnGotPacketPair()
 * <LI>If you get a packet, and the sequence number is not 1 + the last sequence number, send a NAK. On the remote system, call OnNAK() and resend that message.
 * <LI>If you get an ACK, remove that message from retransmission. Call OnNonDuplicateAck().
 * <LI>If a message is not ACKed for GetRTOForRetransmission(), resend it.
 * </OL>
 */
class CCRakNetUDT
{
	public:
	
	CCRakNetUDT();
	~CCRakNetUDT() noexcept;

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
	void OnDuplicateAck( CCTimeType curTime, DatagramSequenceNumberType sequenceNumber ) {}
	
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
	BytesPerMicrosecond GetLocalSendRate(void) const {return 1.0 / SND;}
	BytesPerMicrosecond GetLocalReceiveRate(CCTimeType currentTime) const;
	BytesPerMicrosecond GetRemoveReceiveRate(void) const {return AS;}
	/* BytesPerMicrosecond GetEstimatedBandwidth(void) const {return B;} */
	BytesPerMicrosecond GetEstimatedBandwidth(void) const {return GetLinkCapacityBytesPerSecond()*1000000.0;}
	double GetLinkCapacityBytesPerSecond(void) const {return estimatedLinkCapacityBytesPerSecond;};

	/* 查询统计信息 */
	double GetRTT(void) const;

	bool GetIsInSlowStart(void) const {return isInSlowStart;}
	uint32_t GetCWNDLimit(void) const {return static_cast<uint32_t>(CWND*MAXIMUM_MTU_INCLUDING_UDP_HEADER);}


	/* 考虑变量溢出情况下，a 是否大于 b？ */
	static bool GreaterThan(DatagramSequenceNumberType a, DatagramSequenceNumberType b);
	/* 考虑变量溢出情况下，a 是否小于 b？ */
	static bool LessThan(DatagramSequenceNumberType a, DatagramSequenceNumberType b);
/* void SetTimeBetweenSendsLimit(unsigned int bitsPerSecond); */
	uint64_t GetBytesPerSecondLimitByCongestionControl(void) const;

	protected:
	/* --------------------------- PROTECTED VARIABLES --------------------------- */
	/*
	 * 字节间发送时间间隔，单位为微秒。
	 * 仅在 slowStart == false 时使用
	 * 随着持续收到消息而逐渐增大
	 * 在 NAK 和超时时减小
	 * 初始值为 0（无效状态）
	 */
	MicrosecondsPerByte SND;
	
	/*
	 * 辅助窗口机制，控制网络中最大在途数据包数量
	 * 在慢启动期间及之后均使用，但主要在慢启动期间生效
	 * 初始值为 2，也是最低阈值
	 * 最大值为套接字接收缓冲区大小除以 MTU
	 * CWND = AS × (RTT + SYN) + 16
	 */
	double CWND;
	
	/*
	 * 每次 SYN 间隔更新时，nextSYNUpdate 设置为下次应更新的时刻
	 * 通常使用 nextSYNUpdate += SYN，以维持稳定的更新节奏
	 * 但若这样做会导致立即再次更新，则设置为当前时间 + SYN 微秒（适用于线程长时间未更新的情况）
	 */
	CCTimeType nextSYNUpdate;
		
		
	/*
	 * 下次写入 packetPairRecieptHistory 的索引位置
	 * 历史记录始终为满（以默认值初始化），因此无需读取索引
	 */
	int packetPairRecieptHistoryWriteIndex;

	/*
	 * 每当接收方收到背靠背数据包时，从 packetPairRecieptHistory 中取值发送给发送方
	 * 按 B = B × 0.875 + incomingB × 0.125 更新
	 */
	/* BytesPerMicrosecond B; */
	
	/*
	 * 运行时往返时间（ping × 2）
	 * 仅发送方需要此值
	 * 初始值为 UNSET（未设置）
	 * 首次计算时设置为 rtt
	 * 按 RTT = RTT × 0.875 + rtt × 0.125 逐步更新
	 */
	double RTT;
	
	/*
	 * 往返时间方差
	 * 仅发送方需要此值
	 * 初始值为 UNSET（未设置）
	 * 首次计算时设置为 rtt
	 */
	/* double RTTVar; */
	/* 更新：改用最小/最大值，RTTVar 跟踪当前方差过于紧密会导致丢包 */
	double minRTT, maxRTT;
		
	/*
	 * 用于计算数据包到达速率（UDT 中）或数据到达速率（RakNet 中，数据报大小不一）
	 * 过滤器用于剔除 bytesPerMicrosecond 最低的一半值，以消除抖动和非活跃期的干扰
	 * 文档中称为 AS（数据到达速率）
	 * AS 每 10 个 ACK 计算一次，并发送给发送方
	 * 每个节点表示 (curTime - lastPacketArrivalTime) / bytes
	 * 与 ReceiverCalculateDataArrivalRate() 配合使用
	 */
	BytesPerMicrosecond packetArrivalHistory[CC_RAKNET_UDT_PACKET_HISTORY_LENGTH];
	BytesPerMicrosecond packetArrivalHistoryContinuousGaps[CC_RAKNET_UDT_PACKET_HISTORY_LENGTH];
	unsigned char packetArrivalHistoryContinuousGapsIndex;
	uint64_t continuousBytesReceived;
	CCTimeType continuousBytesReceivedStartTime;
	unsigned int packetArrivalHistoryWriteCount;

	/*
	 * 下次写入 packetArrivalHistory 的索引位置
	 * 历史记录始终为满（以默认值初始化），因此无需读取索引
	 */
	int packetArrivalHistoryWriteIndex;

	/* 记录最后一个到达数据包的时间，供新包到达时计算 packetArrivalHistory 中的 BytesPerMicrosecond */
	CCTimeType lastPacketArrivalTime;

	/*
	 * 发送方到接收方的数据到达速率，由接收方告知
	 * 用于在慢启动结束时计算初始发送速率
	 */
	BytesPerMicrosecond AS;

	/*
	 * 接收方上次从 packetArrivalHistory 和 packetPairRecieptHistory 计算并发送 B 和 AS 的时刻
	 * 用于防止过于频繁地计算和发送（因为这些是耗时操作）
	 */
	CCTimeType lastTransmitOfBAndAS;
	
	/*
	 * 新连接始于慢启动阶段
	 * 慢启动期间不使用 SND，仅使用 CWND
	 * 收到 NAK 或 CWND 达到最大值时，慢启动结束
	 * 慢启动结束时，SND 初始化为接收方数据包到达速率的倒数
	 */
	bool isInSlowStart;
	
	/*
	 * 本次拥塞周期内收到的 NAK 数量
	 * 拥塞周期开始时初始化为 1
	 */
	uint32_t NAKCount;
	
	/*
	 * 拥塞周期内平均收到多少个 NAK？
	 * 初始值为 1
	 * 用于生成 1 到 AvgNAKNum 之间的随机数 DecRandom
	 */
	uint32_t AvgNAKNum;
	
	/* 本拥塞周期内 SND 已减少的次数，用于将减少次数限制在 5 次以内 */
	uint32_t DecCount;

	/* 每经过 DecInterval 个 NAK，在本拥塞周期内降低一次发送速率 */
	uint32_t DecInterval;

	/* 每个发出的数据报都被分配一个序列号，每次分配后递增 1 */
	DatagramSequenceNumberType nextDatagramSequenceNumber;

	/*
	 * 若数据包被标记为数据包对，则 lastPacketPairPacketArrivalTime 设置为其到达时间
	 * 当数据包对的第二个包到达时，可以用此值计算两包之间的时间间隔
	 */
	CCTimeType lastPacketPairPacketArrivalTime;

	/*
	 * 若数据包被标记为数据包对，则检查 lastPacketPairSequenceNumber 是否与刚到达的包的前一个序列号匹配
	 * 即检查上一个收到的包是否紧接在当前包之前
	 * 若是，则可利用 lastPacketPairPacketArrivalTime 计算两包之间的时间差，从而估算链路容量
	 * 初始化为 -1，避免数据包对的第一个包被误识别为第二个包
	 */
	DatagramSequenceNumberType lastPacketPairSequenceNumber;

	/*
	 * 用于限制 UpdateWindowSizeAndAckOnAckPerSyn() 每个 SYN 周期最多提速一次
	 * 这是为了防止提速速度超过拥塞控制的补偿能力
	 */
	CCTimeType lastUpdateWindowSizeAndAck;

	/*
	 * 每次因超时导致 SND 减半时，RTO 相应增大
	 * 这是为了防止向无响应系统发送大量重传包
	 * 收到任何数据时重置
	 */
	double ExpCount;

	/*
	 * 已发送的用户数据总字节数
	 * 用于在慢启动期间收到 ACK 时调整窗口大小
	 */
	uint64_t totalUserDataBytesSent;
	
	/*
	 * 收到 ACK 时，若 oldestUnsentAck == 0，将其设置为当前时间
	 * 发出 ACK 后，将 oldestUnsentAck 重置为 0
	 */
	CCTimeType oldestUnsentAck;
	
	/* 用户可发送的最大字节数，即一个完整数据报的大小 */
	uint32_t MAXIMUM_MTU_INCLUDING_UDP_HEADER;
	
	/* 最大窗口大小 */
	double CWND_MAX_THRESHOLD;
	
	/*
	 * 跟踪已到达的数据报序列号。
	 * 若序列号出现跳跃，则对所有缺失的序列号发送 NAK
	 */
	DatagramSequenceNumberType expectedNextSequenceNumber;

	/* B 和 AS 已发送的次数，用于强制至少发送 CC_RAKNET_UDT_PACKET_HISTORY_LENGTH 次 */
	/* 否则数组中的默认值会导致计算不准确 */
	uint32_t sendBAndASCount;

	/*
	 * 读入对应列表的最新值
	 * 在连接初始阶段使用，此时中位数滤波器尚不准确
	 */
	BytesPerMicrosecond mostRecentPacketArrivalHistory;

	bool hasWrittenToPacketPairReceiptHistory;

/* uint32_t rttHistory[RTT_HISTORY_LENGTH]; */
/* uint32_t rttHistoryIndex; */
/* uint32_t rttHistoryWriteCount; */
/* uint32_t rttSum, rttLow; */
/* CCTimeType lastSndUpdateTime; */
	double estimatedLinkCapacityBytesPerSecond;

	/* --------------------------- PROTECTED METHODS --------------------------- */
	/* 按 SYN 间隔更新 nextSYNUpdate；若长时间未更新，则设置为当前时间加上 SYN 间隔 */
	void SetNextSYNUpdate(CCTimeType currentTime);
	
	/* 根据发送方的数据包到达情况返回数据到达速率。*/
	BytesPerMicrosecond ReceiverCalculateDataArrivalRate(CCTimeType curTime) const;
	/* 返回数据到达速率的中位数 */
	BytesPerMicrosecond ReceiverCalculateDataArrivalRateMedian(void) const;

	/* 计算 BytesPerMicrosecond 数组的中位数 */
	static BytesPerMicrosecond CalculateListMedianRecursive(const BytesPerMicrosecond inputList[CC_RAKNET_UDT_PACKET_HISTORY_LENGTH], int inputListLength, int lessThanSum, int greaterThanSum);
/* static uint32_t CalculateListMedianRecursive(const uint32_t inputList[RTT_HISTORY_LENGTH], int inputListLength, int lessThanSum, int greaterThanSum); */
		
	/*
	 * 与 GetRTOForRetransmission 相同，但不考虑 ExpCount 因子
	 * 因为接收方不知道发送方的 ExpCount，即使知道也不应因此延迟 ACK
	 */
	CCTimeType GetSenderRTOForACK(void) const;

	/* 结束慢启动，进入正常传输速率阶段 */
	void EndSlowStart();

	/* 执行相应的单位换算 */
	inline double BytesPerMicrosecondToPacketsPerMillisecond(BytesPerMicrosecond in);

	/* 根据 ACK 或 ACK2 更新往返时间 */
	/* void UpdateRTT(CCTimeType rtt); */

	/* 更新慢启动之前对应的变量 */
	void UpdateWindowSizeAndAckOnAckPreSlowStart(double totalUserDataBytesAcked);

	/* 更新慢启动之后对应的变量 */
	void UpdateWindowSizeAndAckOnAckPerSyn(CCTimeType curTime, CCTimeType rtt, bool isContinuousSend, DatagramSequenceNumberType sequenceNumber);


	/* 将 halveSNDOnNoDataTime 设置到未来，同时重置 ExpCount（用于在完全无数据到达时倍增 RTO）*/
	void ResetOnDataArrivalHalveSNDOnNoDataTime(CCTimeType curTime);
	
	/* 初始化数组 */
	void InitPacketArrivalHistory();

	/* 调试输出 */
	void PrintLowBandwidthWarning();

	/* 已知问题：SND 偶尔会变得极大，曾出现 11693 的情况 */
	void CapMinSnd(const char *file, int line);

	void DecreaseTimeBetweenSends();
	void IncreaseTimeBetweenSends();

	int bytesCanSendThisTick;

	CCTimeType lastRttOnIncreaseSendRate;
	CCTimeType lastRtt;

	DatagramSequenceNumberType nextCongestionControlBlock;
	bool hadPacketlossThisBlock;
	DataStructures::Queue<CCTimeType> pingsLastInterval;
};

}

#endif
