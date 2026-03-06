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
 * Ready event plugin.  This enables a set of systems to create a signal event, set this signal as ready or unready, and to trigger the event when all systems are ready
 *
 */


#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_ReadyEvent==1

#pragma once
#include "PluginInterface2.h"
#include "DS_OrderedList.h"

namespace RakNet {

class RakPeerInterface;

/*
 * \defgroup READY_EVENT_GROUP ReadyEvent
 * Peer to peer synchronized ready and unready events
 * * \ingroup PLUGINS_GROUP
 */

/*
 * \ingroup READY_EVENT_GROUP
 * 返回 status of a remote system when querying with ReadyEvent::GetReadyStatus
 */
enum class ReadyEventSystemStatus
{
	/*
	 * ----------- Normal states ---------------
	 * The remote system is not in the wait list, and we have never gotten a ready or complete message from it.
	 * 这是the default state for valid events
	 */
	RES_NOT_WAITING,
	/* We are waiting for this remote system to call SetEvent(thisEvent,true). */
	RES_WAITING,
	/* The remote system called SetEvent(thisEvent,true), but it still waiting for other systems before completing the ReadyEvent. */
	RES_READY,
	/*
	 * The remote system called SetEvent(thisEvent,true), and is no longer waiting for any other systems.
	 * This remote system has completed the ReadyEvent
	 */
	RES_ALL_READY,

		/* Error code, we couldn't look up the system because the event was unknown */
	RES_UNKNOWN_EVENT,
};

/*
 * Peer to peer synchronized ready and unready events
 * For peer to peer networks in a fully connected mesh.<BR>
 * Solves the problem of how to tell if all peers, relative to all other peers, are in a certain ready state.<BR>
 * For example, if A is connected to B and C, A may see that B and C are ready, but does not know if B is ready to C, or vice-versa.<BR>
 * This plugin uses two stages to solve that problem, first, everyone I know about is ready. Second, everyone I know about is ready to everyone they know about.<BR>
 * The user will get ID_READY_EVENT_SET and ID_READY_EVENT_UNSET as the signal flag is set or unset<BR>
 * The user will get ID_READY_EVENT_ALL_SET when all systems are done waiting for all other systems, in which case the event is considered complete, and no longer tracked.<BR>
 * 另见 FullyConnectedMesh2
 * \ingroup READY_EVENT_GROUP
 */
class ReadyEvent : public PluginInterface2
{
public:
	/* 获取单例 GetInstance() 和销毁单例 DestroyInstance(instance*) */
	STATIC_FACTORY_DECLARATIONS(ReadyEvent)

	/* 构造函数 */
	ReadyEvent();

	/* 析构函数 */
	virtual ~ReadyEvent();

	/* -------------------------------------------------------------------------------------------- */
	/* User functions */
	/* -------------------------------------------------------------------------------------------- */
	/*
	 * 设置 or updates the initial ready state for our local system
	 * If eventId is an unknown event the event is created.
	 * If eventId was previously used and you want to reuse it, call DeleteEvent first, or else you will keep the same event signals from before
	 * Systems previously or later added through AddToWaitList() with the same eventId when isReady=true will get ID_READY_EVENT_SET
	 * Systems previously added through AddToWaitList with the same eventId will get ID_READY_EVENT_UNSET
	 * For both ID_READY_EVENT_SET and ID_READY_EVENT_UNSET, eventId is encoded in bytes 1 through 1+sizeof(int)
	 * 参数[输入] eventId A user-defined identifier to wait on. This can be a sequence counter, an event identifier, or anything else you want.
	 * 参数[输入] isReady True to signal we are ready to proceed with this event, false to unsignal
	 * 返回值: False if event status is ID_READY_EVENT_FORCE_ALL_SET, or if we are setting to a status we are already in (no change). Otherwise true
	 */
	bool SetEvent(int eventId, bool isReady);

	/*
	 * When systems can call SetEvent() with isReady==false, it is possible for one system to return true from IsEventCompleted() while the other systems return false
	 * This can occur if a system SetEvent() with isReady==false while the completion message is still being transmitted.
	 * If your game has the situation where some action should be taken on all systems when IsEventCompleted() is true for any system, then call ForceCompletion() when the action begins.
	 * This will force all systems to return true from IsEventCompleted().
	 * 参数[输入] eventId A user-defined identifier to immediately set as completed
	 */
	void ForceCompletion(int eventId);

	/*
	 * Deletes an event.  We will no longer wait for this event, and any systems that we know have set the event will be forgotten.
	 * Call this to clear memory when events are completed and you know you will never need them again.
	 * 参数[输入] eventId A user-defined identifier
	 * 返回值: True on success. False (failure) on unknown eventId
	 */
	bool DeleteEvent(int eventId);

	/*
	 * Returns what was passed to SetEvent()
	 * 返回值: The value of isReady passed to SetEvent(). Also returns false on unknown event.
	 */
	bool IsEventSet(int eventId);

	/*
	 * Returns if the event is about to be ready and we are negotiating the final packets.
	 * This will usually only be true for a very short time, after which IsEventCompleted should return true.
	 * While this is true you cannot add to the wait list, or SetEvent() isReady to false anymore.
	 * 参数[输入] eventId A user-defined identifier
	 * 返回值: True if any other system has completed processing. Will always be true if IsEventCompleted() is true
	 */
	bool IsEventCompletionProcessing(int eventId) const;

	/*
	 * Returns if the wait list is a subset of the completion list.
	 * Call this after all systems you want to wait for have been added with AddToWaitList
	 * If you are waiting for a specific number of systems (such as players later connecting), also check GetRemoteWaitListSize(eventId) to be equal to 1 less than the total number of participants.
	 * 参数[输入] eventId A user-defined identifier
	 * 返回值: True on completion. False (failure) on unknown eventId, or the set is not completed.
	 */
	bool IsEventCompleted(int eventId) const;

	/*
	 * 返回 if this is a known event
	 * Events may be known even if we never ourselves referenced them with SetEvent, because other systems created them via ID_READY_EVENT_SET.
	 * 参数[输入] eventId A user-defined identifier
	 * 返回值: true if we have this event, false otherwise
	 */
	bool HasEvent(int eventId);

	/*
	 * 返回 total number of events stored in the system
	 * 返回值: The total number of events stored in the system.
	 */
	unsigned GetEventListSize(void) const;

	/*
	 * 返回 event ID stored at a particular index.  EventIDs are stored sorted from least to greatest
	 * 参数[输入] index Index into the array, from 0 to GetEventListSize()
	 * 返回值: The event ID stored at a particular index
	 */
	int GetEventAtIndex(unsigned index) const;

	/*
	 * Adds a system to wait for to signal an event before considering the event complete and returning ID_READY_EVENT_ALL_SET.
	 * As we add systems, if this event was previously set to true with SetEvent, these systems will get ID_READY_EVENT_SET.
	 * As these systems disconnect (directly or indirectly through the router) they are removed.
	 * 注意: If the event completion process has already started, you cannot add more systems, as this would cause the completion process to fail
	 * 参数[输入] eventId A user-defined number previously passed to SetEvent that has not yet completed
	 * 参数[输入] guid An address to wait for event replies from.  Pass UNASSIGNED_SYSTEM_ADDRESS for all currently connected systems. Until all systems in this list have called SetEvent with this ID and true, and have this system in the list, we won't get ID_READY_EVENT_COMPLETE
	 * 返回值: True on success, false on unknown eventId (this should be considered an error)
	 */
	bool AddToWaitList(int eventId, RakNetGUID guid);
	
	/*
	 * Removes systems from the wait list, which should have been previously added with AddToWaitList
	 * 注意: Systems that directly or indirectly disconnect from us are automatically removed from the wait list
	 * 参数[输入] guid The system to remove from the wait list. Pass UNASSIGNED_RAKNET_GUID for all currently connected systems.
	 * 返回值: True on success, false on unknown eventId (this should be considered an error)
	 */
	bool RemoveFromWaitList(int eventId, RakNetGUID guid);

	/*
	 * Returns if a particular system is waiting on a particular event.
	 * 参数[输入] eventId A user-defined identifier
	 * 参数[输入] guid The system we are checking up on
	 * 返回值: True if this system is waiting on this event, false otherwise.
	 */
	bool IsInWaitList(int eventId, RakNetGUID guid);
	
	/*
	 * 返回 total number of systems we are waiting on for this event
	 * Does not include yourself
	 * 参数[输入] eventId A user-defined identifier
	 * 返回值: The total number of systems we are waiting on for this event.
	 */
	unsigned GetRemoteWaitListSize(int eventId) const;

	/*
	 * 返回 system address of a system at a particular index, for this event
	 * 参数[输入] eventId A user-defined identifier
	 * 参数[输入] index Index into the array, from 0 to GetWaitListSize()
	 * 返回值: The system address of a system at a particular index, for this event.
	 */
	RakNetGUID GetFromWaitListAtIndex(int eventId, unsigned index) const;
		
	/*
	 * For a remote system, find out what their ready status is (waiting, signaled, complete).
	 * 参数[输入] eventId A user-defined identifier
	 * 参数[输入] guid Which system we are checking up on
	 * 返回值: The status of this system, for this particular event. 另见 ReadyEventSystemStatus
	 */
	ReadyEventSystemStatus GetReadyStatus(int eventId, RakNetGUID guid);

	/*
	 * This channel will be used for all RakPeer::Send calls
	 * 参数[输入] newChannel The channel to use for 内部使用 RakPeer::Send calls from this system.  Defaults to 0.
	 */
	void SetSendChannel(unsigned char newChannel);

	/* ---------------------------- ALL 内部使用 AFTER HERE ---------------------------- */
	/*
	 * 内部使用
	 * Status of a remote system
	 */
	struct RemoteSystem
	{
		MessageID lastSentStatus, lastReceivedStatus;
		RakNetGUID rakNetGuid;
	};
	static int RemoteSystemCompByGuid( const RakNetGUID &key, const RemoteSystem &data );
	/*
	 * 内部使用
	 * An event, with a set of systems we are waiting for, a set of systems that are signaled, and a set of systems with completed events
	 */
	struct ReadyEventNode
	{
		int eventId; /* Sorted on this */
		MessageID eventStatus;
		DataStructures::OrderedList<RakNetGUID,RemoteSystem,ReadyEvent::RemoteSystemCompByGuid> systemList;
	};
	static int ReadyEventNodeComp( const int &key, ReadyEvent::ReadyEventNode * const &data );


protected:
	/* -------------------------------------------------------------------------------------------- */
	/* 数据包处理函数 */
	/* -------------------------------------------------------------------------------------------- */
	PluginReceiveResult OnReceive(Packet *packet) override;
	void OnClosedConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, PI2_LostConnectionReason lostConnectionReason ) override;
	void OnRakPeerShutdown() override;
	
	void Clear();
	/*
	bool AnyWaitersCompleted(unsigned eventIndex) const;
	bool AllWaitersCompleted(unsigned eventIndex) const;
	bool AllWaitersReady(unsigned eventIndex) const;
	void SendAllReady(unsigned eventId, RakNetGUID guid);
	void BroadcastAllReady(unsigned eventIndex);
	void SendReadyStateQuery(unsigned eventId, RakNetGUID guid);
	void BroadcastReadyUpdate(unsigned eventIndex);
	bool AddToWaitListInternal(unsigned eventIndex, RakNetGUID guid);
	bool IsLocked(unsigned eventIndex) const;
	bool IsAllReadyByIndex(unsigned eventIndex) const;
	*/

	void SendReadyStateQuery(unsigned eventId, RakNetGUID guid);
	void SendReadyUpdate(unsigned eventIndex, unsigned systemIndex, bool forceIfNotDefault);
	void BroadcastReadyUpdate(unsigned eventIndex, bool forceIfNotDefault);
	void RemoveFromAllLists(RakNetGUID guid);
	void OnReadyEventQuery(Packet *packet);
	void PushCompletionPacket(unsigned eventId);
	bool AddToWaitListInternal(unsigned eventIndex, RakNetGUID guid);
	void OnReadyEventForceAllSet(Packet *packet);
	void OnReadyEventPacketUpdate(Packet *packet);
	void UpdateReadyStatus(unsigned eventIndex);
	bool IsEventCompletedByIndex(unsigned eventIndex) const;
	unsigned CreateNewEvent(int eventId, bool isReady);
	bool SetEventByIndex(int eventIndex, bool isReady);

	DataStructures::OrderedList<int, ReadyEventNode*, ReadyEvent::ReadyEventNodeComp> readyEventNodeList;
	unsigned char channel;
};

} /* RakNet 命名空间 */

#endif
