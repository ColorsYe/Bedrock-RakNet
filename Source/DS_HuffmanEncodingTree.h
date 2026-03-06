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
 *  DS_HuffmanEncodingTree.h
 * \b [内部使用] Generates a huffman encoding tree, used for string and global compression.
 *
 */


#pragma once
#include "RakMemoryOverride.h"
#include "DS_HuffmanEncodingTreeNode.h"
#include "BitStream.h"
#include "Export.h"
#include "DS_LinkedList.h" 

namespace RakNet
{

/* This generates special cases of the huffman encoding tree using 8 bit keys with the additional condition that unused combinations of 8 bits are treated as a frequency of 1 */
class RAK_DLL_EXPORT HuffmanEncodingTree
{

public:
	HuffmanEncodingTree();
	~HuffmanEncodingTree() noexcept;

	/*
	 * Pass an array of bytes to array and a preallocated BitStream to receive the output.
	 * 参数 [in] input Array of bytes to encode
	 * 参数 [in] sizeInBytes size of input
	 * 参数 [out] output The bitstream to write to
	 */
	void EncodeArray( unsigned char *input, size_t sizeInBytes, RakNet::BitStream * output );

	/* Decodes an array encoded by EncodeArray(). */
	unsigned DecodeArray( RakNet::BitStream * input, BitSize_t sizeInBits, size_t maxCharsToWrite, unsigned char *output );
	void DecodeArray( unsigned char *input, BitSize_t sizeInBits, RakNet::BitStream * output );

	/* Given a frequency table of 256 elements, all with a frequency of 1 or more, generate the tree. */
	void GenerateFromFrequencyTable( unsigned int frequencyTable[ 256 ] );

	/* Free the memory used by the tree. */
	void FreeMemory( void );

private:

	/* The root node of the tree */

	HuffmanEncodingTreeNode *root;

	/* 用于hold bit encoding for one character */


	struct CharacterEncoding
	{
		unsigned char* encoding;
		unsigned short bitLength;
	};

	CharacterEncoding encodingTable[ 256 ];

	void InsertNodeIntoSortedList( HuffmanEncodingTreeNode * node, DataStructures::LinkedList<HuffmanEncodingTreeNode *> *huffmanEncodingTreeNodeList ) const;
};

} /* RakNet 命名空间 */
