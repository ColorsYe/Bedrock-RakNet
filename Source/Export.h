/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  本源代码使用 BSD 风格许可证授权，
 *  许可证文件位于源码树根目录的 LICENSE 文件中。
 *  同目录下的 PATENTS 文件中还包含额外的专利授权。
 *
 */

#include "RakNetDefines.h"

/* Windows 平台下的 DLL 导出宏定义 */
#if defined(_WIN32) && !(defined(__GNUC__)  || defined(__GCCXML__)) && !defined(_RAKNET_LIB) && defined(_RAKNET_DLL)
#define RAK_DLL_EXPORT __declspec(dllexport)
#else
#define RAK_DLL_EXPORT  
#endif

/* 静态工厂模式的声明宏：声明 GetInstance() 和 DestroyInstance() */
#define STATIC_FACTORY_DECLARATIONS(x) static x* GetInstance(); \
static void DestroyInstance( x *i);

/* 静态工厂模式的定义宏：实现 GetInstance() 和 DestroyInstance() */
#define STATIC_FACTORY_DEFINITIONS(x,y) x* x::GetInstance() {return RakNet::OP_NEW<y>( _FILE_AND_LINE_ );} \
void x::DestroyInstance( x *i) {RakNet::OP_DELETE(( y* ) i, _FILE_AND_LINE_);}
