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
 *  DS_WeightedGraph.h
 * 内部使用
 * 加权图。
 * 假设索引为复杂的映射类型，而非顺序数字（顺序数字可以更高效地实现）。
 *
 */


#pragma once
#include "DS_OrderedList.h"
#include "DS_Map.h"
#include "DS_Heap.h"
#include "DS_Queue.h"
#include "DS_Tree.h"
#include "RakAssert.h"
#include "RakMemoryOverride.h"
#ifdef _DEBUG
#include <cstdio>
#endif

#ifdef _MSC_VER
#pragma warning( push )
#endif

/*
 * 命名空间 DataStructures 仅为避免常见数据结构名称的编译器错误而添加
 * 由于这些数据结构是独立的，如有需要，你可以在 RakNet 之外的项目中使用它们。
 */
namespace DataStructures
{
	template <class node_type, class weight_type, bool allow_unlinkedNodes>
	class RAK_DLL_EXPORT WeightedGraph
	{
	public:
		static void IMPLEMENT_DEFAULT_COMPARISON() {DataStructures::defaultMapKeyComparison<node_type>(node_type(),node_type());}

		WeightedGraph();
		~WeightedGraph() noexcept;
		WeightedGraph( const WeightedGraph& original_copy );
		WeightedGraph& operator= ( const WeightedGraph& original_copy );
		void AddNode(const node_type &node);
		void RemoveNode(const node_type &node);
		void AddConnection(const node_type &node1, const node_type &node2, weight_type weight);
		void RemoveConnection(const node_type &node1, const node_type &node2);
		bool HasConnection(const node_type &node1, const node_type &node2);
		void Print();
		void Clear();
		bool GetShortestPath(DataStructures::List<node_type> &path, node_type startNode, node_type endNode, weight_type INFINITE_WEIGHT);
		bool GetSpanningTree(DataStructures::Tree<node_type> &outTree, DataStructures::List<node_type> *inputNodes, node_type startNode, weight_type INFINITE_WEIGHT );
		unsigned GetNodeCount(void) const;
		unsigned GetConnectionCount(unsigned nodeIndex) const;
		void GetConnectionAtIndex(unsigned nodeIndex, unsigned connectionIndex, node_type &outNode, weight_type &outWeight) const;
		node_type GetNodeAtIndex(unsigned nodeIndex) const;

	protected:
		void ClearDijkstra();
		void GenerateDisjktraMatrix(node_type startNode, weight_type INFINITE_WEIGHT);

		DataStructures::Map<node_type, DataStructures::Map<node_type, weight_type> *> adjacencyLists;

		/* 以下所有变量用于 Dijkstra 寻路 */
		/* 08/23/06 在此结构体内部作为 DLL 无法编译 */
	/* struct */
	/* { */
			bool isValidPath;
			node_type rootNode;
			DataStructures::OrderedList<node_type, node_type> costMatrixIndices;
			weight_type *costMatrix;
			node_type *leastNodeArray;
	/* } dijkstra; */

		struct NodeAndParent
		{
			DataStructures::Tree<node_type>*node;
			DataStructures::Tree<node_type>*parent;
		};
	};

	template <class node_type, class weight_type, bool allow_unlinkedNodes>
		WeightedGraph<node_type, weight_type, allow_unlinkedNodes>::WeightedGraph()
	{
		isValidPath=false;
		costMatrix=0;
	}

	template <class node_type, class weight_type, bool allow_unlinkedNodes>
		WeightedGraph<node_type, weight_type, allow_unlinkedNodes>::~WeightedGraph() noexcept
	{
		Clear();
	}

	template <class node_type, class weight_type, bool allow_unlinkedNodes>
	WeightedGraph<node_type, weight_type, allow_unlinkedNodes>::WeightedGraph( const WeightedGraph& original_copy )
	{
		adjacencyLists=original_copy.adjacencyLists;

		isValidPath=original_copy.isValidPath;
		if (isValidPath)
		{
			rootNode=original_copy.rootNode;
			costMatrixIndices=original_copy.costMatrixIndices;
			costMatrix = RakNet::OP_NEW_ARRAY<weight_type>(costMatrixIndices.Size() * costMatrixIndices.Size(), _FILE_AND_LINE_ );
			leastNodeArray = RakNet::OP_NEW_ARRAY<node_type>(costMatrixIndices.Size(), _FILE_AND_LINE_ );
			memcpy(costMatrix, original_copy.costMatrix, costMatrixIndices.Size() * costMatrixIndices.Size() * sizeof(weight_type));
			memcpy(leastNodeArray, original_copy.leastNodeArray, costMatrixIndices.Size() * sizeof(weight_type));
		}
	}

	template <class node_type, class weight_type, bool allow_unlinkedNodes>
	WeightedGraph<node_type, weight_type, allow_unlinkedNodes>& WeightedGraph<node_type, weight_type, allow_unlinkedNodes>::operator=( const WeightedGraph& original_copy )
	{
		adjacencyLists=original_copy.adjacencyLists;

		isValidPath=original_copy.isValidPath;
		if (isValidPath)
		{
			rootNode=original_copy.rootNode;
			costMatrixIndices=original_copy.costMatrixIndices;
			costMatrix = RakNet::OP_NEW_ARRAY<weight_type>(costMatrixIndices.Size() * costMatrixIndices.Size(), _FILE_AND_LINE_ );
			leastNodeArray = RakNet::OP_NEW_ARRAY<node_type>(costMatrixIndices.Size(), _FILE_AND_LINE_ );
			memcpy(costMatrix, original_copy.costMatrix, costMatrixIndices.Size() * costMatrixIndices.Size() * sizeof(weight_type));
			memcpy(leastNodeArray, original_copy.leastNodeArray, costMatrixIndices.Size() * sizeof(weight_type));
		}

		return *this;
	}

	template <class node_type, class weight_type, bool allow_unlinkedNodes>
		void WeightedGraph<node_type, weight_type, allow_unlinkedNodes>::AddNode(const node_type &node)
	{
		adjacencyLists.SetNew(node, RakNet::OP_NEW<DataStructures::Map<node_type, weight_type> >( _FILE_AND_LINE_) );
	}

	template <class node_type, class weight_type, bool allow_unlinkedNodes>
		void WeightedGraph<node_type, weight_type, allow_unlinkedNodes>::RemoveNode(const node_type &node)
	{
		unsigned i;
		DataStructures::Queue<node_type> removeNodeQueue;

		removeNodeQueue.Push(node, _FILE_AND_LINE_ );
		while (removeNodeQueue.Size())
		{
			RakNet::OP_DELETE(adjacencyLists.Pop(removeNodeQueue.Pop()), _FILE_AND_LINE_);

			/* 同时从所有其他列表中删除此节点 */
			for (i=0; i < adjacencyLists.Size(); i++)
			{
				adjacencyLists[i]->Delete(node);

#ifdef _MSC_VER
#pragma warning( disable : 4127 ) /* 警告 C4127：条件表达式是常量 */
#endif
				if (allow_unlinkedNodes==false && adjacencyLists[i]->Size()==0)
					removeNodeQueue.Push(adjacencyLists.GetKeyAtIndex(i), _FILE_AND_LINE_ );
			}
		}

		ClearDijkstra();
	}

	template <class node_type, class weight_type, bool allow_unlinkedNodes>
	bool WeightedGraph<node_type, weight_type, allow_unlinkedNodes>::HasConnection(const node_type &node1, const node_type &node2)
	{
		if (node1==node2)
			return false;
		if (adjacencyLists.Has(node1)==false)
			return false;
		return adjacencyLists.Get(node1)->Has(node2);
	}

	template <class node_type, class weight_type, bool allow_unlinkedNodes>
		void WeightedGraph<node_type, weight_type, allow_unlinkedNodes>::AddConnection(const node_type &node1, const node_type &node2, weight_type weight)
	{
		if (node1==node2)
			return;

		if (adjacencyLists.Has(node1)==false)
			AddNode(node1);
		adjacencyLists.Get(node1)->Set(node2, weight);
		if (adjacencyLists.Has(node2)==false)
			AddNode(node2);
		adjacencyLists.Get(node2)->Set(node1, weight);
	}

	template <class node_type, class weight_type, bool allow_unlinkedNodes>
		void WeightedGraph<node_type, weight_type, allow_unlinkedNodes>::RemoveConnection(const node_type &node1, const node_type &node2)
	{
		adjacencyLists.Get(node2)->Delete(node1);
		adjacencyLists.Get(node1)->Delete(node2);

#ifdef _MSC_VER
#pragma warning( disable : 4127 ) /* 警告 C4127：条件表达式是常量 */
#endif
		if (allow_unlinkedNodes==false) /* 若不允许孤立节点，则当没有连接时删除该节点 */
		{
			if (adjacencyLists.Get(node1)->Size()==0)
				RemoveNode(node1); /* 也会从 node2 的邻接列表中删除 node1 */
			if (adjacencyLists.Has(node2) && adjacencyLists.Get(node2)->Size()==0)
				RemoveNode(node2);
		}

		ClearDijkstra();
	}

	template <class node_type, class weight_type, bool allow_unlinkedNodes>
		void WeightedGraph<node_type, weight_type, allow_unlinkedNodes>::Clear()
	{
		unsigned i;
		for (i=0; i < adjacencyLists.Size(); i++)
			RakNet::OP_DELETE(adjacencyLists[i], _FILE_AND_LINE_);
		adjacencyLists.Clear();

		ClearDijkstra();
	}

	template <class node_type, class weight_type, bool allow_unlinkedNodes>
		bool WeightedGraph<node_type, weight_type, allow_unlinkedNodes>::GetShortestPath(DataStructures::List<node_type> &path, node_type startNode, node_type endNode, weight_type INFINITE_WEIGHT)
	{
		path.Clear(false, _FILE_AND_LINE_);
		if (startNode==endNode)
		{
			path.Insert(startNode, _FILE_AND_LINE_);
			path.Insert(endNode, _FILE_AND_LINE_);
			return true;
		}

		if (isValidPath==false || rootNode!=startNode)
		{
			ClearDijkstra();
			GenerateDisjktraMatrix(startNode, INFINITE_WEIGHT);
		}

		/* 返回结果 */
		bool objectExists;
		unsigned col,row;
		weight_type currentWeight;
		DataStructures::Queue<node_type> outputQueue;
		col=costMatrixIndices.GetIndexFromKey(endNode, &objectExists);
		if (costMatrixIndices.Size()<2)
		{
			return false;
		}
		if (objectExists==false)
		{
			return false;
		}
		node_type vertex;
		row=costMatrixIndices.Size()-2;
		if (row==0)
		{
			path.Insert(startNode, _FILE_AND_LINE_);
			path.Insert(endNode, _FILE_AND_LINE_);
			return true;
		}
		currentWeight=costMatrix[row*adjacencyLists.Size() + col];
		if (currentWeight==INFINITE_WEIGHT)
		{
			/* 无路径 */
			return true;
		}
		vertex=endNode;
		outputQueue.PushAtHead(vertex, 0, _FILE_AND_LINE_);
		row--;
#ifdef _MSC_VER
#pragma warning( disable : 4127 ) /* 警告 C4127：条件表达式是常量 */
#endif
		while (1)
		{
			while (costMatrix[row*adjacencyLists.Size() + col] == currentWeight)
			{
				if (row==0)
				{
					path.Insert(startNode, _FILE_AND_LINE_);
					for (col=0; outputQueue.Size(); col++)
						path.Insert(outputQueue.Pop(), _FILE_AND_LINE_);
					return true;
				}
				--row;
			}

			vertex=leastNodeArray[row];
			outputQueue.PushAtHead(vertex, 0, _FILE_AND_LINE_);
			if (row==0)
				break;
			col=costMatrixIndices.GetIndexFromKey(vertex, &objectExists);
			currentWeight=costMatrix[row*adjacencyLists.Size() + col];
		}

		path.Insert(startNode, _FILE_AND_LINE_);
		for (col=0; outputQueue.Size(); col++)
			path.Insert(outputQueue.Pop(), _FILE_AND_LINE_);
		return true;
	}

	template <class node_type, class weight_type, bool allow_unlinkedNodes>
		node_type WeightedGraph<node_type, weight_type, allow_unlinkedNodes>::GetNodeAtIndex(unsigned nodeIndex) const
	{
		return adjacencyLists.GetKeyAtIndex(nodeIndex);
	}

	template <class node_type, class weight_type, bool allow_unlinkedNodes>
	unsigned WeightedGraph<node_type, weight_type, allow_unlinkedNodes>::GetNodeCount(void) const
	{
		return adjacencyLists.Size();
	}

	template <class node_type, class weight_type, bool allow_unlinkedNodes>
	unsigned WeightedGraph<node_type, weight_type, allow_unlinkedNodes>::GetConnectionCount(unsigned nodeIndex) const
	{
        return adjacencyLists[nodeIndex]->Size();
	}

	template <class node_type, class weight_type, bool allow_unlinkedNodes>
	void WeightedGraph<node_type, weight_type, allow_unlinkedNodes>::GetConnectionAtIndex(unsigned nodeIndex, unsigned connectionIndex, node_type &outNode, weight_type &outWeight) const
	{
		outWeight=adjacencyLists[nodeIndex]->operator[](connectionIndex);
		outNode=adjacencyLists[nodeIndex]->GetKeyAtIndex(connectionIndex);
	}

	template <class node_type, class weight_type, bool allow_unlinkedNodes>
		bool WeightedGraph<node_type, weight_type, allow_unlinkedNodes>::GetSpanningTree(DataStructures::Tree<node_type> &outTree, DataStructures::List<node_type> *inputNodes, node_type startNode, weight_type INFINITE_WEIGHT )
	{
		/* 找到从起始节点到每个输入节点的最短路径。若结果可达，则将此路径添加到新的加权图中 */
		DataStructures::List<node_type> path;
		DataStructures::WeightedGraph<node_type, weight_type, allow_unlinkedNodes> outGraph;
		bool res;
		unsigned i,j;
		for (i=0; i < inputNodes->Size(); i++)
		{
			res=GetShortestPath(path, startNode, (*inputNodes)[i], INFINITE_WEIGHT);
			if (res && path.Size()>0)
			{
				for (j=0; j < path.Size()-1; j++)
				{
					/* 不必查找权重 */
 					outGraph.AddConnection(path[j], path[j+1], INFINITE_WEIGHT);
				}
			}
		}

		/* 将图复制为树。 */
		DataStructures::Queue<NodeAndParent> nodesToProcess;
		DataStructures::Tree<node_type> *current;
		DataStructures::Map<node_type, weight_type> *adjacencyList;
		node_type key;
		NodeAndParent nap, nap2;
		outTree.DeleteDecendants();
		outTree.data=startNode;
		current=&outTree;
		if (outGraph.adjacencyLists.Has(startNode)==false)
			return false;
		adjacencyList = outGraph.adjacencyLists.Get(startNode);

		for (i=0; i < adjacencyList->Size(); i++)
		{
			nap2.node=RakNet::OP_NEW<DataStructures::Tree<node_type> >( _FILE_AND_LINE_ );
			nap2.node->data=adjacencyList->GetKeyAtIndex(i);
			nap2.parent=current;
			nodesToProcess.Push(nap2, _FILE_AND_LINE_ );
			current->children.Insert(nap2.node, _FILE_AND_LINE_);
		}

		while (nodesToProcess.Size())
		{
			nap=nodesToProcess.Pop();
			current=nap.node;
			adjacencyList = outGraph.adjacencyLists.Get(nap.node->data);

			for (i=0; i < adjacencyList->Size(); i++)
			{
				key=adjacencyList->GetKeyAtIndex(i);
				if (key!=nap.parent->data)
				{
					nap2.node=RakNet::OP_NEW<DataStructures::Tree<node_type> >( _FILE_AND_LINE_ );
					nap2.node->data=key;
					nap2.parent=current;
					nodesToProcess.Push(nap2, _FILE_AND_LINE_ );
					current->children.Insert(nap2.node, _FILE_AND_LINE_);
				}
			}
		}

		return true;
	}

	template <class node_type, class weight_type, bool allow_unlinkedNodes>
		void WeightedGraph<node_type, weight_type, allow_unlinkedNodes>::GenerateDisjktraMatrix(node_type startNode, weight_type INFINITE_WEIGHT)
	{
		if (adjacencyLists.Size()==0)
			return;

		costMatrix = RakNet::OP_NEW_ARRAY<weight_type>(adjacencyLists.Size() * adjacencyLists.Size(), _FILE_AND_LINE_ );
		leastNodeArray = RakNet::OP_NEW_ARRAY<node_type>(adjacencyLists.Size(), _FILE_AND_LINE_ );

		node_type currentNode;
		unsigned col, row, row2, openSetIndex;
		node_type adjacentKey;
		unsigned adjacentIndex;
		weight_type edgeWeight, currentNodeWeight, adjacentNodeWeight;
		DataStructures::Map<node_type, weight_type> *adjacencyList;
		DataStructures::Heap<weight_type, node_type, false> minHeap;
		DataStructures::Map<node_type, weight_type> openSet;

		for (col=0; col < adjacencyLists.Size(); col++)
		{
			/* 这应该已经排序，因此插入排序有点低效，但无所谓 */
			costMatrixIndices.Insert(adjacencyLists.GetKeyAtIndex(col),adjacencyLists.GetKeyAtIndex(col), true, _FILE_AND_LINE_);
		}
		for (col=0; col < adjacencyLists.Size() * adjacencyLists.Size(); col++)
			costMatrix[col]=INFINITE_WEIGHT;
		currentNode=startNode;
		row=0;
		currentNodeWeight=0;
		rootNode=startNode;

		/* 清除起始节点列 */
		if (adjacencyLists.Size())
		{
			adjacentIndex=adjacencyLists.GetIndexAtKey(startNode);
			for (row2=0; row2 < adjacencyLists.Size(); row2++)
				costMatrix[row2*adjacencyLists.Size() + adjacentIndex]=0;
		}

		while (row < adjacencyLists.Size()-1)
		{
			adjacencyList = adjacencyLists.Get(currentNode);
			/* 遍历当前节点的所有连接。若新权重小于当前权重，则更新该权重。 */
			for (col=0; col < adjacencyList->Size(); col++)
			{
				edgeWeight=(*adjacencyList)[col];
				adjacentKey=adjacencyList->GetKeyAtIndex(col);
				adjacentIndex=adjacencyLists.GetIndexAtKey(adjacentKey);
				adjacentNodeWeight=costMatrix[row*adjacencyLists.Size() + adjacentIndex];

				if (currentNodeWeight + edgeWeight < adjacentNodeWeight)
				{
					/* 更新相邻节点的权重 */
					for (row2=row; row2 < adjacencyLists.Size(); row2++)
						costMatrix[row2*adjacencyLists.Size() + adjacentIndex]=currentNodeWeight + edgeWeight;
					openSet.Set(adjacentKey, currentNodeWeight + edgeWeight);
				}
			}

			/* 在开放集中找到最小值 */
			minHeap.Clear(true,_FILE_AND_LINE_);
			for (openSetIndex=0; openSetIndex < openSet.Size(); openSetIndex++)
				minHeap.Push(openSet[openSetIndex], openSet.GetKeyAtIndex(openSetIndex),_FILE_AND_LINE_);

			/*
			unsigned i,j;
			for (i=0; i < adjacencyLists.Size()-1; i++)
			{
				for (j=0; j < adjacencyLists.Size(); j++)
				{
					RAKNET_DEBUG_PRINTF("%2i ", costMatrix[i*adjacencyLists.Size() + j]);
				}
				RAKNET_DEBUG_PRINTF("Node=%i", leastNodeArray[i]);
				RAKNET_DEBUG_PRINTF("\n");
			}
			*/

			if (minHeap.Size()==0)
			{
				/* 不可达节点 */
				isValidPath=true;
				return;
			}

			currentNodeWeight=minHeap.PeekWeight(0);
			leastNodeArray[row]=minHeap.Pop(0);
			currentNode=leastNodeArray[row];
			openSet.Delete(currentNode);
			row++;
		}

		/*
#ifdef _DEBUG
		unsigned i,j;
		for (i=0; i < adjacencyLists.Size()-1; i++)
		{
			for (j=0; j < adjacencyLists.Size(); j++)
			{
				RAKNET_DEBUG_PRINTF("%2i ", costMatrix[i*adjacencyLists.Size() + j]);
			}
			RAKNET_DEBUG_PRINTF("Node=%i", leastNodeArray[i]);
			RAKNET_DEBUG_PRINTF("\n");
		}
#endif
		*/

		isValidPath=true;
	}

	template <class node_type, class weight_type, bool allow_unlinkedNodes>
		void WeightedGraph<node_type, weight_type, allow_unlinkedNodes>::ClearDijkstra()
	{
		if (isValidPath)
		{
			isValidPath=false;
			RakNet::OP_DELETE_ARRAY(costMatrix, _FILE_AND_LINE_);
			RakNet::OP_DELETE_ARRAY(leastNodeArray, _FILE_AND_LINE_);
			costMatrixIndices.Clear(false, _FILE_AND_LINE_);
		}
	}

	template <class node_type, class weight_type, bool allow_unlinkedNodes>
		void WeightedGraph<node_type, weight_type, allow_unlinkedNodes>::Print()
	{
#ifdef _DEBUG
		unsigned i,j;
		for (i=0; i < adjacencyLists.Size(); i++)
		{
			/* RAKNET_DEBUG_PRINTF("%i connected to ", i); */
			RAKNET_DEBUG_PRINTF("%s connected to ", adjacencyLists.GetKeyAtIndex(i).systemAddress.ToString());

			if (adjacencyLists[i]->Size()==0)
				RAKNET_DEBUG_PRINTF("<Empty>");
			else
			{
				for (j=0; j < adjacencyLists[i]->Size(); j++)
				/* RAKNET_DEBUG_PRINTF("%i (%.2f) ", adjacencyLists.GetIndexAtKey(adjacencyLists[i]->GetKeyAtIndex(j)), (float) adjacencyLists[i]->operator[](j) ); */
					RAKNET_DEBUG_PRINTF("%s (%.2f) ", adjacencyLists[i]->GetKeyAtIndex(j).systemAddress.ToString(), (float) adjacencyLists[i]->operator[](j) );
			}

			RAKNET_DEBUG_PRINTF("\n");
		}
#endif
	}
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif
