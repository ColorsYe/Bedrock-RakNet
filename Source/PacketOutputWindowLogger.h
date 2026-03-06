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
 * 
 * 将所有传入和传出的网络消息写入文件
 *
 */


#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_PacketLogger==1

#pragma once
#include "PacketLogger.h"

namespace RakNet
{

/*
 * \ingroup PACKETLOGGER_GROUP
 * Packetlogger that outputs to the output window in the debugger. Windows only.
 */
class RAK_DLL_EXPORT  PacketOutputWindowLogger : public PacketLogger
{
public:
	PacketOutputWindowLogger();
	virtual ~PacketOutputWindowLogger();
	void WriteLog(const char *str) override;
protected:
};

} /* RakNet 命名空间 */

#endif
