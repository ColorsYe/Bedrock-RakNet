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
 *  CloudClient.h
 * 查询 CloudMemoryServer 以下载其他客户端上传的数据
 *
 */


#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_CloudClient==1

#pragma once
#include "PluginInterface2.h"
#include "CloudCommon.h"
#include "RakMemoryOverride.h"
#include "DS_Hash.h"

namespace RakNet
{
/* 前向声明 */
class RakPeerInterface;
class CloudClientCallback;

/*
 * \defgroup CLOUD_GROUP CloudComputing
 * 包含 CloudClient 和 CloudServer 插件
 * CloudServer 插件处理来自 CloudClient 插件的请求。服务器之间构成全连接网状拓扑，客户端可以连接到任意服务器。客户端之间可以通过发布和订阅内存更新来相互交互，无需直接连接甚至无需知道彼此的存在。
 * \ingroup PLUGINS_GROUP
 */

/*
 * 在 CloudMemoryServer 上执行 Post() 和 Get() 操作
 * CloudClient 是连接到一个或多个云配置服务器的计算机。一个 CloudClient 的操作可以被其他 CloudClient 实例接收和订阅，即使这些客户端之间没有直接连接，甚至在不同的服务器上。
 * \ingroup CLOUD_GROUP
 */
class RAK_DLL_EXPORT CloudClient : public PluginInterface2
{
public:
	/* 获取单例 GetInstance() 和销毁单例 DestroyInstance(instance*) */
	STATIC_FACTORY_DECLARATIONS(CloudClient)

	CloudClient();
	virtual ~CloudClient();

	/*
	 * 设置 OnGetReponse()、OnSubscriptionNotification() 和 OnSubscriptionDataDeleted() 的默认回调
	 * 如果需要，系统可以存储 CloudAllocator 和 CloudClientCallback 的指针。如果 OnGetReponse()、OnSubscriptionNotification()、OnSubscriptionDataDeleted() 未提供回调，则使用此处传入的回调。
	 * 参数[输入] _allocator An instance of CloudAllocator
	 * 参数[输入] _callback An instance of CloudClientCallback
	 */
	virtual void SetDefaultCallbacks(CloudAllocator *_allocator, CloudClientCallback *_callback);

	/*
	 * 上传数据到云
	 * Data uploaded to the cloud will be stored by the server sent to, identified by systemIdentifier.
	 * As long as you are connected to this server, the data will persist. Queries for that data by the Get() operation will
	 * 返回 RakNetGUID and SystemAddress of the uploader, as well as the data itself
	 * Furthermore, if any clients are subscribed to the particular CloudKey passed, those clients will get update notices that the data has changed
	 * Passing data with the same cloudKey more than once will overwrite the prior value.
	 * 如果超过 CloudServer::SetMaxUploadBytesPerClient()，此调用将静默失败
	 * 参数[输入] cloudKey Identifies the data being uploaded
	 * 参数[输入] data A pointer to data to upload. This pointer does not need to persist past the call
	 * 参数[输入] dataLengthBytes The length in bytes of data
	 * 参数[输入] systemIdentifier A remote system running CloudServer that we are already connected to.
	 */
	virtual void Post(CloudKey *cloudKey, const unsigned char *data, uint32_t dataLengthBytes, RakNetGUID systemIdentifier);

	/*
	 * 释放之前通过 Post() 上传的一条或多条数据
	 * If a remote system has subscribed to one or more of the keys uploaded, they will get ID_CLOUD_SUBSCRIPTION_NOTIFICATION notifications containing the last value uploaded before deletions
	 * 参数[输入] cloudKey Identifies the data to release. It is possible to remove uploads from multiple Post() calls at once.
	 * 参数[输入] systemIdentifier A remote system running CloudServer that we are already connected to.
	 */
	virtual void Release(DataStructures::List<CloudKey> &keys, RakNetGUID systemIdentifier);

	/*
	 * 从云获取数据
	 * 对于包含一个或多个键的给定查询，返回匹配这些键的数据。
	 * 值将在 ID_CLOUD_GET_RESPONSE 数据包中返回，该包应传递给 OnGetReponse()，并会调用 CloudClientCallback::OnGet()
	 * CloudQuery::startingRowIndex is used to skip the first n values that would normally be returned..
	 * CloudQuery::maxRowsToReturn is used to limit the number of rows returned. The number of rows returned may also be limited by CloudServer::SetMaxBytesPerDownload();
	 * CloudQuery::subscribeToResults if set to true, will cause ID_CLOUD_SUBSCRIPTION_NOTIFICATION to be returned to us when any of the keys in the query are updated or are deleted.
	 * ID_CLOUD_GET_RESPONSE will be returned even if subscribing to the result list. Only later updates will return ID_CLOUD_SUBSCRIPTION_NOTIFICATION.
	 * Calling Get() with CloudQuery::subscribeToResults false, when you are already subscribed, does not remove the subscription. Use Unsubscribe() for this.
	 * Resubscribing using the same CloudKey but a different or no specificSystems overwrites the subscribed systems for those keys.
	 * 参数[输入] cloudQuery One or more keys, and optional parameters to perform with the Get
	 * 参数[输入] systemIdentifier A remote system running CloudServer that we are already connected to.
	 * 参数[输入] specificSystems It is possible to get or subscribe to updates only for specific uploading CloudClient instances. Pass the desired instances here. The overload that does not have the specificSystems parameter is treated as subscribing to all updates from all clients.
	 */
	virtual bool Get(CloudQuery *cloudQuery, RakNetGUID systemIdentifier);
	virtual bool Get(CloudQuery *cloudQuery, DataStructures::List<RakNetGUID> &specificSystems, RakNetGUID systemIdentifier);
	virtual bool Get(CloudQuery *cloudQuery, DataStructures::List<CloudQueryRow*> &specificSystems, RakNetGUID systemIdentifier);

	/*
	 * Unsubscribe from updates previously subscribed to using Get() with the CloudQuery::subscribeToResults set to true
	 * 在服务器上检查订阅时，keys 和 specificSystems 参数逻辑上被视为 AND 关系
	 * 不接受 specificSystems 的重载版本将取消所有传入键的订阅，不论系统
	 * You cannot unsubscribe specific systems when previously subscribed to updates from any system. To do this, first Unsubscribe() from all systems, and call Get() with the specificSystems parameter explicilty listing the systems you want to subscribe to.
	 */
	virtual void Unsubscribe(DataStructures::List<CloudKey> &keys, RakNetGUID systemIdentifier);
	virtual void Unsubscribe(DataStructures::List<CloudKey> &keys, DataStructures::List<RakNetGUID> &specificSystems, RakNetGUID systemIdentifier);
	virtual void Unsubscribe(DataStructures::List<CloudKey> &keys, DataStructures::List<CloudQueryRow*> &specificSystems, RakNetGUID systemIdentifier);

	/*
	 * Call this when you get ID_CLOUD_GET_RESPONSE
	 * 如果 callback 或 allocator 为 0，则使用传递给 SetDefaultCallbacks() 的默认回调
	 * 参数[输入] packet Packet structure returned from RakPeerInterface
	 * 参数[输入] _callback Callback to be called from the function containing output parameters. If 0, default is used.
	 * 参数[输入] _allocator Allocator to be used to allocate data. If 0, default is used.
	 */
	virtual void OnGetReponse(Packet *packet, CloudClientCallback *_callback=0, CloudAllocator *_allocator=0);

	/*
	 * Call this when you get ID_CLOUD_GET_RESPONSE
	 * Different form of OnGetReponse that returns to a structure that you pass, instead of using a callback
	 * 使用此形式时，你负责进行释放
	 * 如果 allocator 为 0，则使用传递给 SetDefaultCallbacks() 的默认回调
	 * 参数[输出] cloudQueryResult A pointer to a structure that will be filled out with data
	 * 参数[输入] packet Packet structure returned from RakPeerInterface
	 * 参数[输入] _allocator Allocator to be used to allocate data. If 0, default is used.
	 */
	virtual void OnGetReponse(CloudQueryResult *cloudQueryResult, Packet *packet, CloudAllocator *_allocator=0);

	/*
	 * Call this when you get ID_CLOUD_SUBSCRIPTION_NOTIFICATION
	 * 如果 callback 或 allocator 为 0，则使用传递给 SetDefaultCallbacks() 的默认回调
	 * 参数[输入] packet Packet structure returned from RakPeerInterface
	 * 参数[输入] _callback Callback to be called from the function containing output parameters. If 0, default is used.
	 * 参数[输入] _allocator Allocator to be used to allocate data. If 0, default is used.
	 */
	virtual void OnSubscriptionNotification(Packet *packet, CloudClientCallback *_callback=0, CloudAllocator *_allocator=0);

	/*
	 * Call this when you get ID_CLOUD_SUBSCRIPTION_NOTIFICATION
	 * Different form of OnSubscriptionNotification that returns to a structure that you pass, instead of using a callback
	 * 使用此形式时，你负责进行释放
	 * 如果 allocator 为 0，则使用传递给 SetDefaultCallbacks() 的默认回调
	 * 参数[输出] wasUpdated If true, the row was updated. If false, it was deleted. result will contain the last value just before deletion
	 * 参数[输出] row A pointer to a structure that will be filled out with data
	 * 参数[输入] packet Packet structure returned from RakPeerInterface
	 * 参数[输入] _allocator Allocator to be used to allocate data. If 0, default is used.
	 */
	virtual void OnSubscriptionNotification(bool *wasUpdated, CloudQueryRow *row, Packet *packet, CloudAllocator *_allocator=0);

	/* If you never specified an allocator, and used the non-callback form of OnGetReponse(), deallocate cloudQueryResult with this function */
	virtual void DeallocateWithDefaultAllocator(CloudQueryResult *cloudQueryResult);

	/* If you never specified an allocator, and used the non-callback form of OnSubscriptionNotification(), deallocate row with this function */
	virtual void DeallocateWithDefaultAllocator(CloudQueryRow *row);

protected:
	PluginReceiveResult OnReceive(Packet *packet) override;

	CloudClientCallback *callback;
	CloudAllocator *allocator;

	CloudAllocator unsetDefaultAllocator;
};

/*
 * \ingroup CLOUD_GROUP
 * Parses ID_CLOUD_GET_RESPONSE and ID_CLOUD_SUBSCRIPTION_NOTIFICATION in a convenient callback form
 */
class RAK_DLL_EXPORT CloudClientCallback
{
public:
	CloudClientCallback() {}
	virtual ~CloudClientCallback() {}

	/*
	 * Called in response to ID_CLOUD_GET_RESPONSE
	 * 参数[输出] result Contains the original query passed to Get(), and a list of rows returned.
	 * 参数[输出] deallocateRowsAfterReturn CloudQueryResult::rowsReturned will be deallocated after the function returns by default. Set to false to not deallocate these pointers. The pointers are allocated through CloudAllocator.
	 */
	virtual void OnGet(RakNet::CloudQueryResult *result, bool *deallocateRowsAfterReturn) {(void) result; (void) deallocateRowsAfterReturn;}

	/*
	 * Called in response to ID_CLOUD_SUBSCRIPTION_NOTIFICATION
	 * 参数[输出] result 包含 row updated
	 * 参数[输出] wasUpdated If true, the row was updated. If false, it was deleted. result will contain the last value just before deletion
	 * 参数[输出] deallocateRowAfterReturn result will be deallocated after the function returns by default. Set to false to not deallocate these pointers. The pointers are allocated through CloudAllocator.
	 */
	virtual void OnSubscriptionNotification(RakNet::CloudQueryRow *result, bool wasUpdated, bool *deallocateRowAfterReturn) {(void) result; (void) wasUpdated; (void) deallocateRowAfterReturn;}
};

} /* RakNet 命名空间 */

#endif
