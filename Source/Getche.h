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

#if   defined(_WIN32)
#include <conio.h> /* getche() 回显式字符读取 */

#else
#include <termios.h>
#include <cstdio>
#include <unistd.h>
char getche();
#endif 
