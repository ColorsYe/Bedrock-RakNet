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
 *  FileListTransfer.h
 * 一个插件，提供简单的方式来压缩和增量发送 FileList 结构中的文件。
 *
 */


#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_FileListTransfer==1 && _RAKNET_SUPPORT_FileOperations==1

#pragma once
#include "RakNetTypes.h"
#include "Export.h"
#include "PluginInterface2.h"
#include "DS_Map.h"
#include "RakNetTypes.h"
#include "PacketPriority.h"
#include "RakMemoryOverride.h"
#include "FileList.h"
#include "DS_Queue.h"
#include "SimpleMutex.h"
#include "ThreadPool.h"

namespace RakNet
{
/* 前向声明 */
class IncrementalReadInterface;
class FileListTransferCBInterface;
class FileListProgress;
struct FileListReceiver;

/*
 * \defgroup FILE_LIST_TRANSFER_GROUP FileListTransfer
 * 一个插件，提供简单的方式来压缩和增量发送 FileList 结构中的文件。
 * * \ingroup PLUGINS_GROUP
 */

/*
 * 一个插件，提供简单的方式来压缩和增量发送 FileList 结构中的文件。
 * Similar to the DirectoryDeltaTransfer plugin, except that it doesn't send deltas based on pre-existing files or actually write the files to disk.
 *
 * Usage:
 * Call SetupReceive to allow one file set to arrive.  The value returned by FileListTransfer::SetupReceive()<BR>
 * is the setID that is allowed.<BR>
 * It's up to you to transmit this value to the other system, along with information indicating what kind of files you want to get.<BR>
 * The other system should then prepare a FileList and call FileListTransfer::Send(), passing the return value of FileListTransfer::SetupReceive()<BR>
 * as the setID parameter to FileListTransfer::Send()
 * \ingroup FILE_LIST_TRANSFER_GROUP
 */
class RAK_DLL_EXPORT FileListTransfer : public PluginInterface2
{
public:

	/* 获取单例 GetInstance() 和销毁单例 DestroyInstance(instance*) */
	STATIC_FACTORY_DECLARATIONS(FileListTransfer)

	FileListTransfer();
	virtual ~FileListTransfer();

	/*
	 * Optionally start worker threads when using _incrementalReadInterface for the Send() operation
	 * 参数[输入] numThreads how many worker threads to start
	 * 参数[输入] threadPriority Passed to the thread creation routine. Use THREAD_PRIORITY_NORMAL for Windows. For Linux based systems, you MUST pass something reasonable based on the thread priorities for your application.
	 */
	void StartIncrementalReadThreads(int numThreads, int threadPriority=-99999);
	
	/*
	 * Allows one corresponding Send() call from another system to arrive.
	 * 参数[输入] handler The class to call on each file
	 * 参数[输入] deleteHandler True to delete the handler when it is no longer needed.  False to not do so.
	 * 参数[输入] allowedSender Which system to allow files from.
	 * 返回值: A set ID value, which should be passed as the setID value to the Send() call on the other system.  This value will be returned in the callback and is unique per file set.  Returns 65535 on failure (not connected to sender)
	 */
    unsigned short SetupReceive(FileListTransferCBInterface *handler, bool deleteHandler, SystemAddress allowedSender);

	/*
	 * Send the FileList structure to another system, which must have previously called SetupReceive().
	 * 参数[输入] fileList A list of files.  The data contained in FileList::data will be sent incrementally and compressed among all files in the set
	 * 参数[输入] rakPeer The instance of RakNet to use to send the message. Pass 0 to use the instance the plugin is attached to
	 * 参数[输入] recipient The address of the system to send to
	 * 参数[输入] setID The return value of SetupReceive() which was previously called on recipient
	 * 参数[输入] priority Passed to RakPeerInterface::Send()
	 * 参数[输入] orderingChannel Passed to RakPeerInterface::Send()
	 * 参数[输入] _incrementalReadInterface If a file in fileList has no data, _incrementalReadInterface will be used to read the file in chunks of size chunkSize
	 * 参数[输入] _chunkSize How large of a block of a file to read/send at once. Large values use more memory but transfer slightly faster.
	 */
	void Send(FileList *fileList, RakNet::RakPeerInterface *rakPeer, SystemAddress recipient, unsigned short setID, PacketPriority priority, char orderingChannel, IncrementalReadInterface *_incrementalReadInterface=0, unsigned int _chunkSize=262144*4*16);

	/* 返回等待发送到特定地址的文件数量 */
	unsigned int GetPendingFilesToAddress(SystemAddress recipient);

	/* 停止 download */
	void CancelReceive(unsigned short setId);

	/* Remove all handlers associated with a particular system address. */
	void RemoveReceiver(SystemAddress systemAddress);

	/* Is a handler passed to SetupReceive still running? */
	bool IsHandlerActive(unsigned short setId);

	/*
	 * Adds a callback to get progress reports about what the file list instances do.
	 * 参数[输入] cb A pointer to an externally defined instance of FileListProgress. This pointer is held internally, so should remain valid as long as this class is valid.
	 */
	void AddCallback(FileListProgress *cb);

	/*
	 * 移除 callback
	 * 参数[输入] cb A pointer to an externally defined instance of FileListProgress that was previously added with AddCallback()
	 */
	void RemoveCallback(FileListProgress *cb);

	/* 移除 all callbacks */
	void ClearCallbacks();

	/*
	 * 返回通过 AddCallback() 添加的所有回调
	 * 参数[输出] callbacks The list is set to the list of callbacks
	 */
	void GetCallbacks(DataStructures::List<FileListProgress*> &callbacks);

	/* 内部使用 For plugin handling */
	PluginReceiveResult OnReceive(Packet *packet) override;
	/* 内部使用 For plugin handling */
	void OnRakPeerShutdown() override;
	/* 内部使用 For plugin handling */
	void OnClosedConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, PI2_LostConnectionReason lostConnectionReason ) override;
	/* 内部使用 For plugin handling */
	void Update() override;

protected:
	bool DecodeSetHeader(Packet *packet);
	bool DecodeFile(Packet *packet, bool fullFile);

	void Clear();

	void OnReferencePush(Packet *packet, bool fullFile);
	void OnReferencePushAck(Packet *packet);
	void SendIRIToAddress(SystemAddress systemAddress, unsigned short setId);

	DataStructures::Map<unsigned short, FileListReceiver*> fileListReceivers;
	unsigned short setId;
	DataStructures::List<FileListProgress*> fileListProgressCallbacks;

	struct FileToPush
	{
		FileListNode fileListNode;
		PacketPriority packetPriority;
		char orderingChannel;
		unsigned int currentOffset;
		/* /unsigned short setID; */
		unsigned int setIndex;
		IncrementalReadInterface *incrementalReadInterface;
		unsigned int chunkSize;
	};
	struct FileToPushRecipient
	{
		unsigned int refCount;
		SimpleMutex refCountMutex;
		void DeleteThis();
		void AddRef();
		void Deref();

		SystemAddress systemAddress;
		unsigned short setId;

		/* / SimpleMutex filesToPushMutex; */
		DataStructures::Queue<FileToPush*> filesToPush;
	};
	DataStructures::List< FileToPushRecipient* > fileToPushRecipientList;
	SimpleMutex fileToPushRecipientListMutex;
	void RemoveFromList(FileToPushRecipient *ftpr);

	struct ThreadData
	{
		FileListTransfer *fileListTransfer;
		SystemAddress systemAddress;
		unsigned short setId;
	};

	ThreadPool<ThreadData, int> threadPool;

	friend int SendIRIToAddressCB(FileListTransfer::ThreadData threadData, bool *returnOutput, void* perThreadData);
};

} /* RakNet 命名空间 */

#endif
