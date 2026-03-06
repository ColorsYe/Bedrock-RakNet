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
 * Derivation of the packet logger to defer the call to WriteLog until the user thread.
 *
 */


#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_PacketLogger==1

#pragma once
#include "PacketLogger.h"
#include "SingleProducerConsumer.h"

namespace RakNet
{

/*
 * \ingroup PACKETLOGGER_GROUP
 * 与 PacketLogger, but writes output in the user thread 相同
 */
class RAK_DLL_EXPORT ThreadsafePacketLogger : public PacketLogger
{
public:
	ThreadsafePacketLogger();
	virtual ~ThreadsafePacketLogger();

	void Update() override;

protected:
	virtual void AddToLog(const char *str);

	DataStructures::SingleProducerConsumer<char*> logMessages;
};

} /* RakNet 命名空间 */

#endif
