/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/*  FileList.h */


#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_FileOperations==1

#pragma once
#include "Export.h"
#include "DS_List.h"
#include "RakMemoryOverride.h"
#include "RakNetTypes.h"
#include "FileListNodeContext.h"
#include "RakString.h"

#ifdef _MSC_VER
#pragma warning( push )
#endif

namespace RakNet
{
	class BitStream;
}

namespace RakNet
{
/* 前向声明 */
class RakPeerInterface;
class FileList;


/* 表示一个文件实例 */
struct FileListNode
{
	/* 文件名 */
	RakNet::RakString filename;

	/* 文件的完整路径，可能与文件名不同 */
	RakNet::RakString fullPathToFile;

	/* 文件数据（若尚未就绪则可能为 null） */
	char *data;

	/* 数据长度。若前面附加了文件哈希，则可能大于 fileLength */
	BitSize_t dataLengthBytes;

	/* 文件的长度 */
	unsigned fileLengthBytes;

	/* 用户自定义数据，用于描述此文件。 */
	FileListNodeContext context;

	/* 若为 true，data 和 dataLengthBytes 应为空。这只是存储文件名 */
	bool isAReference;
};

/* 通过 FileList::SetCallback() 设置的回调接口，用于在调用 FileList::AddFilesFromDirectory() 时获取进度通知 */
class RAK_DLL_EXPORT FileListProgress
{
public:
	/* 获取单例 GetInstance() 和销毁单例 DestroyInstance(instance*) */
	STATIC_FACTORY_DECLARATIONS(FileListProgress)

	FileListProgress() {}
	virtual ~FileListProgress() {}

	/* FileList::AddFilesFromDirectory() 开始时调用的第一个回调 */
	virtual void OnAddFilesFromDirectoryStarted(FileList *fileList, char *dir) {
		static_cast<void>(fileList);
		static_cast<void>(dir);
	}

	/* 对每个目录，当该目录开始处理时调用 */
	virtual void OnDirectory(FileList *fileList, char *dir, unsigned int directoriesRemaining) {
		static_cast<void>(fileList);
		static_cast<void>(dir);
		static_cast<void>(directoriesRemaining);
	}

	/* 对每个文件，当该文件开始处理时调用 */
	virtual void OnFile(FileList *fileList, char *dir, char *fileName, unsigned int fileSize) {
		static_cast<void>(fileList);
		static_cast<void>(dir);
		static_cast<void>(fileName);
		static_cast<void>(fileSize);
	}

	/*
	 * 当我们向远程系统发送文件时调用此函数。
	 * 参数[输入] fileName 正在发送的文件名
	 * 参数[输入] fileLengthBytes 文件的长度
	 * 参数[输入] offset 正在发送的文件字节偏移量
	 * 参数[输入] bytesBeingSent 本次推送发送的字节数
	 * 参数[输入] done 此文件是否已在本次推送中完成
	 * 参数[输入] targetSystem 发送目标
	 */
	virtual void OnFilePush(const char *fileName, unsigned int fileLengthBytes, unsigned int offset, unsigned int bytesBeingSent, bool done, SystemAddress targetSystem, unsigned short setId)
	{
		static_cast<void>(fileName);
		static_cast<void>(fileLengthBytes);
		static_cast<void>(offset);
		static_cast<void>(bytesBeingSent);
		static_cast<void>(done);
		static_cast<void>(targetSystem);
        static_cast<void>(setId);
	}

	/* 当所有文件均已读取并正在传输到远程系统时调用此函数 */
	virtual void OnFilePushesComplete( SystemAddress systemAddress, unsigned short setId )
	{
		static_cast<void>(systemAddress);
        static_cast<void>(setId);
	}

	/* 当向某系统的发送被中止（可能由于断连）时调用此函数 */
	virtual void OnSendAborted( SystemAddress systemAddress )
	{
		static_cast<void>(systemAddress);
	}
};

/* 使用 RAKNET_DEBUG_PRINTF 的 FileListProgress 实现 */
class RAK_DLL_EXPORT FLP_Printf : public FileListProgress
{
public:
	/* 获取单例 GetInstance() 和销毁单例 DestroyInstance(instance*) */
	STATIC_FACTORY_DECLARATIONS(FLP_Printf)

	FLP_Printf() {}
	virtual ~FLP_Printf() {}

	/* FileList::AddFilesFromDirectory() 开始时调用的第一个回调 */
	virtual void OnAddFilesFromDirectoryStarted(FileList *fileList, char *dir);

	/* 对每个目录，当该目录开始处理时调用 */
	virtual void OnDirectory(FileList *fileList, char *dir, unsigned int directoriesRemaining);

	/* 当所有文件均已传输到特定远程系统时调用此函数 */
    virtual void OnFilePushesComplete( SystemAddress systemAddress, unsigned short setID );

	/* 当向某系统的发送被中止（可能由于断连）时调用此函数 */
	virtual void OnSendAborted( SystemAddress systemAddress );
};

class RAK_DLL_EXPORT FileList
{
public:
	/* 获取单例 GetInstance() 和销毁单例 DestroyInstance(instance*) */
	STATIC_FACTORY_DECLARATIONS(FileList)

	FileList();
	~FileList() noexcept;
	/*
	 * 添加给定目录下的所有文件。
	 * 参数[输入] applicationDirectory 路径的第一部分。不作为文件名的一部分存储。使用 \ 作为路径分隔符。
	 * 参数[输入] subDirectory 文件路径的其余部分。作为文件名的前缀存储
	 * 参数[输入] writeHash 前 4 字节为文件的哈希值，其余为实际文件数据（当 writeData 为 true 时）
	 * 参数[输入] writeData 写入每个文件的内容
	 * 参数[输入] recursive 是否遍历子目录
	 * 参数[输入] context 用户自定义字节，与每个文件一起存储。可随意使用。
	 */
	void AddFilesFromDirectory(const char *applicationDirectory, const char *subDirectory, bool writeHash, bool writeData, bool recursive, FileListNodeContext context);

	/* 释放所有内存 */
	void Clear();

	/* 将所有编码数据写入位流 */
	void Serialize(RakNet::BitStream *outBitStream);

	/* 从位流读取所有编码数据。反序列化前先调用 Clear()。 */
	bool Deserialize(RakNet::BitStream *inBitStream);

	/*
	 * 给定现有文件集，在 applicationDirectory 中搜索相同的文件。
	 * 对于每个缺失或不同的文件，将该文件添加到 missingOrChangedFiles。注意：不写入文件内容，仅在 alwaysWriteHash 为 true 时写入哈希
	 * alwaysWriteHash 和 neverWriteHash 是优化项，用于避免在文件缺失或长度不同时不必要地读取文件内容来生成哈希。
	 * 参数[输入] applicationDirectory 路径的第一部分。不作为文件名的一部分存储。使用 \ 作为路径分隔符。
	 * 参数[输出] missingOrChangedFiles 输出列表
	 * 参数[输入] alwaysWriteHash 若为 true 且 neverWriteHash 为 false，将对磁盘上文件内容进行哈希，并将其作为长度为 SHA1_LENGTH 字节的文件数据写入。若为 false，且文件长度不同，则只写入文件名。
	 * 参数[输入] neverWriteHash 若为 true，即使可用也不写入哈希。若为 false，当文件长度相同且被强制比较时写入哈希。
	 */
	void ListMissingOrChangedFiles(const char *applicationDirectory, FileList *missingOrChangedFiles, bool alwaysWriteHash, bool neverWriteHash);

	/*
	 * 返回需要写入以使输入与当前 FileList 匹配的文件。
	 * 指定 dirSubset 以仅考虑以此路径开头的文件
	 * 指定 remoteSubdir 以假设输入中的所有文件名均以此路径开头，因此在比较文件名时将其去除。
	 * 参数[输入] input 完整文件列表
	 * 参数[输出] output 需要匹配输入的文件
	 * 参数[输入] dirSubset 若文件名不以此路径开头，则跳过该文件。
	 * 参数[输入] remoteSubdir 比较现有文件名时，从输入的文件名中删除此前缀。
	 */
	void GetDeltaToCurrent(FileList *input, FileList *output, const char *dirSubset, const char *remoteSubdir);

	/*
	 * 假设 FileList 包含一个没有数据的文件名列表，读取这些文件名的数据
	 * 参数[输入] applicationDirectory 在每个文件名前添加此路径。如有必要会自动添加尾部斜杠。使用 \ 作为路径分隔符。
	 * 参数[输入] writeFileData 为 true 时读取并存储文件数据。若 writeFileHash 为 true，则前 SHA1_LENGTH 字节包含哈希
	 * 参数[输入] writeFileHash 为 true 时读取并存储文件数据的哈希。若 writeFileHash 为 true，则前 SHA1_LENGTH 字节包含哈希
	 * 参数[输入] removeUnknownFiles 若文件在磁盘上不存在但在文件列表中，是否将其从文件列表中删除？
	 */
	void PopulateDataFromDisk(const char *applicationDirectory, bool writeFileData, bool writeFileHash, bool removeUnknownFiles);

	/*
	 * 默认情况下，GetDeltaToCurrent 将文件标记为非引用，即假设稍后填充
	 * 这将所有文件标记为引用，IncrementalReadInterface 增量处理它们时必须如此
	 */
	void FlagFilesAsReferences();

	/*
	 * 将所有文件写入磁盘，路径以 applicationDirectory 为前缀
	 * 参数[输入] applicationDirectory 路径前缀
	 */
	void WriteDataToDisk(const char *applicationDirectory);

	/*
	 * 添加一个文件，数据已在内存中。
	 * 参数[输入] filename 文件名，可选地以部分或完整路径为前缀。使用 \ 作为路径分隔符。
	 * 参数[输入] fullPathToFile 磁盘上文件的完整路径
	 * 参数[输入] data 要写入的内容
	 * 参数[输入] dataLength 数据的长度，若附加了额外数据（如哈希），可能大于 fileLength
	 * 参数[输入] fileLength 文件的长度
	 * 参数[输入] context 用户自定义字节，与每个文件一起存储。可随意使用。
	 * 参数[输入] isAReference 表示这只是对其他地方文件的引用——实际上没有任何数据
	 * 参数[输入] takeDataPointer 若为 true，则不分配 dataLength。只接管传递给 data 参数的指针
	 */
	void AddFile(const char *filename, const char *fullPathToFile, const char *data, const unsigned dataLength, const unsigned fileLength, FileListNodeContext context, bool isAReference=false, bool takeDataPointer=false);

	/*
	 * 添加一个文件，从磁盘读取。
	 * 参数[输入] filepath 文件的完整路径，包括文件名本身
	 * 参数[输入] filename 内部存储的文件名，可以是任意内容，但通常是完整路径或完整路径的一部分。
	 * 参数[输入] context 用户自定义字节，与每个文件一起存储。可随意使用。
	 */
	void AddFile(const char *filepath, const char *filename, FileListNodeContext context);

	/*
	 * 删除文件列表中存储的所有文件。
	 * 参数[输入] applicationDirectory 在每个文件名路径前添加此前缀。使用 \ 作为路径分隔符。
	 */
	void DeleteFiles(const char *applicationDirectory);

	/*
	 * 添加一个回调以获取文件列表实例操作的进度报告。
	 * 参数[输入] cb 指向外部定义的 FileListProgress 实例的指针。该指针在内部保存，因此应在此类有效期间保持有效。
	 */
	void AddCallback(FileListProgress *cb);

	/*
	 * 移除一个回调
	 * 参数[输入] cb 之前通过 AddCallback() 添加的 FileListProgress 实例指针
	 */
	void RemoveCallback(FileListProgress *cb);

	/* 移除所有回调 */
	void ClearCallbacks();

	/*
	 * 返回通过 AddCallback() 添加的所有回调
	 * 参数[输出] callbacks 列表被设置为回调列表
	 */
	void GetCallbacks(DataStructures::List<FileListProgress*> &callbacks);

	/* 可以读取，但不要修改 */
	DataStructures::List<FileListNode> fileList;

	static bool FixEndingSlash(char *str);
protected:
	DataStructures::List<FileListProgress*> fileListProgressCallbacks;
};

} /* RakNet 命名空间 */

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif
