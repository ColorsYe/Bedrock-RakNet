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
 *  DS_Tree.h
 * 内部使用
 * Just a regular tree
 *
 */



#pragma once
#include "Export.h"
#include "DS_List.h"
#include "DS_Queue.h"
#include "RakMemoryOverride.h"

/*
 * DataStructures 命名空间的添加仅是为了避免常见数据结构名称导致的编译器错误
 * 由于这些数据结构是独立的，如果需要，你可以在 RakNet 之外将它们用于自己的项目。
 */
namespace DataStructures
{
	template <class TreeType>
	class RAK_DLL_EXPORT Tree
	{
	public:
		Tree();
		Tree(TreeType &inputData);
		~Tree() noexcept;
		void LevelOrderTraversal(DataStructures::List<Tree*> &output);
		void AddChild(TreeType &newData);
		void DeleteDecendants();

		TreeType data;
		DataStructures::List<Tree *> children;
	};

	template <class TreeType>
	Tree<TreeType>::Tree()
	{

	}

	template <class TreeType>
	Tree<TreeType>::Tree(TreeType &inputData)
	{
		data=inputData;
	}

	template <class TreeType>
	Tree<TreeType>::~Tree() noexcept
	{
		DeleteDecendants();
	}

	template <class TreeType>
	void Tree<TreeType>::LevelOrderTraversal(DataStructures::List<Tree*> &output)
	{
		unsigned i;
		Tree<TreeType> *node;
		DataStructures::Queue<Tree<TreeType>*> queue;

		for (i=0; i < children.Size(); i++)
			queue.Push(children[i]);

		while (queue.Size())
		{
			node=queue.Pop();
			output.Insert(node, _FILE_AND_LINE_);
			for (i=0; i < node->children.Size(); i++)
				queue.Push(node->children[i]);
		}
	}

	template <class TreeType>
	void Tree<TreeType>::AddChild(TreeType &newData)
	{
		children.Insert(RakNet::OP_NEW<Tree>(newData, _FILE_AND_LINE_));
	}

	template <class TreeType>
	void Tree<TreeType>::DeleteDecendants()
	{
		/*
        DataStructures::List<Tree*> output;
		LevelOrderTraversal(output);
		unsigned i;
		for (i=0; i < output.Size(); i++)
			RakNet::OP_DELETE(output[i], _FILE_AND_LINE_);
*/

		/* Already recursive to do this */
		unsigned int i;
		for (i=0; i < children.Size(); i++)
			RakNet::OP_DELETE(children[i], _FILE_AND_LINE_);
	}
}
