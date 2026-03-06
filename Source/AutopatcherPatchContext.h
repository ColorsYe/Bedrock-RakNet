/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  本源代码使用 BSD 风格许可证授权，
 *  许可证文件位于源码树根目录的 LICENSE 文件中。
 *  同目录下的 PATENTS 文件中还包含额外的专利授权。
 *
 */

#pragma once
enum class PatchContext
{
	PC_HASH_1_WITH_PATCH,       /* 使用哈希方法1进行补丁 */
	PC_HASH_2_WITH_PATCH,       /* 使用哈希方法2进行补丁 */
	PC_WRITE_FILE,              /* 写入文件 */
	PC_ERROR_FILE_WRITE_FAILURE,          /* 错误：文件写入失败 */
	PC_ERROR_PATCH_TARGET_MISSING,        /* 错误：补丁目标文件缺失 */
	PC_ERROR_PATCH_APPLICATION_FAILURE,   /* 错误：补丁应用失败 */
	PC_ERROR_PATCH_RESULT_CHECKSUM_FAILURE, /* 错误：补丁结果校验和失败 */
	PC_NOTICE_WILL_COPY_ON_RESTART,       /* 通知：将在重启时复制 */
	PC_NOTICE_FILE_DOWNLOADED,            /* 通知：文件已下载 */
	PC_NOTICE_FILE_DOWNLOADED_PATCH,      /* 通知：补丁文件已下载 */
};
