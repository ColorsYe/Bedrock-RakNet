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
 *  DS_QueueLinkedList.h
 * 内部使用
 * A queue implemented as a linked list.
 *
 */


#pragma once
#include "DS_LinkedList.h" 
#include "Export.h"
#include "RakMemoryOverride.h"

/*
 * DataStructures 命名空间的添加仅是为了避免常见数据结构名称导致的编译器错误
 * 由于这些数据结构是独立的，如果需要，你可以在 RakNet 之外将它们用于自己的项目。
 */
namespace DataStructures
{
	/* 使用链表实现的队列。很少使用。*/
	template <class QueueType>
	class RAK_DLL_EXPORT QueueLinkedList
	{
	
	public:
		QueueLinkedList();
		QueueLinkedList( const QueueLinkedList& original_copy );
		bool operator= ( const QueueLinkedList& original_copy );
		QueueType Pop( void );
		QueueType& Peek( void );
		QueueType& EndPeek( void );
		void Push( const QueueType& input );
		unsigned int Size( void );
		void Clear( void );
		void Compress( void );
		
	private:
		LinkedList<QueueType> data;
	};
	
	template <class QueueType>
	QueueLinkedList<QueueType>::QueueLinkedList()
	{
	}
	
	template <class QueueType>
	inline unsigned int QueueLinkedList<QueueType>::Size()
	{
		return data.Size();
	}
	
	template <class QueueType>
	inline QueueType QueueLinkedList<QueueType>::Pop( void )
	{
		data.Beginning();
		return ( QueueType ) data.Pop();
	}
	
	template <class QueueType>
	inline QueueType& QueueLinkedList<QueueType>::Peek( void )
	{
		data.Beginning();
		return ( QueueType ) data.Peek();
	}
	
	template <class QueueType>
	inline QueueType& QueueLinkedList<QueueType>::EndPeek( void )
	{
		data.End();
		return ( QueueType ) data.Peek();
	}
	
	template <class QueueType>
	void QueueLinkedList<QueueType>::Push( const QueueType& input )
	{
		data.End();
		data.Add( input );
	}
	
	template <class QueueType>
	QueueLinkedList<QueueType>::QueueLinkedList( const QueueLinkedList& original_copy )
	{
		data = original_copy.data;
	}
	
	template <class QueueType>
	bool QueueLinkedList<QueueType>::operator= ( const QueueLinkedList& original_copy )
	{
		if ( ( &original_copy ) == this )
			return false;
			
		data = original_copy.data;
	}
	
	template <class QueueType>
	void QueueLinkedList<QueueType>::Clear ( void )
	{
		data.Clear();
	}
} /* 命名空间结束 */
