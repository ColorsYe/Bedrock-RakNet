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
 *  DirectoryDeltaTransfer.h
 * Simple class to send changes between directories.
 * In essence, a simple autopatcher that can be used for transmitting levels, skins, etc.
 *
 */


#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_DirectoryDeltaTransfer==1 && _RAKNET_SUPPORT_FileOperations==1

#pragma once
#include "RakMemoryOverride.h"
#include "RakNetTypes.h"
#include "Export.h"
#include "PluginInterface2.h"
#include "DS_Map.h"
#include "PacketPriority.h"

/*
 * \defgroup DIRECTORY_DELTA_TRANSFER_GROUP DirectoryDeltaTransfer
 * Simple class to send changes between directories
 * * \ingroup PLUGINS_GROUP
 */

/*
 * Simple class to send changes between directories.  In essence, a simple autopatcher that can be used for transmitting levels, skins, etc.
 * * 另见 AutopatcherClient class for database driven patching, including binary deltas and search by date.
 *
 * To use, first set the path to your application.  For example "C:/Games/MyRPG/"<BR>
 * To allow other systems to download files, call AddUploadsFromSubdirectory, where the parameter is a path relative<BR>
 * to the path to your application.  This includes subdirectories.<BR>
 * For example:<BR>
 * SetApplicationDirectory("C:/Games/MyRPG/");<BR>
 * AddUploadsFromSubdirectory("Mods/Skins/");<BR>
 * would allow downloads from<BR>
 * "C:/Games/MyRPG/Mods/Skins/*.*" as well as "C:/Games/MyRPG/Mods/Skins/Level1/*.*"<BR>
 * It would NOT allow downloads from C:/Games/MyRPG/Levels, nor would it allow downloads from C:/Windows<BR>
 * While pathToApplication can be anything you want, applicationSubdirectory must match either partially or fully between systems.
 * \ingroup DIRECTORY_DELTA_TRANSFER_GROUP
 */

namespace RakNet
{
/* 前向声明 */
class RakPeerInterface;
class FileList;
struct Packet;
struct InternalPacket;
struct DownloadRequest;
class FileListTransfer;
class FileListTransferCBInterface;
class FileListProgress;
class IncrementalReadInterface;

class RAK_DLL_EXPORT DirectoryDeltaTransfer : public PluginInterface2
{
public:
	/* 获取单例 GetInstance() 和销毁单例 DestroyInstance(instance*) */
	STATIC_FACTORY_DECLARATIONS(DirectoryDeltaTransfer)

	/* 构造函数 */
	DirectoryDeltaTransfer();

	/* 析构函数 */
	virtual ~DirectoryDeltaTransfer();

	/*
	 * This plugin has a dependency on the FileListTransfer plugin, which it uses to actually send the files.
	 * So you need an instance of that plugin registered with RakPeerInterface, and a pointer to that interface should be passed here.
	 * 参数[输入] flt A pointer to a registered instance of FileListTransfer
	 */
	void SetFileListTransferPlugin(FileListTransfer *flt);

	/*
	 * 将local root directory设置为base all file uploads and downloads off of
	 * 参数[输入] pathToApplication This path will be prepended to applicationSubdirectory in AddUploadsFromSubdirectory to find the actual path on disk.
	 */
	void SetApplicationDirectory(const char *pathToApplication);

	/*
	 * What parameters to use for the RakPeerInterface::Send() call when uploading files.
	 * 参数[输入] _priority 参见 RakPeerInterface::Send()
	 * 参数[输入] _orderingChannel 参见 RakPeerInterface::Send()
	 */
	void SetUploadSendParameters(PacketPriority _priority, char _orderingChannel);

	/*
	 * Add all files in the specified subdirectory recursively.
	 * subdir is appended to pathToApplication in SetApplicationDirectory().
	 * All files in the resultant directory and subdirectories are then hashed so that users can download them.
	 * 前提条件: You must call SetFileListTransferPlugin with a valid FileListTransfer plugin
	 * 参数[输入] subdir Concatenated with pathToApplication to form the final path from which to allow uploads.
	 */
	void AddUploadsFromSubdirectory(const char *subdir);

	/*
	 * Downloads files from the matching parameter subdir in AddUploadsFromSubdirectory.
	 * subdir must contain all starting characters in subdir in AddUploadsFromSubdirectory
	 * Therefore,
	 * AddUploadsFromSubdirectory("Levels/Level1/"); would allow you to download using DownloadFromSubdirectory("Levels/Level1/Textures/"...
	 * but it would NOT allow you to download from DownloadFromSubdirectory("Levels/"... or DownloadFromSubdirectory("Levels/Level2/"...
	 * 前提条件: You must call SetFileListTransferPlugin with a valid FileListTransfer plugin
	 * 注意: Blocking. Will block while hashes of the local files are generated
	 * 参数[输入] subdir A directory passed to AddUploadsFromSubdirectory on the remote system.  The passed dir can be more specific than the remote dir.
	 * 参数[输入] outputSubdir The directory to write the output to.  Usually this will match subdir but it can be different if you want.
	 * 参数[输入] prependAppDirToOutputSubdir True to prepend outputSubdir with pathToApplication when determining the final output path.  Usually you want this to be true.
	 * 参数[输入] host The address of the remote system to send the message to.
	 * 参数[输入] onFileCallback Callback to call per-file (可选).  When fileIndex+1==setCount in the callback then the download is done
	 * 参数[输入] _priority 参见 RakPeerInterface::Send()
	 * 参数[输入] _orderingChannel 参见 RakPeerInterface::Send()
	 * 参数[输入] cb Callback to get progress updates. Pass 0 to not use.
	 * 返回值: A set ID, identifying this download set.  Returns 65535 on host unreachable.
	 */
	unsigned short DownloadFromSubdirectory(const char *subdir, const char *outputSubdir, bool prependAppDirToOutputSubdir, SystemAddress host, FileListTransferCBInterface *onFileCallback, PacketPriority _priority, char _orderingChannel, FileListProgress *cb);

	/*
	 * Downloads files from the matching parameter subdir in AddUploadsFromSubdirectory.
	 * subdir must contain all starting characters in subdir in AddUploadsFromSubdirectory
	 * Therefore,
	 * AddUploadsFromSubdirectory("Levels/Level1/"); would allow you to download using DownloadFromSubdirectory("Levels/Level1/Textures/"...
	 * but it would NOT allow you to download from DownloadFromSubdirectory("Levels/"... or DownloadFromSubdirectory("Levels/Level2/"...
	 * 前提条件: You must call SetFileListTransferPlugin with a valid FileListTransfer plugin
	 * 注意: Nonblocking, but requires call to GenerateHashes()
	 * 参数[输入] localFiles Hashes of local files already on the harddrive. Populate with GenerateHashes(), which you may wish to call from a thread
	 * 参数[输入] subdir A directory passed to AddUploadsFromSubdirectory on the remote system.  The passed dir can be more specific than the remote dir.
	 * 参数[输入] outputSubdir The directory to write the output to.  Usually this will match subdir but it can be different if you want.
	 * 参数[输入] prependAppDirToOutputSubdir True to prepend outputSubdir with pathToApplication when determining the final output path.  Usually you want this to be true.
	 * 参数[输入] host The address of the remote system to send the message to.
	 * 参数[输入] onFileCallback Callback to call per-file (可选).  When fileIndex+1==setCount in the callback then the download is done
	 * 参数[输入] _priority 参见 RakPeerInterface::Send()
	 * 参数[输入] _orderingChannel 参见 RakPeerInterface::Send()
	 * 参数[输入] cb Callback to get progress updates. Pass 0 to not use.
	 * 返回值: A set ID, identifying this download set.  Returns 65535 on host unreachable.
	 */
	unsigned short DownloadFromSubdirectory(FileList &localFiles, const char *subdir, const char *outputSubdir, bool prependAppDirToOutputSubdir, SystemAddress host, FileListTransferCBInterface *onFileCallback, PacketPriority _priority, char _orderingChannel, FileListProgress *cb);

	/*
	 * Hash files already on the harddrive, in preparation for a call to DownloadFromSubdirectory(). Passed to second version of DownloadFromSubdirectory()
	 * This is slow, and it is exposed so you can call it from a thread before calling DownloadFromSubdirectory()
	 * 参数[输出] localFiles List of hashed files populated from outputSubdir and prependAppDirToOutputSubdir
	 * 参数[输入] outputSubdir The directory to write the output to.  Usually this will match subdir but it can be different if you want.
	 * 参数[输入] prependAppDirToOutputSubdir True to prepend outputSubdir with pathToApplication when determining the final output path.  Usually you want this to be true.
	 */
	void GenerateHashes(FileList &localFiles, const char *outputSubdir, bool prependAppDirToOutputSubdir);

	/* Clear all allowed uploads previously set with AddUploadsFromSubdirectory */
	void ClearUploads();

	/*
	 * 返回可供上传的文件数量
	 * 返回值: How many files are available for upload
	 */
	unsigned GetNumberOfFilesForUpload(void) const;

	/*
	 * Normally, if a remote system requests files, those files are all loaded into memory and sent immediately.
	 * This function allows the files to be read in incremental chunks, saving memory
	 * 参数[输入] _incrementalReadInterface If a file in fileList has no data, filePullInterface will be used to read the file in chunks of size chunkSize
	 * 参数[输入] _chunkSize How large of a block of a file to send at once
	 */
	void SetDownloadRequestIncrementalReadInterface(IncrementalReadInterface *_incrementalReadInterface, unsigned int _chunkSize);
	
	/* 内部使用 For plugin handling */
	PluginReceiveResult OnReceive(Packet *packet) override;
protected:
	void OnDownloadRequest(Packet *packet);

	char applicationDirectory[512];
	FileListTransfer *fileListTransfer;
	FileList *availableUploads;
	PacketPriority priority;
	char orderingChannel;
	IncrementalReadInterface *incrementalReadInterface;
	unsigned int chunkSize;
};

} /* RakNet 命名空间 */

#endif
