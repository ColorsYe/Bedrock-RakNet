/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  本源代码使用 BSD 风格许可证授权，
 *  许可证文件位于源码树根目录的 LICENSE 文件中。
 *  同目录下的 PATENTS 文件中还包含额外的专利授权。
 *
 */

/* Linux/UNIX 平台下的字符串兼容函数 */
#pragma once
#if  defined(__native_client__)
	#ifndef _stricmp
		int _stricmp(const char* s1, const char* s2);
	#endif
	int _strnicmp(const char* s1, const char* s2, size_t n);
	char *_strlwr(char * str );
	#define _vsnprintf vsnprintf
#else
 #if (defined(__GNUC__)  || defined(__GCCXML__) || defined(__S3E__) ) && !defined(_WIN32)
		#ifndef _stricmp
			int _stricmp(const char* s1, const char* s2);
		#endif 
		int _strnicmp(const char* s1, const char* s2, size_t n);
		/* 参考: http://www.jenkinssoftware.com/forum/index.php?topic=5010.msg20920#msg20920 */
     /*   #ifndef _vsnprintf */
		    #define _vsnprintf vsnprintf
       /* #endif */
#ifndef __APPLE__
		char *_strlwr(char * str ); /* 在 OSX 上由于某种原因无法编译 */
#endif



	#endif
#endif
