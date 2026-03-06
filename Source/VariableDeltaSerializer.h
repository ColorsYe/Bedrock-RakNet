/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#pragma once
#include "VariableListDeltaTracker.h"
#include "DS_MemoryPool.h"
#include "NativeTypes.h"
#include "BitStream.h"
#include "PacketPriority.h"
#include "DS_OrderedList.h"

namespace RakNet
{

/*
 * Class to compare memory values of variables in a current state to a prior state
 * Results of the comparisons will be written to a bitStream, such that only changed variables get written<BR>
 * Can be used with ReplicaManager3 to Serialize a Replica3 per-variable, rather than comparing the entire object against itself<BR>
 * Usage:<BR>
 * <BR>
 * 1. Call BeginUnreliableAckedSerialize(), BeginUniqueSerialize(), or BeginIdenticalSerialize(). In the case of Replica3, this would be in the Serialize() call<BR>
 * 2. For each variable of the type in step 1, call Serialize(). The same variables must be serialized every tick()<BR>
 * 3. Call EndSerialize()<BR>
 * 4. Repeat step 1 for each of the other categories of how to send varaibles<BR>
 * <BR>
 * On the receiver:<BR>
 * <BR>
 * 1. Call BeginDeserialize(). In the case of Replica3, this would be in the Deserialize() call<BR>
 * 2. Call DeserializeVariable() for each variable, in the same order as was Serialized()<BR>
 * 3. Call EndSerialize()<BR>
 * 另见 The ReplicaManager3 sample
 */
class RAK_DLL_EXPORT VariableDeltaSerializer
{
protected:
	struct RemoteSystemVariableHistory;
	struct ChangedVariablesList;

public:
	VariableDeltaSerializer();
	~VariableDeltaSerializer() noexcept;

	struct SerializationContext
	{
		SerializationContext();
		~SerializationContext() noexcept;

		RakNetGUID guid;
		BitStream *bitStream;
		uint32_t rakPeerSendReceipt;
		RemoteSystemVariableHistory *variableHistory;
		RemoteSystemVariableHistory *variableHistoryIdentical;
		RemoteSystemVariableHistory *variableHistoryUnique;
		ChangedVariablesList *changedVariables;
		uint32_t sendReceipt;
		PacketReliability serializationMode;
		bool anyVariablesWritten;
		bool newSystemSend; /* Force send all, do not record */
	};

	struct DeserializationContext
	{
		BitStream *bitStream;
	};

	/*
	 * Call before doing one or more SerializeVariable calls when the data will be sent UNRELIABLE_WITH_ACK_RECEIPT
	 * The last value of each variable will be saved per remote system. Additionally, a history of _sendReceipts is stored to determine what to resend on packetloss.
	 * When variables are lost, they will be flagged dirty and always resent to the system that lost it
	 * Disadvantages: Every variable for every remote system is copied internally, in addition to a history list of what variables changed for which _sendReceipt. Very memory and CPU intensive for multiple connections.
	 * Advantages: When data needs to be resent by RakNet, RakNet can only resend the value it currently has. This allows the application to control the resend, sending the most recent value of the variable. The end result is that bandwidth is used more efficiently because old data is never sent.
	 * 前提条件: Upon getting ID_SND_RECEIPT_LOSS or ID_SND_RECEIPT_ACKED call OnMessageReceipt()
	 * 前提条件: AddRemoteSystemVariableHistory() and RemoveRemoteSystemVariableHistory() must be called for new and lost connections
	 * 参数[输入] context Holds the context of this group of serialize calls. This can be a stack object just passed to the function.
	 * 参数[输入] _guid Which system we are sending to
	 * 参数[输入] _bitSteam Which bitStream to write to
	 * 参数[输入] _sendReceipt Returned from RakPeer::IncrementNextSendReceipt() and passed to the Send() or SendLists() function. Identifies this update for ID_SND_RECEIPT_LOSS and ID_SND_RECEIPT_ACKED
	 */
	void BeginUnreliableAckedSerialize(SerializationContext *context, RakNetGUID _guid, BitStream *_bitStream, uint32_t _sendReceipt);

	/*
	 * Call before doing one or more SerializeVariable calls for data that may be sent differently to every remote system (such as an invisibility flag that only teammates can see)
	 * The last value of each variable will be saved per remote system.
	 * Unlike BeginUnreliableAckedSerialize(), send receipts are not necessary
	 * Disadvantages: Every variable for every remote system is copied internally. Very memory and CPU intensive for multiple connections.
	 * Advantages: When data is sent differently depending on the recipient, this system can make things easier to use and is as efficient as it can be.
	 * 前提条件: AddRemoteSystemVariableHistory() and RemoveRemoteSystemVariableHistory() must be called for new and lost connections
	 * 参数[输入] context Holds the context of this group of serialize calls. This can be a stack object just passed to the function.
	 * 参数[输入] _guid Which system we are sending to
	 * 参数[输入] _bitSteam Which bitStream to write to
	 */
	void BeginUniqueSerialize(SerializationContext *context, RakNetGUID _guid, BitStream *_bitStream);

	/*
	 * Call before doing one or more SerializeVariable calls for data that is sent with the same value to every remote system (such as health, position, etc.)
	 * This is the most common type of serialization, and also the most efficient
	 * Disadvantages: A copy of every variable still needs to be held, although only once
	 * Advantages: After the first serialization, the last serialized bitStream will be used for subsequent sends
	 * 前提条件: Call OnPreSerializeTick() before doing any calls to BeginIdenticalSerialize() for each of your objects, once per game tick
	 * 参数[输入] context Holds the context of this group of serialize calls. This can be a stack object just passed to the function.
	 * 参数[输入] _isFirstSerializeToThisSystem Pass true if this is the first time ever serializing to this system (the initial download). This way all variables will be written, rather than checking against prior sent values.
	 * 参数[输入] _bitSteam Which bitStream to write to
	 */
	void BeginIdenticalSerialize(SerializationContext *context, bool _isFirstSerializeToThisSystem, BitStream *_bitStream);

	/*
	 * Call after BeginUnreliableAckedSerialize(), BeginUniqueSerialize(), or BeginIdenticalSerialize(), then after calling SerializeVariable() one or more times
	 * 参数[输入] context Same context pointer passed to BeginUnreliableAckedSerialize(), BeginUniqueSerialize(), or BeginIdenticalSerialize()
	 */
	void EndSerialize(SerializationContext *context);

	/*
	 * Call when you receive the BitStream written by SerializeVariable(), before calling DeserializeVariable()
	 * 参数[输入] context Holds the context of this group of deserialize calls. This can be a stack object just passed to the function.
	 * 参数[输入] _bitStream Pass the bitStream originally passed to and written to by serialize calls
	 */
	void BeginDeserialize(DeserializationContext *context, BitStream *_bitStream);

	/* 参数[输入] context Same context pointer passed to BeginDeserialize() */
	void EndDeserialize(DeserializationContext *context);

	/*
	 * BeginUnreliableAckedSerialize() and BeginUniqueSerialize() require knowledge of when connections are added and dropped
	 * Call AddRemoteSystemVariableHistory() and RemoveRemoteSystemVariableHistory() to notify the system of these events
	 * 参数[输入] _guid Which system we are sending to
	 */
	void AddRemoteSystemVariableHistory(RakNetGUID guid);

	/*
	 * BeginUnreliableAckedSerialize() and BeginUniqueSerialize() require knowledge of when connections are added and dropped
	 * Call AddRemoteSystemVariableHistory() and RemoveRemoteSystemVariableHistory() to notify the system of these events
	 * 参数[输入] _guid Which system we are sending to
	 */
	void RemoveRemoteSystemVariableHistory(RakNetGUID guid);

	/*
	 * BeginIdenticalSerialize() requires knowledge of when serialization has started for an object across multiple systems
	 * This way it can setup the flag to do new comparisons against the last sent values, rather than just resending the last sent bitStream
	 * For Replica3, overload and call this from Replica3::OnUserReplicaPreSerializeTick()
	 */
	void OnPreSerializeTick();

	/*
	 * Call when getting ID_SND_RECEIPT_LOSS or ID_SND_RECEIPT_ACKED for a particular system
	 * Example:
	 *
	 * uint32_t msgNumber;
	 * memcpy(&msgNumber, packet->data+1, 4);
	 * DataStructures::List<Replica3*> replicaListOut;
	 * replicaManager.GetReplicasCreatedByMe(replicaListOut);
	 * unsigned int idx;
	 * for (idx=0; idx < replicaListOut.GetSize(); idx++)
	 * {
	 * 	((SampleReplica*)replicaListOut[idx])->NotifyReplicaOfMessageDeliveryStatus(packet->guid,msgNumber, packet->data[0]==ID_SND_RECEIPT_ACKED);
	 * }
	 *
	 * 参数[输入] guid Which system we are sending to
	 * 参数[输入] receiptId Encoded in bytes 1-4 inclusive of ID_SND_RECEIPT_LOSS and ID_SND_RECEIPT_ACKED
	 * 参数[输入] messageArrived True for ID_SND_RECEIPT_ACKED, false otherwise
	 */
	void OnMessageReceipt(RakNetGUID guid, uint32_t receiptId, bool messageArrived);

	/*
	 * Call to Serialize a variable
	 * Will write to the bitSteam passed to context true, variableValue if the variable has changed or has never been written. Otherwise will write false.
	 * 前提条件: You have called BeginUnreliableAckedSerialize(), BeginUniqueSerialize(), or BeginIdenticalSerialize()
	 * 前提条件: Will also require calling OnPreSerializeTick() if using BeginIdenticalSerialize()
	 * 注意: Be sure to call EndSerialize() after finishing all serializations
	 * 参数[输入] context Same context pointer passed to BeginUnreliableAckedSerialize(), BeginUniqueSerialize(), or BeginIdenticalSerialize()
	 * 参数[输入] variable A variable to write to the bitStream passed to context
	 */
	template <class VarType>
	void SerializeVariable(SerializationContext *context, const VarType &variable)
	{
		if (context->newSystemSend)
		{
			if (context->variableHistory->variableListDeltaTracker.IsPastEndOfList()==false)
			{
				/* previously sent data to another system */
				context->bitStream->Write(true);
				context->bitStream->Write(variable);
				context->anyVariablesWritten=true;
			}
			else
			{
				/* never sent data to another system */
				context->variableHistory->variableListDeltaTracker.WriteVarToBitstream(variable, context->bitStream);
				context->anyVariablesWritten=true;
			}
		}
		else if (context->serializationMode==UNRELIABLE_WITH_ACK_RECEIPT)
		{
			context->anyVariablesWritten|=
			context->variableHistory->variableListDeltaTracker.WriteVarToBitstream(variable, context->bitStream, context->changedVariables->bitField, context->changedVariables->bitWriteIndex++);
		}
		else
		{
			if (context->variableHistoryIdentical)
			{
				/* Identical serialization to a number of systems */
				if (didComparisonThisTick==false)
					context->anyVariablesWritten|=
					context->variableHistory->variableListDeltaTracker.WriteVarToBitstream(variable, context->bitStream);
				/* Else bitstream is written to at the end */
			}
			else
			{
				/* Per-system serialization */
				context->anyVariablesWritten|=
					context->variableHistory->variableListDeltaTracker.WriteVarToBitstream(variable, context->bitStream);
			}
		}
	}

	/*
	 * Call to deserialize into a variable
	 * 前提条件: You have called BeginDeserialize()
	 * 注意: Be sure to call EndDeserialize() after finishing all deserializations
	 * 参数[输入] context Same context pointer passed to BeginDeserialize()
	 * 参数[输入] variable A variable to write to the bitStream passed to context
	 */
	template <class VarType>
	bool DeserializeVariable(DeserializationContext *context, VarType &variable)
	{
		return VariableListDeltaTracker::ReadVarFromBitstream(variable, context->bitStream);
	}



protected:

	/* For a given send receipt from RakPeer::Send() track which variables we updated */
	/* That way if that send does not arrive (ID_SND_RECEIPT_LOSS) we can mark those variables as dirty to resend them with current values */
	struct ChangedVariablesList
	{
		uint32_t sendReceipt;
		unsigned short bitWriteIndex;
		unsigned char bitField[56];
	};

	/* static int Replica2ObjectComp( const uint32_t &key, ChangedVariablesList* const &data ); */

	static int UpdatedVariablesListPtrComp( const uint32_t &key, ChangedVariablesList* const &data );

	/* For each remote system, track the last values of variables we sent to them, and the history of what values changed per call to Send() */
	/* Every serialize if a variable changes from its last value, send it out again */
	/* Also if a send does not arrive (ID_SND_RECEIPT_LOSS) we use updatedVariablesHistory to mark those variables as dirty, to resend them unreliably with the current values */
	struct RemoteSystemVariableHistory
	{
		RakNetGUID guid;
		VariableListDeltaTracker variableListDeltaTracker;
		DataStructures::OrderedList<uint32_t,ChangedVariablesList*,VariableDeltaSerializer::UpdatedVariablesListPtrComp> updatedVariablesHistory;
	};
	/*
	 * A list of RemoteSystemVariableHistory indexed by guid, one per connection that we serialize to
	 * List is added to when SerializeConstruction is called, and removed from when SerializeDestruction is called, or when a given connection is dropped
	 */
	DataStructures::List<RemoteSystemVariableHistory*> remoteSystemVariableHistoryList;

	/* Because the ChangedVariablesList is created every serialize and destroyed every receipt I use a pool to avoid fragmentation */
	DataStructures::MemoryPool<ChangedVariablesList> updatedVariablesMemoryPool;

	bool didComparisonThisTick;
	RakNet::BitStream identicalSerializationBs;

	void FreeVarsAssociatedWithReceipt(RakNetGUID guid, uint32_t receiptId);
	void DirtyAndFreeVarsAssociatedWithReceipt(RakNetGUID guid, uint32_t receiptId);
	unsigned int GetVarsWrittenPerRemoteSystemListIndex(RakNetGUID guid);
	void RemoveRemoteSystemVariableHistory();

	RemoteSystemVariableHistory* GetRemoteSystemVariableHistory(RakNetGUID guid);

	ChangedVariablesList *AllocChangedVariablesList();
	void FreeChangedVariablesList(ChangedVariablesList *changedVariables);
	void StoreChangedVariablesList(RemoteSystemVariableHistory *variableHistory, ChangedVariablesList *changedVariables, uint32_t sendReceipt);

	RemoteSystemVariableHistory *StartVariableHistoryWrite(RakNetGUID guid);
	unsigned int GetRemoteSystemHistoryListIndex(RakNetGUID guid);

};

}
