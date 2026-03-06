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
 * \b Compresses/Decompresses ASCII strings and writes/reads them to BitStream class instances.  You can use this to easily serialize and deserialize your own strings.
 *
 */



#pragma once
#include "Export.h"
#include "DS_Map.h"
#include "RakMemoryOverride.h"
#include "NativeTypes.h"

#ifdef _STD_STRING_COMPRESSOR
#include <string>
#endif

/* 前向声明 */
namespace RakNet
{
	class BitStream;
	class RakString;
};


namespace RakNet
{
/* 前向声明 */
class HuffmanEncodingTree;

/*
 * Writes and reads strings to and from bitstreams.
 *
 * Only works with ASCII strings.  The default compression is for English.
 * You can call GenerateTreeFromStrings to compress and decompress other languages efficiently as well.
 */
class RAK_DLL_EXPORT StringCompressor
{
public:
	
	/* 析构函数 */
	~StringCompressor() noexcept;
	
	/*
	 * static function because only static functions can access static members
	 * The RakPeer 构造函数 adds a reference to this class, so don't call this until an instance of RakPeer exists, or unless you call AddReference yourself.
	 * 返回值: the unique instance of the StringCompressor
	 */
	static StringCompressor* Instance();

	/*
	 * Given an array of strings, such as a chat log, generate the optimal encoding tree for it.
	 * This function is 可选 and if it is not called a default tree will be used instead.
	 * 参数[输入] input An array of bytes which should point to text.
	 * 参数[输入] inputLength Length of input
	 * 参数[输入] languageID An identifier for the language / string table to generate the tree for.  English is automatically created with ID 0 in the 构造函数.
	 */
	void GenerateTreeFromStrings( unsigned char *input, unsigned inputLength, uint8_t languageId );
	
 	/*
 	 * Writes input to output, compressed.  Takes care of the null terminator for you.
 	 * 参数[输入] input Pointer to an ASCII string
 	 * 参数[输入] maxCharsToWrite The max number of bytes to write of input.  Use 0 to mean no limit.
 	 * 参数[输出] output The bitstream to write the compressed string to
 	 * 参数[输入] languageID Which language to use
 	 */
	void EncodeString( const char *input, int maxCharsToWrite, RakNet::BitStream *output, uint8_t languageId=0 );
	
	/*
	 * Writes input to output, uncompressed.  Takes care of the null terminator for you.
	 * 参数[输出] output A block of bytes to receive the output
	 * 参数[输入] maxCharsToWrite Size, in bytes, of output .  A nullptr terminator will always be appended to the output string.  If the maxCharsToWrite is not large enough, the string will be truncated.
	 * 参数[输入] input The bitstream containing the compressed string
	 * 参数[输入] languageID Which language to use
	 */
	bool DecodeString( char *output, int maxCharsToWrite, RakNet::BitStream *input, uint8_t languageId=0 );

#ifdef _CSTRING_COMPRESSOR
	void EncodeString( const CString &input, int maxCharsToWrite, RakNet::BitStream *output, uint8_t languageId=0 );
	bool DecodeString( CString &output, int maxCharsToWrite, RakNet::BitStream *input, uint8_t languageId=0 );
#endif

#ifdef _STD_STRING_COMPRESSOR
	void EncodeString( const std::string &input, int maxCharsToWrite, RakNet::BitStream *output, uint8_t languageId=0 );
	bool DecodeString( std::string *output, int maxCharsToWrite, RakNet::BitStream *input, uint8_t languageId=0 );
#endif

	void EncodeString( const RakNet::RakString *input, int maxCharsToWrite, RakNet::BitStream *output, uint8_t languageId=0 );
	bool DecodeString( RakNet::RakString *output, int maxCharsToWrite, RakNet::BitStream *input, uint8_t languageId=0 );

	/* 用于在运行时分配和释放此单例 */
	static void AddReference();
	
	/* 用于在运行时分配和释放此单例 */
	static void RemoveReference();

	StringCompressor();

private:
	
	/* 单例实例 */
	static StringCompressor *instance;
	
	/* Pointer to the huffman encoding trees. */
	DataStructures::Map<int, HuffmanEncodingTree *> huffmanEncodingTrees;
	
	static int referenceCount;
};

} /* RakNet 命名空间 */
