/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/*  FileListTransferCBInterface.h */


#pragma once
#include "RakMemoryOverride.h"
#include "FileListNodeContext.h"

#ifdef _MSC_VER
#pragma warning( push )
#endif

namespace RakNet
{

/*
 * 被FileListTransfer plugin as a callback for when we get a file使用
 * You get the last file when fileIndex==numberOfFilesInThisSet
 * 另见 FileListTransfer
 */
class FileListTransferCBInterface
{
public:
	/* 注意：如果修改了此结构体，swig 文件中的对应结构体也需要同步修改 */
	struct OnFileStruct
	{
		/* The index into the set of files, from 0 to numberOfFilesInThisSet */
		unsigned fileIndex;

		/* The name of the file */
		char fileName[512];

		/* The data pointed to by the file */
		char *fileData;

		/* The amount of data to be downloaded for this file */
		BitSize_t byteLengthOfThisFile;

		/* How many bytes of this file has been downloaded */
		BitSize_t bytesDownloadedForThisFile;

		/*
		 * Files are transmitted in sets, where more than one set of files can be transmitted at the same time.
		 * This is the identifier for the set, which is returned by FileListTransfer::SetupReceive
		 */
		unsigned short setID;

		/* The number of files that are in this set. */
		unsigned numberOfFilesInThisSet;

		/* The total length of the transmitted files for this set, after being uncompressed */
		unsigned byteLengthOfThisSet;

		/* The total length, in bytes, downloaded for this set. */
		unsigned bytesDownloadedForThisSet;

		/*
		 * User data passed to one of the functions in the FileList class.
		 * However, on error, this is instead changed to one of the enumerations in the PatchContext structure.
		 */
		FileListNodeContext context;

		/* Who sent this file */
		SystemAddress senderSystemAddress;

		/* Who sent this file. Not valid when using TCP, only RakPeer (UDP) */
		RakNetGUID senderGuid;
	};

	/* 注意：如果修改了此结构体，swig 文件中的对应结构体也需要同步修改 */
	struct FileProgressStruct
	{
		/* 参数[输出] onFileStruct General information about this file, such as the filename and the first partLength bytes. You do NOT need to save this data yourself. The complete file will arrive normally. */
		OnFileStruct *onFileStruct;
		/* 参数[输出] partCount The zero based index into partTotal. The percentage complete done of this file is 100 * (partCount+1)/partTotal */
		unsigned int partCount;
		/* 参数[输出] partTotal The total number of parts this file was split into. Each part will be roughly the MTU size, minus the UDP header and RakNet headers */
		unsigned int partTotal;
		/* 参数[输出] dataChunkLength How many bytes long firstDataChunk and iriDataChunk are */
		unsigned int dataChunkLength;
		/* 参数[输出] firstDataChunk The first partLength of the final file. If you store identifying information about the file in the first partLength bytes, you can read them while the download is taking place. If this hasn't arrived yet, firstDataChunk will be 0 */
		char *firstDataChunk;
		/* 参数[输出] iriDataChunk If the remote system is sending this file using IncrementalReadInterface, then this is the chunk we just downloaded. It will not exist in memory after this callback. You should either store this to disk, or in memory. If it is 0, then the file is smaller than one chunk, and will be held in memory automatically */
		char *iriDataChunk;
		/* 参数[输出] iriWriteOffset Offset in bytes from the start of the file for the data pointed to by iriDataChunk */
		unsigned int iriWriteOffset;
		/* 参数[输出] Who sent this file */
		SystemAddress senderSystemAddress;
		/* 参数[输出] Who sent this file. Not valid when using TCP, only RakPeer (UDP) */
		RakNetGUID senderGuid;
		/* 参数[输入] allocateIrIDataChunkAutomatically If true, then RakNet will hold iriDataChunk for you and return it in OnFile. Defaults to true */
		bool allocateIrIDataChunkAutomatically;
	};

	struct DownloadCompleteStruct
	{
		/*
		 * Files are transmitted in sets, where more than one set of files can be transmitted at the same time.
		 * This is the identifier for the set, which is returned by FileListTransfer::SetupReceive
		 */
		unsigned short setID;

		/* The number of files that are in this set. */
		unsigned numberOfFilesInThisSet;

		/* The total length of the transmitted files for this set, after being uncompressed */
		unsigned byteLengthOfThisSet;

		/* Who sent this file */
		SystemAddress senderSystemAddress;

		/* Who sent this file. Not valid when using TCP, only RakPeer (UDP) */
		RakNetGUID senderGuid;
	};

	FileListTransferCBInterface() {}
	virtual ~FileListTransferCBInterface() {}

	/*
	 * Got a file.
	 * This structure is only valid for the duration of this function call.
	 * 返回值: Return true to have RakNet delete the memory allocated to hold this file for this function call.
	 */
	virtual bool OnFile(OnFileStruct *onFileStruct)=0;

	/*
	 * Got part of a big file internally in RakNet
	 * This is called in one of two circumstances: Either the transport layer is returning ID_PROGRESS_NOTIFICATION, or you got a block via IncrementalReadInterface
	 * If the transport layer is returning ID_PROGRESS_NOTIFICATION (see RakPeer::SetSplitMessageProgressInterval()) then FileProgressStruct::iriDataChunk will be 0.
	 * If this is a block via IncrementalReadInterface, then iriDataChunk will point to the block just downloaded.
	 * If not using IncrementalReadInterface, then you only care about partCount and partTotal to tell how far the download has progressed. YOu can use firstDataChunk to read the first part of the file if desired. The file is usable when you get the OnFile callback.
	 * If using IncrementalReadInterface and you let RakNet buffer the files in memory (default), then it is the same as above. The file is usable when you get the OnFile callback.
	 * If using IncrementalReadInterface and you do not let RakNet buffer the files in memory, then set allocateIrIDataChunkAutomatically to false. Write the file to disk whenever you get OnFileProgress and iriDataChunk is not 0, and ignore OnFile.
	 */
	virtual void OnFileProgress(FileProgressStruct *fps)=0;

	/*
	 * Called while the handler is active by FileListTransfer
	 * 当你完成该类的使用后返回 false。
	 * At that point OnDereference will be called and the class will no longer be maintained by the FileListTransfer plugin.
	 */
	virtual bool Update() {return true;}

	/*
	 * 当the download is completed时调用
	 * If you are finished with this class, return false.
	 * At that point OnDereference will be called and the class will no longer be maintained by the FileListTransfer plugin.
	 * Otherwise return true, and Update will continue to be called.
	 */
	virtual bool OnDownloadComplete(DownloadCompleteStruct *dcs) {(void) dcs; return false;}

	/*
	 * This function is called when this instance is about to be dereferenced by the FileListTransfer plugin.
	 * Update will no longer be called.
	 * 如果向 FileListTransfer::SetupReceive::deleteHandler 传递了 true，它将被自动删除
	 * Otherwise it is up to you to delete it yourself.
	 */
	virtual void OnDereference() {}
};

} /* RakNet 命名空间 */

#ifdef _MSC_VER
#pragma warning( pop )
#endif
