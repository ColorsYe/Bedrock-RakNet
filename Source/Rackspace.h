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
 *  Rackspace.h
 * Helper to class to manage Rackspace servers
 *
 */


#include "NativeFeatureIncludes.h"

#if _RAKNET_SUPPORT_Rackspace==1 && _RAKNET_SUPPORT_TCPInterface==1

#include "Export.h"
#include "DS_List.h"
#include "RakNetTypes.h"
#include "DS_Queue.h"
#include "RakString.h"

#pragma once
namespace RakNet
{

	class TCPInterface;
	struct Packet;

	/*
	 * Result codes for Rackspace commands
	 * /sa Rackspace::EventTypeToString()
	 */
	enum RackspaceEventType
	{
		RET_Success_200,
		RET_Success_201,
		RET_Success_202,
		RET_Success_203,
		RET_Success_204,
		RET_Cloud_Servers_Fault_500,
		RET_Service_Unavailable_503,
		RET_Unauthorized_401,
		RET_Bad_Request_400,
		RET_Over_Limit_413,
		RET_Bad_Media_Type_415,
		RET_Item_Not_Found_404,
		RET_Build_In_Progress_409,
		RET_Resize_Not_Allowed_403,
		RET_Connection_Closed_Without_Reponse,
		RET_Unknown_Failure,
	};

	/* 内部使用 */
	enum RackspaceOperationType
	{
		RO_CONNECT_AND_AUTHENTICATE,
		RO_LIST_SERVERS,
		RO_LIST_SERVERS_WITH_DETAILS,
		RO_CREATE_SERVER,
		RO_GET_SERVER_DETAILS,
		RO_UPDATE_SERVER_NAME_OR_PASSWORD,
		RO_DELETE_SERVER,
		RO_LIST_SERVER_ADDRESSES,
		RO_SHARE_SERVER_ADDRESS,
		RO_DELETE_SERVER_ADDRESS,
		RO_REBOOT_SERVER,
		RO_REBUILD_SERVER,
		RO_RESIZE_SERVER,
		RO_CONFIRM_RESIZED_SERVER,
		RO_REVERT_RESIZED_SERVER,
		RO_LIST_FLAVORS,
		RO_GET_FLAVOR_DETAILS,
		RO_LIST_IMAGES,
		RO_CREATE_IMAGE,
		RO_GET_IMAGE_DETAILS,
		RO_DELETE_IMAGE,
		RO_LIST_SHARED_IP_GROUPS,
		RO_LIST_SHARED_IP_GROUPS_WITH_DETAILS,
		RO_CREATE_SHARED_IP_GROUP,
		RO_GET_SHARED_IP_GROUP_DETAILS,
		RO_DELETE_SHARED_IP_GROUP,

		RO_NONE,
	};

	/* Callback interface to receive the results of operations */
	class RAK_DLL_EXPORT Rackspace2EventCallback
	{
	public:
		Rackspace2EventCallback() {}
		virtual ~Rackspace2EventCallback() {}
		virtual void OnAuthenticationResult(RackspaceEventType eventType, const char *htmlAdditionalInfo)=0;
		virtual void OnListServersResult(RackspaceEventType eventType, const char *htmlAdditionalInfo)=0;
		virtual void OnListServersWithDetailsResult(RackspaceEventType eventType, const char *htmlAdditionalInfo)=0;
		virtual void OnCreateServerResult(RackspaceEventType eventType, const char *htmlAdditionalInfo)=0;
		virtual void OnGetServerDetails(RackspaceEventType eventType, const char *htmlAdditionalInfo)=0;
		virtual void OnUpdateServerNameOrPassword(RackspaceEventType eventType, const char *htmlAdditionalInfo)=0;
		virtual void OnDeleteServer(RackspaceEventType eventType, const char *htmlAdditionalInfo)=0;
		virtual void OnListServerAddresses(RackspaceEventType eventType, const char *htmlAdditionalInfo)=0;
		virtual void OnShareServerAddress(RackspaceEventType eventType, const char *htmlAdditionalInfo)=0;
		virtual void OnDeleteServerAddress(RackspaceEventType eventType, const char *htmlAdditionalInfo)=0;
		virtual void OnRebootServer(RackspaceEventType eventType, const char *htmlAdditionalInfo)=0;
		virtual void OnRebuildServer(RackspaceEventType eventType, const char *htmlAdditionalInfo)=0;
		virtual void OnResizeServer(RackspaceEventType eventType, const char *htmlAdditionalInfo)=0;
		virtual void OnConfirmResizedServer(RackspaceEventType eventType, const char *htmlAdditionalInfo)=0;
		virtual void OnRevertResizedServer(RackspaceEventType eventType, const char *htmlAdditionalInfo)=0;
		virtual void OnListFlavorsResult(RackspaceEventType eventType, const char *htmlAdditionalInfo)=0;
		virtual void OnGetFlavorDetailsResult(RackspaceEventType eventType, const char *htmlAdditionalInfo)=0;
		virtual void OnListImagesResult(RackspaceEventType eventType, const char *htmlAdditionalInfo)=0;
		virtual void OnCreateImageResult(RackspaceEventType eventType, const char *htmlAdditionalInfo)=0;
		virtual void OnGetImageDetailsResult(RackspaceEventType eventType, const char *htmlAdditionalInfo)=0;
		virtual void OnDeleteImageResult(RackspaceEventType eventType, const char *htmlAdditionalInfo)=0;
		virtual void OnListSharedIPGroups(RackspaceEventType eventType, const char *htmlAdditionalInfo)=0;
		virtual void OnListSharedIPGroupsWithDetails(RackspaceEventType eventType, const char *htmlAdditionalInfo)=0;
		virtual void OnCreateSharedIPGroup(RackspaceEventType eventType, const char *htmlAdditionalInfo)=0;
		virtual void OnGetSharedIPGroupDetails(RackspaceEventType eventType, const char *htmlAdditionalInfo)=0;
		virtual void OnDeleteSharedIPGroup(RackspaceEventType eventType, const char *htmlAdditionalInfo)=0;

		virtual void OnConnectionAttemptFailure(RackspaceOperationType operationType, const char *url)=0;
	};

	/* Callback interface to receive the results of operations, with a default result */
	class RAK_DLL_EXPORT RackspaceEventCallback_Default : public Rackspace2EventCallback
	{
	public:
		virtual void ExecuteDefault(const char *callbackName, RackspaceEventType eventType, const char *htmlAdditionalInfo) {(void) callbackName; (void) eventType; (void) htmlAdditionalInfo;}

		virtual void OnAuthenticationResult(RackspaceEventType eventType, const char *htmlAdditionalInfo) {ExecuteDefault("OnAuthenticationResult", eventType, htmlAdditionalInfo);}
		virtual void OnListServersResult(RackspaceEventType eventType, const char *htmlAdditionalInfo) {ExecuteDefault("OnListServersResult", eventType, htmlAdditionalInfo);}
		virtual void OnListServersWithDetailsResult(RackspaceEventType eventType, const char *htmlAdditionalInfo) {ExecuteDefault("OnListServersWithDetailsResult", eventType, htmlAdditionalInfo);}
		virtual void OnCreateServerResult(RackspaceEventType eventType, const char *htmlAdditionalInfo) {ExecuteDefault("OnCreateServerResult", eventType, htmlAdditionalInfo);}
		virtual void OnGetServerDetails(RackspaceEventType eventType, const char *htmlAdditionalInfo) {ExecuteDefault("OnGetServerDetails", eventType, htmlAdditionalInfo);}
		virtual void OnUpdateServerNameOrPassword(RackspaceEventType eventType, const char *htmlAdditionalInfo) {ExecuteDefault("OnUpdateServerNameOrPassword", eventType, htmlAdditionalInfo);}
		virtual void OnDeleteServer(RackspaceEventType eventType, const char *htmlAdditionalInfo) {ExecuteDefault("OnDeleteServer", eventType, htmlAdditionalInfo);}
		virtual void OnListServerAddresses(RackspaceEventType eventType, const char *htmlAdditionalInfo) {ExecuteDefault("OnListServerAddresses", eventType, htmlAdditionalInfo);}
		virtual void OnShareServerAddress(RackspaceEventType eventType, const char *htmlAdditionalInfo) {ExecuteDefault("OnShareServerAddress", eventType, htmlAdditionalInfo);}
		virtual void OnDeleteServerAddress(RackspaceEventType eventType, const char *htmlAdditionalInfo) {ExecuteDefault("OnDeleteServerAddress", eventType, htmlAdditionalInfo);}
		virtual void OnRebootServer(RackspaceEventType eventType, const char *htmlAdditionalInfo) {ExecuteDefault("OnRebootServer", eventType, htmlAdditionalInfo);}
		virtual void OnRebuildServer(RackspaceEventType eventType, const char *htmlAdditionalInfo) {ExecuteDefault("OnRebuildServer", eventType, htmlAdditionalInfo);}
		virtual void OnResizeServer(RackspaceEventType eventType, const char *htmlAdditionalInfo) {ExecuteDefault("OnResizeServer", eventType, htmlAdditionalInfo);}
		virtual void OnConfirmResizedServer(RackspaceEventType eventType, const char *htmlAdditionalInfo) {ExecuteDefault("OnConfirmResizedServer", eventType, htmlAdditionalInfo);}
		virtual void OnRevertResizedServer(RackspaceEventType eventType, const char *htmlAdditionalInfo) {ExecuteDefault("OnRevertResizedServer", eventType, htmlAdditionalInfo);}
		virtual void OnListFlavorsResult(RackspaceEventType eventType, const char *htmlAdditionalInfo) {ExecuteDefault("OnListFlavorsResult", eventType, htmlAdditionalInfo);}
		virtual void OnGetFlavorDetailsResult(RackspaceEventType eventType, const char *htmlAdditionalInfo) {ExecuteDefault("OnGetFlavorDetailsResult", eventType, htmlAdditionalInfo);}
		virtual void OnListImagesResult(RackspaceEventType eventType, const char *htmlAdditionalInfo) {ExecuteDefault("OnListImagesResult", eventType, htmlAdditionalInfo);}
		virtual void OnCreateImageResult(RackspaceEventType eventType, const char *htmlAdditionalInfo) {ExecuteDefault("OnCreateImageResult", eventType, htmlAdditionalInfo);}
		virtual void OnGetImageDetailsResult(RackspaceEventType eventType, const char *htmlAdditionalInfo) {ExecuteDefault("OnGetImageDetailsResult", eventType, htmlAdditionalInfo);}
		virtual void OnDeleteImageResult(RackspaceEventType eventType, const char *htmlAdditionalInfo) {ExecuteDefault("OnDeleteImageResult", eventType, htmlAdditionalInfo);}
		virtual void OnListSharedIPGroups(RackspaceEventType eventType, const char *htmlAdditionalInfo) {ExecuteDefault("OnListSharedIPGroups", eventType, htmlAdditionalInfo);}
		virtual void OnListSharedIPGroupsWithDetails(RackspaceEventType eventType, const char *htmlAdditionalInfo) {ExecuteDefault("OnListSharedIPGroupsWithDetails", eventType, htmlAdditionalInfo);}
		virtual void OnCreateSharedIPGroup(RackspaceEventType eventType, const char *htmlAdditionalInfo) {ExecuteDefault("OnCreateSharedIPGroup", eventType, htmlAdditionalInfo);}
		virtual void OnGetSharedIPGroupDetails(RackspaceEventType eventType, const char *htmlAdditionalInfo) {ExecuteDefault("OnGetSharedIPGroupDetails", eventType, htmlAdditionalInfo);}
		virtual void OnDeleteSharedIPGroup(RackspaceEventType eventType, const char *htmlAdditionalInfo) {ExecuteDefault("OnDeleteSharedIPGroup", eventType, htmlAdditionalInfo);}

		virtual void OnConnectionAttemptFailure(RackspaceOperationType operationType, const char *url) {(void) operationType; (void) url;}
	};

	/*
	 * Code that uses the TCPInterface class to communicate with the Rackspace API servers
	 * 前提条件: Compile RakNet with OPEN_SSL_CLIENT_SUPPORT set to 1
	 * 前提条件: Packets returned from TCPInterface::OnReceive() must be passed to Rackspace::OnReceive()
	 * 前提条件: Packets returned from TCPInterface::HasLostConnection() must be passed to Rackspace::OnClosedConnection()
	 */
	class RAK_DLL_EXPORT Rackspace
	{
	public:
		Rackspace();
		~Rackspace() noexcept;

		/*
		 * Authenticate with Rackspace servers, 必须 before executing any commands.
		 * All requests to authenticate and operate against Cloud Servers are performed using SSL over HTTP (HTTPS) on TCP port 443.
		 * Times out after 24 hours - if you get RET_Authenticate_Unauthorized in the RackspaceEventCallback callback, call again
		 * 另见 RackspaceEventCallback::OnAuthenticationResult()
		 * 参数[输入] _tcpInterface An instance of TCPInterface, build with OPEN_SSL_CLIENT_SUPPORT 1 and already started
		 * 参数[输入] _authenticationURL See http://docs.rackspacecloud.com/servers/api/v1.0/cs-devguide-20110112.pdf . US-based accounts authenticate through auth.api.rackspacecloud.com. UK-based accounts authenticate through lon.auth.api.rackspacecloud.com
		 * 参数[输入] _rackspaceCloudUsername Username you registered with Rackspace on their website
		 * 参数[输入] _apiAccessKey Obtain your API access key from the Rackspace Cloud Control Panel in the Your Account API Access section.
		 * 返回值: The address of the authentication server, or UNASSIGNED_SYSTEM_ADDRESS if the connection attempt failed
		 */
		SystemAddress Authenticate(TCPInterface *_tcpInterface, const char *_authenticationURL, const char *_rackspaceCloudUsername, const char *_apiAccessKey);

		/*
		 * 获取 a list of running servers
		 * 另见 http://docs.rackspacecloud.com/servers/api/v1.0/cs-devguide-20110112.pdf
		 * 另见 RackspaceEventCallback::OnListServersResult()
		 */
		void ListServers();

		/*
		 * 获取 a list of running servers, with extended details on each server
		 * 另见 GetServerDetails()
		 * 另见 http://docs.rackspacecloud.com/servers/api/v1.0/cs-devguide-20110112.pdf
		 * 另见 RackspaceEventCallback::OnListServersWithDetailsResult()
		 */
		void ListServersWithDetails();

		/*
		 * 创建 server
		 * Create a server with a given image (harddrive contents) and flavor (hardware configuration)
		 * 获取 available images with ListImages()
		 * 获取 available flavors with ListFlavors()
		 * It is possible to configure the server in more detail. See the XML schema at http://docs.rackspacecloud.com/servers/api/v1.0
		 * You can execute such a custom command by calling AddOperation() manually. See the implementation of CreateServer for how to do so.
		 * The server takes a while to build. Call GetServerDetails() to get the current build status. Server id to pass to GetServerDetails() is returned in the field <server ... id="1234">
		 * 另见 http://docs.rackspacecloud.com/servers/api/v1.0/cs-devguide-20110112.pdf
		 * 另见 RackspaceEventCallback::OnCreateServerResult()
		 * 参数[输入] name Name of the server. Only alphanumeric characters, periods, and hyphens are valid. Server Name cannot start or end with a period or hyphen.
		 * 参数[输入] imageId Which image (harddrive contents, including OS) to use
		 * 参数[输入] flavorId Which flavor (hardware config) to use, primarily how much memory is available.
		 */
		void CreateServer(RakNet::RakString name, RakNet::RakString imageId, RakNet::RakString flavorId);

		/*
		 * 获取 details on a particular server
		 * 另见 http://docs.rackspacecloud.com/servers/api/v1.0/cs-devguide-20110112.pdf
		 * 另见 RackspaceEventCallback::OnGetServerDetailsResult()
		 * 参数[输入] serverId Which server to get details on. You can call ListServers() to get the list of active servers.
		 */
		void GetServerDetails(RakNet::RakString serverId);

		/*
		 * Changes the name or password for a server
		 * 另见 http://docs.rackspacecloud.com/servers/api/v1.0/cs-devguide-20110112.pdf
		 * 另见 RackspaceEventCallback::OnUpdateServerNameOrPasswordResult()
		 * 参数[输入] serverId Which server to get details on. You can call ListServers() to get the list of active servers.
		 * 参数[输入] newName The new server name. Leave blank to leave unchanged. Only alphanumeric characters, periods, and hyphens are valid. Server Name cannot start or end with a period or hyphen.
		 * 参数[输入] newPassword The new server password. Leave blank to leave unchanged.
		 */
		void UpdateServerNameOrPassword(RakNet::RakString serverId, RakNet::RakString newName, RakNet::RakString newPassword);

		/*
		 * 删除 server
		 * 另见 http://docs.rackspacecloud.com/servers/api/v1.0/cs-devguide-20110112.pdf
		 * 另见 RackspaceEventCallback::OnDeleteServerResult()
		 * 参数[输入] serverId Which server to get details on. You can call ListServers() to get the list of active servers.
		 */
		void DeleteServer(RakNet::RakString serverId);
		
		/*
		 * Lists the IP addresses available to a server
		 * 另见 http://docs.rackspacecloud.com/servers/api/v1.0/cs-devguide-20110112.pdf
		 * 另见 RackspaceEventCallback::OnListServerAddressesResult()
		 * 参数[输入] serverId Which server to operate on. You can call ListServers() to get the list of active servers.
		 */
		void ListServerAddresses(RakNet::RakString serverId);

		/*
		 * Shares an IP address with a server
		 * 另见 http://docs.rackspacecloud.com/servers/api/v1.0/cs-devguide-20110112.pdf
		 * 另见 RackspaceEventCallback::OnShareServerAddressResult()
		 * 参数[输入] serverId Which server to operate on. You can call ListServers() to get the list of active servers.
		 * 参数[输入] ipAddress Which IP address. You can call ListServerAddresses() to get the list of addresses for the specified server
		 */
		void ShareServerAddress(RakNet::RakString serverId, RakNet::RakString ipAddress);

		/*
		 * Stops sharing an IP address with a server
		 * 另见 http://docs.rackspacecloud.com/servers/api/v1.0/cs-devguide-20110112.pdf
		 * 另见 RackspaceEventCallback::OnDeleteServerAddressResult()
		 * 参数[输入] serverId Which server to operate on. You can call ListServers() to get the list of active servers.
		 * 参数[输入] ipAddress Which IP address. You can call ListServerAddresses() to get the list of addresses for the specified server
		 */
		void DeleteServerAddress(RakNet::RakString serverId, RakNet::RakString ipAddress);

		/*
		 * Reboots a server
		 * 另见 http://docs.rackspacecloud.com/servers/api/v1.0/cs-devguide-20110112.pdf
		 * 另见 RackspaceEventCallback::OnRebootServerResult()
		 * 参数[输入] serverId Which server to operate on. You can call ListServers() to get the list of active servers.
		 * 参数[输入] rebootType Should be either "HARD" or "SOFT"
		 */
		void RebootServer(RakNet::RakString serverId, RakNet::RakString rebootType);

		/*
		 * Rebuilds a server with a different image (harddrive contents)
		 * 另见 http://docs.rackspacecloud.com/servers/api/v1.0/cs-devguide-20110112.pdf
		 * 另见 RackspaceEventCallback::OnRebuildServerResult()
		 * 参数[输入] serverId Which server to operate on. You can call ListServers() to get the list of active servers.
		 * 参数[输入] imageId Which image (harddrive contents, including OS) to use
		 */
		void RebuildServer(RakNet::RakString serverId, RakNet::RakString imageId);

		/*
		 * Changes the hardware configuration of a server. This does not take effect until you call ConfirmResizedServer()
		 * 另见 http://docs.rackspacecloud.com/servers/api/v1.0/cs-devguide-20110112.pdf
		 * 另见 RackspaceEventCallback::OnResizeServerResult()
		 * 另见 RevertResizedServer()
		 * 参数[输入] serverId Which server to operate on. You can call ListServers() to get the list of active servers.
		 * 参数[输入] flavorId Which flavor (hardware config) to use, primarily how much memory is available.
		 */
		void ResizeServer(RakNet::RakString serverId, RakNet::RakString flavorId);

		/*
		 * Confirm a resize for the specified server
		 * 另见 http://docs.rackspacecloud.com/servers/api/v1.0/cs-devguide-20110112.pdf
		 * 另见 RackspaceEventCallback::OnConfirmResizedServerResult()
		 * 另见 ResizeServer()
		 * 参数[输入] serverId Which server to operate on. You can call ListServers() to get the list of active servers.
		 */
		void ConfirmResizedServer(RakNet::RakString serverId);

		/*
		 * Reverts a resize for the specified server
		 * 另见 http://docs.rackspacecloud.com/servers/api/v1.0/cs-devguide-20110112.pdf
		 * 另见 RackspaceEventCallback::OnRevertResizedServerResult()
		 * 另见 ResizeServer()
		 * 参数[输入] serverId Which server to operate on. You can call ListServers() to get the list of active servers.
		 */
		void RevertResizedServer(RakNet::RakString serverId);

		/*
		 * List all flavors (hardware configs, primarily memory)
		 * 另见 http://docs.rackspacecloud.com/servers/api/v1.0/cs-devguide-20110112.pdf
		 * 另见 RackspaceEventCallback::OnListFlavorsResult()
		 */
		void ListFlavors();

		/*
		 * 获取 extended details about a specific flavor
		 * 另见 http://docs.rackspacecloud.com/servers/api/v1.0/cs-devguide-20110112.pdf
		 * 另见 RackspaceEventCallback::OnGetFlavorDetailsResult()
		 * 另见 ListFlavors()
		 * 参数[输入] flavorId Which flavor (hardware config)
		 */
		void GetFlavorDetails(RakNet::RakString flavorId);

		/*
		 * List all images (software configs, including operating systems), which includes images you create yourself
		 * 另见 http://docs.rackspacecloud.com/servers/api/v1.0/cs-devguide-20110112.pdf
		 * 另见 RackspaceEventCallback::OnListImagesResult()
		 * 另见 CreateImage()
		 */
		void ListImages();

		/*
		 * Images a running server. This essentially copies the harddrive, and lets you start a server with the same harddrive contents later
		 * 另见 http://docs.rackspacecloud.com/servers/api/v1.0/cs-devguide-20110112.pdf
		 * 另见 RackspaceEventCallback::OnCreateImageResult()
		 * 另见 ListImages()
		 * 参数[输入] serverId Which server to operate on. You can call ListServers() to get the list of active servers.
		 * 参数[输入] imageName What to call this image
		 */
		void CreateImage(RakNet::RakString serverId, RakNet::RakString imageName);

		/*
		 * 获取 extended details about a particular image
		 * 另见 http://docs.rackspacecloud.com/servers/api/v1.0/cs-devguide-20110112.pdf
		 * 另见 RackspaceEventCallback::OnGetImageDetailsResult()
		 * 另见 ListImages()
		 * 参数[输入] imageId Which image
		 */
		void GetImageDetails(RakNet::RakString imageId);

		/*
		 * Delete a custom image created with CreateImage()
		 * 另见 http://docs.rackspacecloud.com/servers/api/v1.0/cs-devguide-20110112.pdf
		 * 另见 RackspaceEventCallback::OnDeleteImageResult()
		 * 另见 ListImages()
		 * 参数[输入] imageId Which image
		 */
		void DeleteImage(RakNet::RakString imageId);

		/*
		 * List IP groups
		 * 另见 http://docs.rackspacecloud.com/servers/api/v1.0/cs-devguide-20110112.pdf
		 * 另见 RackspaceEventCallback::OnListSharedIPGroupsResult()
		 */
		void ListSharedIPGroups();

		/*
		 * List IP groups with extended details
		 * 另见 http://docs.rackspacecloud.com/servers/api/v1.0/cs-devguide-20110112.pdf
		 * 另见 RackspaceEventCallback::OnListSharedIPGroupsWithDetailsResult()
		 */
		void ListSharedIPGroupsWithDetails();

		/* 我不知道这个是做什么的 */
		void CreateSharedIPGroup(RakNet::RakString name, RakNet::RakString optionalServerId);
		/* 我不知道这个是做什么的 */
		void GetSharedIPGroupDetails(RakNet::RakString groupId);
		/* 我不知道这个是做什么的 */
		void DeleteSharedIPGroup(RakNet::RakString groupId);

		/*
		 * Adds a callback to the list of callbacks to be called when any of the above functions finish executing
		 * The callbacks are called in the order they are added
		 */
		void AddEventCallback(Rackspace2EventCallback *callback);
		/*
		 * Removes a callback from the list of callbacks to be called when any of the above functions finish executing
		 * The callbacks are called in the order they are added
		 */
		void RemoveEventCallback(Rackspace2EventCallback *callback);
		/* 移除 all callbacks */
		void ClearEventCallbacks();

		/* Call this anytime TCPInterface returns a packet */
		void OnReceive(Packet *packet);

		/* Call this when TCPInterface returns something other than UNASSIGNED_SYSTEM_ADDRESS from HasLostConnection() */
		void OnClosedConnection(SystemAddress systemAddress);

		/* String representation of each RackspaceEventType */
		static const char * EventTypeToString(RackspaceEventType eventType);

		/*
		 * Mostly for internal use, but you can use it to execute an operation with more complex xml if desired
		 * 参见 the Rackspace.cpp on how to use it
		 */
		void AddOperation(RackspaceOperationType type, RakNet::RakString httpCommand, RakNet::RakString operation, RakNet::RakString xml);
	protected:

		DataStructures::List<Rackspace2EventCallback*> eventCallbacks;

		struct RackspaceOperation
		{
			RackspaceOperationType type;
		/* RakNet::RakString stringInfo; */
			SystemAddress connectionAddress;
			bool isPendingAuthentication;
			RakNet::RakString incomingStream;
			RakNet::RakString httpCommand;
			RakNet::RakString operation;
			RakNet::RakString xml;
		};

		TCPInterface *tcpInterface;

		/* RackspaceOperationType currentOperation; */
		/* DataStructures::Queue<RackspaceOperation> nextOperationQueue; */

		DataStructures::List<RackspaceOperation> operations;
		bool HasOperationOfType(RackspaceOperationType t);
		unsigned int GetOperationOfTypeIndex(RackspaceOperationType t);

		RakNet::RakString serverManagementURL;
		RakNet::RakString serverManagementDomain;
		RakNet::RakString serverManagementPath;
		RakNet::RakString storageURL;
		RakNet::RakString storageDomain;
		RakNet::RakString storagePath;
		RakNet::RakString cdnManagementURL;
		RakNet::RakString cdnManagementDomain;
		RakNet::RakString cdnManagementPath;

		RakNet::RakString storageToken;
		RakNet::RakString authToken;
		RakNet::RakString rackspaceCloudUsername;
		RakNet::RakString apiAccessKey;

		bool ExecuteOperation(RackspaceOperation &ro);
		void ReadLine(const char *data, const char *stringStart, RakNet::RakString &output);
		bool ConnectToServerManagementDomain(RackspaceOperation &ro);


	};

} /* RakNet 命名空间 */

#endif /* __RACKSPACE_API_H */
