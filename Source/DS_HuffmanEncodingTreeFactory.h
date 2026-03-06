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
 *  DS_HuffmanEncodingTreeFactory.h
 * 内部使用
 * Creates instances of the class HuffmanEncodingTree
 *
 */

 
#pragma once
#include "RakMemoryOverride.h"

namespace RakNet {
/* 前向声明 */
class HuffmanEncodingTree;

/*
 * Creates instances of the class HuffmanEncodingTree
 * This class takes a frequency table and given that frequence table, will generate an instance of HuffmanEncodingTree
 */
class HuffmanEncodingTreeFactory
{
public:
	/* 默认构造函数 */
	HuffmanEncodingTreeFactory();
	
	/*
	 * 重置 frequency table
	 * You don't need to call this unless you want to reuse the class for a new tree
	 */
	void Reset( void );
	
	/*
	 * Pass an array of bytes to this to add those elements to the frequency table.
	 * 参数[输入] array the data to insert into the frequency table
	 * 参数[输入] size the size of the data to insert
	 */
	void AddToFrequencyTable( unsigned char *array, int size );
	
	/*
	 * Copies the frequency table to the array passed. Retrieve the frequency table.
	 * 参数[输入] _frequency The frequency table used currently
	 */
	void GetFrequencyTable( unsigned int _frequency[ 256 ] );
	
	/*
	 * 返回 frequency table as a pointer
	 * 返回值: the address of the frenquency table
	 */
	unsigned int * GetFrequencyTable( void );
	
	/*
	 * Generate a HuffmanEncodingTree.
	 * You can also use GetFrequencyTable and GenerateFromFrequencyTable in the tree itself
	 * 返回值: The generated instance of HuffmanEncodingTree
	 */
	HuffmanEncodingTree * GenerateTree( void );
	
private:

	/* Frequency table */
	unsigned int frequency[ 256 ];
};

} /* RakNet 命名空间 */
