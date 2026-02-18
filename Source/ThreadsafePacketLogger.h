/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/// \file
/// \brief Derivation of the packet logger to defer the call to WriteLog until the user thread.
///


#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_PacketLogger==1

#pragma once
#include "PacketLogger.h"
#include "SingleProducerConsumer.h"

namespace RakNet
{

/// \ingroup PACKETLOGGER_GROUP
/// \brief Same as PacketLogger, but writes output in the user thread.
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

} // namespace RakNet

#endif
