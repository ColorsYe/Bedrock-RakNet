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
 * 文件: AutopatcherRepositoryInterface.h
 * 说明: AutopatcherServer 用于获取自动补丁程序运行所需数据的接口。
 */


#pragma once
#include "IncrementalReadInterface.h"
#include "SimpleMutex.h"

namespace RakNet
{
/* 前向声明 */
class FileList;
class BitStream;

/* AutopatcherServer 用于获取自动补丁程序运行所需数据的接口。需由用户为自定义存储库方案实现。 */
class AutopatcherRepositoryInterface : public IncrementalReadInterface
{
public:
	/*
	 * 获取自指定日期以来已添加和已删除的文件列表。由 AutopatcherServer 调用，通常不需要直接调用。
	 * 参数[in]  applicationName                  标识应用程序的以空字符结尾的字符串
	 * 参数[out] addedOrModifiedFilesWithHashData  sinceDate 之后创建的文件的当前版本列表，数据字段为哈希值
	 * 参数[out] deletedFiles                      sinceDate 之后已删除的文件的当前版本名称列表
	 * 参数[in]  sinceDate                         输入日期，格式取决于所使用的存储库
	 * 参数[out] currentDate                       服务器当前日期，格式取决于所使用的存储库
	 * 返回值: 成功返回 true，失败返回 false。
	 */
	virtual bool GetChangelistSinceDate(const char *applicationName, FileList *addedOrModifiedFilesWithHashData, FileList *deletedFiles, double sinceDate)=0;

	/*
	 * 获取输入列表中每个文件的补丁（或文件本体），假定输入列表中每个文件均带有哈希值。
	 * 参数[in]  applicationName                          标识应用程序的以空字符结尾的字符串
	 * 参数[in]  input                                    包含 SHA1_LENGTH 字节哈希值的文件列表，用于从数据库中获取对应数据
	 * 参数[out] patchList                                返回包含文件数据或补丁的文件列表，是 input 的子集。
	 *                                                   每个文件的上下文数据为 PatchContext::PC_WRITE_FILE（直接写入文件）
	 *                                                   或 PC_HASH_WITH_PATCH（应用补丁）。若为 PC_HASH_WITH_PATCH，
	 *                                                   则文件内容为 SHA1_LENGTH 字节的补丁后跟哈希值，数据长度为补丁长度 + SHA1_LENGTH。
	 * 参数[out] currentDate                              服务器当前日期，格式取决于所使用的存储库
	 * 返回值: 成功返回 1，数据库失败返回 0，尝试下载原始未修改文件返回 -1。
	 */
	virtual int GetPatches(const char *applicationName, FileList *input, bool allowDownloadOfOriginalUnmodifiedFiles, FileList *patchList)=0;

	/*
	 * 返回最近一次更新中被修补、新增或删除的文件。对于被修补的文件，同时在 patchedFiles 中返回补丁，在 updatedFiles 中返回当前版本。
	 * 参数[in,out] applicationName            要获取补丁的应用程序名称。若为空，则使用最近更新的应用程序，并将名称更新到此字符串中。
	 * 参数[out]    patchedFiles               操作类型为 PatchContext::PC_HASH_2_WITH_PATCH 的补丁文件列表，
	 *                                         包含 priorHash（先前哈希）和 currentHash（当前哈希）。
	 *                                         currentHash 用于客户端在打补丁后验证补丁是否成功；
	 *                                         priorHash 用于 AutopatcherServer::OnGetPatch() 检查客户端是否能够对其当前版本进行哈希匹配。
	 * 参数[out]    updatedFiles               新增文件列表，包含文件名和实际数据。
	 * 参数[out]    addedOrModifiedFileHashes  被修改或新增文件的哈希列表，用于回复 ID_AUTOPATCHER_CREATION_LIST，
	 *                                         告知客户端自指定日期以来服务器上哪些文件已发生变更。
	 * 参数[out]    deletedFiles               最近一次补丁中被删除文件的当前版本名称列表。
	 * 参数[out]    whenPatched                补丁发布时间（自纪元起的秒数），可使用 C 语言的 time() 函数获取。
	 * 返回值: 成功返回 true，失败返回 false。
	 */
	virtual bool GetMostRecentChangelistWithPatches(
		RakNet::RakString &applicationName,
		FileList *patchedFiles,
		FileList *updatedFiles,
		FileList *addedOrModifiedFileHashes,
		FileList *deletedFiles,
		double *priorRowPatchTime,
		double *mostRecentRowPatchTime)=0;

	/* 返回值将在上述任意函数返回 false 时由 AutopatcherServer 发送给 AutopatcherClient。 */
	virtual const char *GetLastError(void) const=0;

	/* 返回值将作为 _chunkSize 参数传递给 FileListTransfer::Send()。 */
	virtual const int GetIncrementalReadChunkSize(void) const=0;
};

} /* RakNet 命名空间 */
