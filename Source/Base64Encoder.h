/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#pragma once
#include "Export.h"

extern "C" {
/*
 * 返回写入的字节数。
 * outputData 的大小至少应为 inputData * 2 + 6。
 */
int Base64Encoding(const unsigned char *inputData, int dataLength, char *outputData);
}

extern "C" {
const char *Base64Map();
}
