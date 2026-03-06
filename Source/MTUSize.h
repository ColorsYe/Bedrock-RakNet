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
 * \b [内部使用] 定义默认最大传输单元（MTU）。
 *
 */


#ifndef MAXIMUM_MTU_SIZE

/*
 * \li \em 17914 16 Mbit/秒 令牌环
 * \li \em 4464 4 Mbit/秒 令牌环
 * \li \em 4352 FDDI
 * \li \em 1500. 最大以太网数据包大小（\b 推荐值）。这是非 PPPoE、非 VPN 连接的典型设置，也是 NETGEAR 路由器、适配器和交换机的默认值。
 * \li \em 1492. PPPoE 的首选大小。
 * \li \em 1472. 用于 ping 的最大大小（更大的数据包会被分片）。
 * \li \em 1468. DHCP 的首选大小。
 * \li \em 1460. 在没有大型邮件附件等情况下，AOL 可用的大小。
 * \li \em 1430. VPN 和 PPTP 的首选大小。
 * \li \em 1400. AOL DSL 的最大大小。
 * \li \em 576. 连接到拨号 ISP 的典型值。
 * UDP 数据报的最大值
 */



#define MAXIMUM_MTU_SIZE 1492


#define MINIMUM_MTU_SIZE 400

#endif
