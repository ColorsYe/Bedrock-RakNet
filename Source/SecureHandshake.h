/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  本源代码使用 BSD 风格许可证授权，
 *  许可证文件位于源码树根目录的 LICENSE 文件中。
 *  同目录下的 PATENTS 文件中还包含额外的专利授权。
 *
 */

/* 安全握手加密支持 */


#pragma once
#include "NativeFeatureIncludes.h"

#if LIBCAT_SECURITY==1

/* 如果构建 RakNet DLL，请确保调整 CAT_EXPORT 宏的含义 */
#if !defined(_RAKNET_LIB) && defined(_RAKNET_DLL)
# define CAT_BUILD_DLL
#else
# define CAT_NEUTER_EXPORT
#endif

/* 将 DependentExtensions 添加到您的包含路径中以包含此文件 */
#include "cat/AllTunnel.hpp"

#endif /* LIBCAT_SECURITY */
