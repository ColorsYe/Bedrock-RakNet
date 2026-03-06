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
 * 文件: BitStream.h
 * 此类允许将原生类型以位串形式进行读写。
 * BitStream 在 RakNet 中被广泛使用，同时也面向用户开放使用。
 *
 */


#if defined(_MSC_VER) && _MSC_VER < 1299 /* VC6 不支持模板特化 */
#include "BitStream_NoTemplate.h"
#else

#pragma once
#include "RakMemoryOverride.h"
#include "RakNetDefines.h"
#include "Export.h"
#include "RakNetTypes.h"
#include "RakString.h"
#include "RakWString.h"
#include "RakAssert.h"
#include <cmath>
#include <cfloat>

#ifdef _MSC_VER
#pragma warning( push )
#endif

/* MSWin 使用 _copysign，其他平台使用 copysign...*/
#ifndef _WIN32
#define _copysign copysign
#endif

namespace RakNet
{
	/*
	 * 此类允许将原生类型以位串形式进行读写。BitStream 在 RakNet 中被广泛使用，同时也面向用户开放使用。
	 * 另见 BitStreamSample.txt
	 */
	class RAK_DLL_EXPORT BitStream
	{

	public:
		/* 获取单例 GetInstance() 和销毁单例 DestroyInstance(instance*) */
		STATIC_FACTORY_DECLARATIONS(BitStream)

		/* 默认构造函数 */
		BitStream();

		/*
		 * 创建位流，立即分配指定数量的字节。
		 * 仅当明确知道所需字节数且大于 BITSTREAM_STACK_ALLOCATION_SIZE 时才有必要调用，否则调用此构造函数没有额外收益。
		 * 在该情况下，此函数的作用是减少一次或多次 realloc 调用。
		 * 参数[输入] initialBytesToAllocate 预分配的字节数。
		 */
		BitStream( const unsigned int initialBytesToAllocate );

		/*
		 * 初始化 BitStream，立即将其包含的数据设置为一个预定义的指针。
		 * Set _copyData to true if you want to make an internal copy of the data you are passing. Set it to false to just save a pointer to the data.
		 * 在 _copyData 为 false 时不应调用 Write 函数，因为这会写入未分配的内存
		 * 99% 的情况下，此函数用于将 Packet::data 转换为位流进行读取, in which case you should write something as follows:
		 * 
		 * RakNet::BitStream bs(packet->data, packet->length, false);
		 * 
		 * 参数[输入] _data 字节数组。
		 * 参数[输入] lengthInBytes Size of the _data.
		 * 参数[输入] _copyData true or false to make a copy of _data or not.
		 */
		BitStream( unsigned char* _data, const unsigned int lengthInBytes, bool _copyData );

		/* 析构函数 */
		~BitStream() noexcept;

		/* 重置位流以便复用。 */
		void Reset( void );

		/*
		 * 对任意整型数据进行双向序列化/反序列化（位流读写）。
		 * 若需要字节序转换，请取消定义 __BITSTREAM_NATIVE_END。
		 * 参数[输入] writeToBitstream true 表示将数据写入位流；false 表示从位流读取数据写入变量
		 * 参数[输入] inOutTemplateVar 要写入的值
		 * 返回值: true if writeToBitstream is true.  true if writeToBitstream is false and the read was successful.  false if writeToBitstream is false and the read was not successful.
		 */
		template <class templateType>
			bool Serialize(bool writeToBitstream, templateType &inOutTemplateVar);

		/*
		 * 对任意整型数据进行双向序列化/反序列化（位流读写）。
		 * 若当前值与上一次的值不同
		 * 则写入当前值；否则仅写入一个比特
		 * 参数[输入] writeToBitstream true 表示将数据写入位流；false 表示从位流读取数据写入变量
		 * 参数[输入] inOutCurrentValue 要写入的当前值
		 * 参数[输入] lastValue The last value to compare against.  Only used if writeToBitstream is true.
		 * 返回值: true if writeToBitstream is true.  true if writeToBitstream is false and the read was successful.  false if writeToBitstream is false and the read was not successful.
		 */
		template <class templateType>
			bool SerializeDelta(bool writeToBitstream, templateType &inOutCurrentValue, const templateType &lastValue);

		/*
		 * 当不知道上一次值或没有上一次值时使用的 SerializeDelta 双向版本。
		 * 参数[输入] writeToBitstream true 表示将数据写入位流；false 表示从位流读取数据写入变量
		 * 参数[输入] inOutCurrentValue 要写入的当前值
		 * 返回值: true if writeToBitstream is true.  true if writeToBitstream is false and the read was successful.  false if writeToBitstream is false and the read was not successful.
		 */
		template <class templateType>
			bool SerializeDelta(bool writeToBitstream, templateType &inOutCurrentValue);

		/*
		 * 对任意整型数据进行双向序列化/反序列化（位流读写）。
		 * 若需要字节序转换，请取消定义 __BITSTREAM_NATIVE_END。
		 * 如果未使用 __BITSTREAM_NATIVE_END，则对于大于 1 字节的类型，情况相反
		 * 对于浮点数，这是有损的，float 使用 2 字节，double 使用 4 字节。范围必须在 -1 到 +1 之间。
		 * 对于非浮点数，这是无损的，但仅在使用不到该类型一半位数时才有收益
		 * 参数[输入] writeToBitstream true 表示将数据写入位流；false 表示从位流读取数据写入变量
		 * 参数[输入] inOutTemplateVar 要写入的值
		 * 返回值: true if writeToBitstream is true.  true if writeToBitstream is false and the read was successful.  false if writeToBitstream is false and the read was not successful.
		 */
		template <class templateType>
			bool SerializeCompressed(bool writeToBitstream, templateType &inOutTemplateVar);

		/*
		 * 对任意整型数据进行双向序列化/反序列化（位流读写）。
		 * 若当前值与上一次的值不同
		 * 则写入当前值；否则仅写入一个比特
		 * 对于浮点数，这是有损的，float 使用 2 字节，double 使用 4 字节。范围必须在 -1 到 +1 之间。
		 * 对于非浮点数，这是无损的，但仅在使用不到该类型一半位数时才有收益
		 * 如果未使用 __BITSTREAM_NATIVE_END，则对于大于 1 字节的类型，情况相反
		 * 参数[输入] writeToBitstream true 表示将数据写入位流；false 表示从位流读取数据写入变量
		 * 参数[输入] inOutCurrentValue 要写入的当前值
		 * 参数[输入] lastValue The last value to compare against.  Only used if writeToBitstream is true.
		 * 返回值: true if writeToBitstream is true.  true if writeToBitstream is false and the read was successful.  false if writeToBitstream is false and the read was not successful.
		 */
		template <class templateType>
			bool SerializeCompressedDelta(bool writeToBitstream, templateType &inOutCurrentValue, const templateType &lastValue);

		/*
		 * Save as SerializeCompressedDelta(templateType &currentValue, const templateType &lastValue) when we have an unknown second parameter
		 * 返回值: true on data read. False on insufficient data in bitstream
		 */
		template <class templateType>
			bool SerializeCompressedDelta(bool writeToBitstream, templateType &inOutTemplateVar);

		/*
		 * Bidirectional serialize/deserialize an array or casted stream or raw data.  This does NOT do endian swapping.
		 * 参数[输入] writeToBitstream true 表示将数据写入位流；false 表示从位流读取数据写入变量
		 * 参数[输入] inOutByteArray a byte buffer
		 * 参数[输入] numberOfBytes the size of input in bytes
		 * 返回值: true if writeToBitstream is true.  true if writeToBitstream is false and the read was successful.  false if writeToBitstream is false and the read was not successful.
		 */
		bool Serialize(bool writeToBitstream,  char* inOutByteArray, const unsigned int numberOfBytes );

		/*
		 * Serialize a float into 2 bytes, spanning the range between floatMin and floatMax
		 * 参数[输入] writeToBitstream true 表示将数据写入位流；false 表示从位流读取数据写入变量
		 * 参数[输入] inOutFloat The float to write
		 * 参数[输入] floatMin Predetermined minimum value of f
		 * 参数[输入] floatMax Predetermined maximum value of f
		 */
		bool SerializeFloat16(bool writeToBitstream, float &inOutFloat, float floatMin, float floatMax);

		/*
		 * Serialize one type casted to another (smaller) type, to save bandwidth
		 * serializationType 应为 uint8_t、uint16_t、uint24_t 或 uint32_t
		 * Example: int num=53; SerializeCasted<uint8_t>(true, num); would use 1 byte to write what would otherwise be an integer (4 or 8 bytes)
		 * 参数[输入] writeToBitstream true 表示将数据写入位流；false 表示从位流读取数据写入变量
		 * 参数[输入] value The value to serialize
		 */
		template <class serializationType, class sourceType >
		bool SerializeCasted( bool writeToBitstream, sourceType &value );

		/*
		 * 给定整数类型的最小值和最大值，计算表示该范围所需的最少位数
		 * Then serialize only those bits
		 * 注意: A static is used so that the required number of bits for (maximum-minimum) is only calculated once. This does require that minimum and \maximum are fixed values for a given line of code for the life of the program
		 * 参数[输入] writeToBitstream true 表示将数据写入位流；false 表示从位流读取数据写入变量
		 * 参数[输入] value Integer value to write, which should be between minimum and maximum
		 * 参数[输入] minimum Minimum value of value
		 * 参数[输入] maximum Maximum value of value
		 * 参数[输入] allowOutsideRange If true, all sends will take an extra bit, however value can deviate from outside minimum and maximum. If false, will assert if the value deviates
		 */
		template <class templateType>
		bool SerializeBitsFromIntegerRange( bool writeToBitstream, templateType &value, const templateType minimum, const templateType maximum, bool allowOutsideRange=false );
		/* 参数[输入] requiredBits Primarily for internal use, called from above function() after calculating number of bits needed to represent maximum-minimum */
		template <class templateType>
		bool SerializeBitsFromIntegerRange( bool writeToBitstream, templateType &value, const templateType minimum, const templateType maximum, const int requiredBits, bool allowOutsideRange=false );

		/*
		 * Bidirectional serialize/deserialize a normalized 3D vector, using (at most) 4 bytes + 3 bits instead of 12-24 bytes.
		 * 将进一步压缩沿 y 或 z 轴对齐的向量。
		 * 精度为 1/32767.5。
		 * 参数[输入] writeToBitstream true 表示将数据写入位流；false 表示从位流读取数据写入变量
		 * 参数[输入] x x
		 * 参数[输入] y y
		 * 参数[输入] z z
		 * 返回值: true if writeToBitstream is true.  true if writeToBitstream is false and the read was successful.  false if writeToBitstream is false and the read was not successful.
		 */
		template <class templateType> /* 此函数的 templateType 必须为 float 或 double 类型 */
			bool SerializeNormVector(bool writeToBitstream,  templateType &x, templateType &y, templateType &z );

		/*
		 * Bidirectional serialize/deserialize a vector, using 10 bytes instead of 12.
		 * Loses accuracy to about 3/10ths and only saves 2 bytes, so only use if accuracy is not important.
		 * 参数[输入] writeToBitstream true 表示将数据写入位流；false 表示从位流读取数据写入变量
		 * 参数[输入] x x
		 * 参数[输入] y y
		 * 参数[输入] z z
		 * 返回值: true if writeToBitstream is true.  true if writeToBitstream is false and the read was successful.  false if writeToBitstream is false and the read was not successful.
		 */
		template <class templateType> /* 此函数的 templateType 必须为 float 或 double 类型 */
			bool SerializeVector(bool writeToBitstream,  templateType &x, templateType &y, templateType &z );

		/*
		 * Bidirectional serialize/deserialize a normalized quaternion in 6 bytes + 4 bits instead of 16 bytes. Slightly lossy.
		 * 参数[输入] writeToBitstream true 表示将数据写入位流；false 表示从位流读取数据写入变量
		 * 参数[输入] w w
		 * 参数[输入] x x
		 * 参数[输入] y y
		 * 参数[输入] z z
		 * 返回值: true if writeToBitstream is true.  true if writeToBitstream is false and the read was successful.  false if writeToBitstream is false and the read was not successful.
		 */
		template <class templateType> /* 此函数的 templateType 必须为 float 或 double 类型 */
			bool SerializeNormQuat(bool writeToBitstream,  templateType &w, templateType &x, templateType &y, templateType &z);

		/*
		 * Bidirectional serialize/deserialize an orthogonal matrix by creating a quaternion, and writing 3 components of the quaternion in 2 bytes each.
		 * 使用 6 字节代替 36 字节
		 * 虽然有损，但结果会被重新归一化
		 * 返回值: 成功返回 true，失败返回 false。
		 */
		template <class templateType> /* 此函数的 templateType 必须为 float 或 double 类型 */
			bool SerializeOrthMatrix(
			bool writeToBitstream,
			templateType &m00, templateType &m01, templateType &m02,
			templateType &m10, templateType &m11, templateType &m12,
			templateType &m20, templateType &m21, templateType &m22 );

		/*
		 * Bidirectional serialize/deserialize numberToSerialize bits to/from the input.
		 * 右对齐数据表示在不完整字节的情况下，位从右侧（位 0）对齐
		 * 而不是从左侧对齐（如常规
		 * 内部使用 representation) You would set this to true when
		 * 写入用户数据时），复制位流数据时应为 false，例如
		 * as writing one bitstream to another
		 * 参数[输入] writeToBitstream true 表示将数据写入位流；false 表示从位流读取数据写入变量
		 * 参数[输入] inOutByteArray The data
		 * 参数[输入] numberOfBitsToSerialize The number of bits to write
		 * 参数[输入] rightAlignedBits if true data will be right aligned
		 * 返回值: true if writeToBitstream is true.  true if writeToBitstream is false and the read was successful.  false if writeToBitstream is false and the read was not successful.
		 */
		bool SerializeBits(bool writeToBitstream, unsigned char* inOutByteArray, const BitSize_t numberOfBitsToSerialize, const bool rightAlignedBits = true );

		/*
		 * 将任意整数类型写入位流。
		 * 若需要字节序转换，请取消定义 __BITSTREAM_NATIVE_END。
		 * 参数[输入] inTemplateVar 要写入的值
		 */
		template <class templateType>
			void Write(const templateType &inTemplateVar);

		/*
		 * Write the dereferenced pointer to any integral type to a bitstream.
		 * 若需要字节序转换，请取消定义 __BITSTREAM_NATIVE_END。
		 * 参数[输入] inTemplateVar 要写入的值
		 */
		template <class templateType>
			void WritePtr(templateType *inTemplateVar);

		/*
		 * 将任意整数类型写入位流。
		 * 若当前值与上一次的值不同
		 * 则写入当前值；否则仅写入一个比特
		 * 参数[输入] currentValue 要写入的当前值
		 * 参数[输入] lastValue The last value to compare against
		 */
		template <class templateType>
			void WriteDelta(const templateType &currentValue, const templateType &lastValue);

		/*
		 * 当你不知道上一次的值是什么或没有上一次的值时，使用 WriteDelta。
		 * 参数[输入] currentValue 要写入的当前值
		 */
		template <class templateType>
			void WriteDelta(const templateType &currentValue);

		/*
		 * 将任意整数类型写入位流。
		 * 若需要字节序转换，请取消定义 __BITSTREAM_NATIVE_END。
		 * 如果未使用 __BITSTREAM_NATIVE_END，则对于大于 1 字节的类型，情况相反
		 * 对于浮点数，这是有损的，float 使用 2 字节，double 使用 4 字节。范围必须在 -1 到 +1 之间。
		 * 对于非浮点数，这是无损的，但仅在使用不到该类型一半位数时才有收益
		 * 参数[输入] inTemplateVar 要写入的值
		 */
		template <class templateType>
			void WriteCompressed(const templateType &inTemplateVar);

		/*
		 * 将任意整数类型写入位流。
		 * 若当前值与上一次的值不同
		 * 则写入当前值；否则仅写入一个比特
		 * 对于浮点数，这是有损的，float 使用 2 字节，double 使用 4 字节。范围必须在 -1 到 +1 之间。
		 * 对于非浮点数，这是无损的，但仅在使用不到该类型一半位数时才有收益
		 * 如果未使用 __BITSTREAM_NATIVE_END，则对于大于 1 字节的类型，情况相反
		 * 参数[输入] currentValue 要写入的当前值
		 * 参数[输入] lastValue The last value to compare against
		 */
		template <class templateType>
			void WriteCompressedDelta(const templateType &currentValue, const templateType &lastValue);

		/* 当第二个参数未知时，与 WriteCompressedDelta(const templateType &currentValue, const templateType &lastValue) 功能相同 */
		template <class templateType>
			void WriteCompressedDelta(const templateType &currentValue);

		/*
		 * 从位流中读取任意整数类型。
		 * Define __BITSTREAM_NATIVE_END if you need endian swapping.
		 * 参数[输入] outTemplateVar The value to read
		 * 返回值: 成功返回 true，失败返回 false。
		 */
		template <class templateType>
			bool Read(templateType &outTemplateVar);

		/*
		 * 从位流中读取任意整数类型。
		 * 如果写入的值与写入函数中用于比较的值不同，
		 * var 将被更新。否则将保留当前值。
		 * ReadDelta 仅在之前调用过 WriteDelta 后才有效
		 * 参数[输入] outTemplateVar The value to read
		 * 返回值: 成功返回 true，失败返回 false。
		 */
		template <class templateType>
			bool ReadDelta(templateType &outTemplateVar);

		/*
		 * 从位流中读取任意整数类型。
		 * 若需要字节序转换，请取消定义 __BITSTREAM_NATIVE_END。
		 * 对于浮点数，这是有损的，float 使用 2 字节，double 使用 4 字节。范围必须在 -1 到 +1 之间。
		 * 对于非浮点数，这是无损的，但仅在使用不到该类型一半位数时才有收益
		 * 如果未使用 __BITSTREAM_NATIVE_END，则对于大于 1 字节的类型，情况相反
		 * 参数[输入] outTemplateVar The value to read
		 * 返回值: 成功返回 true，失败返回 false。
		 */
		template <class templateType>
			bool ReadCompressed(templateType &outTemplateVar);

		/*
		 * 从位流中读取任意整数类型。
		 * 如果写入的值与写入函数中用于比较的值不同，
		 * var 将被更新。否则将保留当前值。
		 * 当前值将被更新。
		 * 对于浮点数，这是有损的，float 使用 2 字节，double 使用 4 字节。范围必须在 -1 到 +1 之间。
		 * 对于非浮点数，这是无损的，但仅在使用不到该类型一半位数时才有收益
		 * 如果未使用 __BITSTREAM_NATIVE_END，则对于大于 1 字节的类型，情况相反
		 * ReadCompressedDelta 仅在之前调用过 WriteDelta 后才有效
		 * 参数[输入] outTemplateVar The value to read
		 * 返回值: 成功返回 true，失败返回 false。
		 */
		template <class templateType>
			bool ReadCompressedDelta(templateType &outTemplateVar);

		/*
		 * Read one bitstream to another.
		 * 参数[输入] numberOfBits bits to read
		 * 参数 bitStream the bitstream to read into from
		 * 返回值: 成功返回 true，失败返回 false。
		 */
		bool Read( BitStream *bitStream, BitSize_t numberOfBits );
		bool Read( BitStream *bitStream );
		bool Read( BitStream &bitStream, BitSize_t numberOfBits );
		bool Read( BitStream &bitStream );

		/*
		 * Write an array or casted stream or raw data.  This does NOT do endian swapping.
		 * 参数[输入] inputByteArray a byte buffer
		 * 参数[输入] numberOfBytes the size of input in bytes
		 */
		void Write( const char* inputByteArray, const unsigned int numberOfBytes );

		/*
		 * Write one bitstream to another.
		 * 参数[输入] numberOfBits bits to write
		 * 参数 bitStream the bitstream to copy from
		 */
		void Write( BitStream *bitStream, BitSize_t numberOfBits );
		void Write( BitStream *bitStream );
		void Write( BitStream &bitStream, BitSize_t numberOfBits );
		void Write( BitStream &bitStream );\
		
		/*
		 * Write a float into 2 bytes, spanning the range between floatMin and floatMax
		 * 参数[输入] x The float to write
		 * 参数[输入] floatMin Predetermined minimum value of f
		 * 参数[输入] floatMax Predetermined maximum value of f
		 */
		void WriteFloat16( float x, float floatMin, float floatMax );

		/*
		 * Write one type serialized as another (smaller) type, to save bandwidth
		 * serializationType 应为 uint8_t、uint16_t、uint24_t 或 uint32_t
		 * Example: int num=53; WriteCasted<uint8_t>(num); would use 1 byte to write what would otherwise be an integer (4 or 8 bytes)
		 * 参数[输入] value 要写入的值
		 */
		template <class serializationType, class sourceType >
		void WriteCasted( const sourceType &value );

		/*
		 * 给定整数类型的最小值和最大值，计算表示该范围所需的最少位数
		 * Then write only those bits
		 * 注意: A static is used so that the required number of bits for (maximum-minimum) is only calculated once. This does require that minimum and \maximum are fixed values for a given line of code for the life of the program
		 * 参数[输入] value Integer value to write, which should be between minimum and maximum
		 * 参数[输入] minimum Minimum value of value
		 * 参数[输入] maximum Maximum value of value
		 * 参数[输入] allowOutsideRange If true, all sends will take an extra bit, however value can deviate from outside minimum and maximum. If false, will assert if the value deviates. This should match the corresponding value passed to Read().
		 */
		template <class templateType>
		void WriteBitsFromIntegerRange( const templateType value, const templateType minimum, const templateType maximum, bool allowOutsideRange=false );
		/* 参数[输入] requiredBits Primarily for internal use, called from above function() after calculating number of bits needed to represent maximum-minimum */
		template <class templateType>
		void WriteBitsFromIntegerRange( const templateType value, const templateType minimum, const templateType maximum, const int requiredBits, bool allowOutsideRange=false );

		/*
		 * Write a normalized 3D vector, using (at most) 4 bytes + 3 bits instead of 12-24 bytes.
		 * 将进一步压缩沿 y 或 z 轴对齐的向量。
		 * 精度为 1/32767.5。
		 * 参数[输入] x x
		 * 参数[输入] y y
		 * 参数[输入] z z
		 */
		template <class templateType> /* 此函数的 templateType 必须为 float 或 double 类型 */
		void WriteNormVector( templateType x, templateType y, templateType z );

		/*
		 * Write a vector, using 10 bytes instead of 12.
		 * 精度损失约为 3/10，且仅节省 2 字节，
		 * 因此仅在精度不重要时使用。
		 * 参数[输入] x x
		 * 参数[输入] y y
		 * 参数[输入] z z
		 */
		template <class templateType> /* 此函数的 templateType 必须为 float 或 double 类型 */
		void WriteVector( templateType x, templateType y, templateType z );

		/*
		 * Write a normalized quaternion in 6 bytes + 4 bits instead of 16 bytes.  Slightly lossy.
		 * 参数[输入] w w
		 * 参数[输入] x x
		 * 参数[输入] y y
		 * 参数[输入] z z
		 */
		template <class templateType> /* 此函数的 templateType 必须为 float 或 double 类型 */
		void WriteNormQuat( templateType w, templateType x, templateType y, templateType z);

		/*
		 * Write an orthogonal matrix by creating a quaternion, and writing 3 components of the quaternion in 2 bytes each.
		 * 使用 6 字节代替 36 字节
		 * 虽然有损，但结果会被重新归一化
		 */
		template <class templateType> /* 此函数的 templateType 必须为 float 或 double 类型 */
		void WriteOrthMatrix(
			templateType m00, templateType m01, templateType m02,
			templateType m10, templateType m11, templateType m12,
			templateType m20, templateType m21, templateType m22 );

		/*
		 * Read an array or casted stream of byte.
		 * 数组是原始数据。此函数不会进行自动字节序转换
		 * 参数[输入] output The result byte array. It should be larger than @em numberOfBytes.
		 * 参数[输入] numberOfBytes The number of byte to read
		 * 返回值: true on success false if there is some missing bytes.
		 */
		bool Read( char* output, const unsigned int numberOfBytes );

		/*
		 * Read a float into 2 bytes, spanning the range between floatMin and floatMax
		 * 参数[输入] outFloat The float to read
		 * 参数[输入] floatMin Predetermined minimum value of f
		 * 参数[输入] floatMax Predetermined maximum value of f
		 */
		bool ReadFloat16( float &outFloat, float floatMin, float floatMax );

		/*
		 * Read one type serialized to another (smaller) type, to save bandwidth
		 * serializationType 应为 uint8_t、uint16_t、uint24_t 或 uint32_t
		 * Example: int num; ReadCasted<uint8_t>(num); would read 1 bytefrom the stream, and put the value in an integer
		 * 参数[输入] value 要写入的值
		 */
		template <class serializationType, class sourceType >
		bool ReadCasted( sourceType &value );

		/*
		 * 给定整数类型的最小值和最大值，计算表示该范围所需的最少位数
		 * Then read only those bits
		 * 注意: A static is used so that the required number of bits for (maximum-minimum) is only calculated once. This does require that minimum and \maximum are fixed values for a given line of code for the life of the program
		 * 参数[输入] value Integer value to read, which should be between minimum and maximum
		 * 参数[输入] minimum Minimum value of value
		 * 参数[输入] maximum Maximum value of value
		 * 参数[输入] allowOutsideRange If true, all sends will take an extra bit, however value can deviate from outside minimum and maximum. If false, will assert if the value deviates. This should match the corresponding value passed to Write().
		 */
		template <class templateType>
		bool ReadBitsFromIntegerRange( templateType &value, const templateType minimum, const templateType maximum, bool allowOutsideRange=false );
		/* 参数[输入] requiredBits Primarily for internal use, called from above function() after calculating number of bits needed to represent maximum-minimum */
		template <class templateType>
		bool ReadBitsFromIntegerRange( templateType &value, const templateType minimum, const templateType maximum, const int requiredBits, bool allowOutsideRange=false );

		/*
		 * Read a normalized 3D vector, using (at most) 4 bytes + 3 bits instead of 12-24 bytes.
		 * 将进一步压缩沿 y 或 z 轴对齐的向量。
		 * 精度为 1/32767.5。
		 * 参数[输入] x x
		 * 参数[输入] y y
		 * 参数[输入] z z
		 * 返回值: 成功返回 true，失败返回 false。
		 */
		template <class templateType> /* 此函数的 templateType 必须为 float 或 double 类型 */
		bool ReadNormVector( templateType &x, templateType &y, templateType &z );

		/*
		 * Read 3 floats or doubles, using 10 bytes, where those float or doubles comprise a vector.
		 * 精度损失约为 3/10，且仅节省 2 字节，
		 * 因此仅在精度不重要时使用。
		 * 参数[输入] x x
		 * 参数[输入] y y
		 * 参数[输入] z z
		 * 返回值: 成功返回 true，失败返回 false。
		 */
		template <class templateType> /* 此函数的 templateType 必须为 float 或 double 类型 */
		bool ReadVector( templateType &x, templateType &y, templateType &z );

		/*
		 * Read a normalized quaternion in 6 bytes + 4 bits instead of 16 bytes.
		 * 参数[输入] w w
		 * 参数[输入] x x
		 * 参数[输入] y y
		 * 参数[输入] z z
		 * 返回值: 成功返回 true，失败返回 false。
		 */
		template <class templateType> /* 此函数的 templateType 必须为 float 或 double 类型 */
		bool ReadNormQuat( templateType &w, templateType &x, templateType &y, templateType &z);

		/*
		 * Read an orthogonal matrix from a quaternion, reading 3 components of the quaternion in 2 bytes each and extrapolatig the 4th.
		 * 使用 6 字节代替 36 字节
		 * 虽然有损，但结果会被重新归一化
		 * 返回值: 成功返回 true，失败返回 false。
		 */
		template <class templateType> /* 此函数的 templateType 必须为 float 或 double 类型 */
		bool ReadOrthMatrix(
			templateType &m00, templateType &m01, templateType &m02,
			templateType &m10, templateType &m11, templateType &m12,
			templateType &m20, templateType &m21, templateType &m22 );

		/* 将读取指针重置到数据的起始位置。*/
		void ResetReadPointer( void );

		/* 将写入指针重置到数据的起始位置。*/
		void ResetWritePointer( void );

		/*
		 * This is good to call when you are done with the stream to make
		 * sure you didn't leave any data left over void
		 */
		void AssertStreamEmpty( void );

		/* 使用 RAKNET_DEBUG_PRINTF 打印流中的位数据。非常适合调试。*/
		void PrintBits( char *out ) const;
		void PrintBits( void ) const;
		void PrintHex( char *out ) const;
		void PrintHex( void ) const;

		/*
		 * 忽略我们不打算读取的数据
		 * 参数[输入] numberOfBits The number of bits to ignore
		 */
		void IgnoreBits( const BitSize_t numberOfBits );

		/*
		 * 忽略我们不打算读取的数据
		 * 参数[输入] numberOfBits The number of bytes to ignore
		 */
		void IgnoreBytes( const unsigned int numberOfBytes );

		/*
		 * Move the write pointer to a position on the array.
		 * 参数[输入] offset the offset from the start of the array.
		 * \attention
		 * Dangerous if you don't know what you are doing!
		 * 出于效率考虑，只有在数据字节对齐时才能在流的中间位置写入。
		 */
		void SetWriteOffset( const BitSize_t offset );

		/* 返回流的长度（以位为单位）*/
		[[nodiscard]] inline BitSize_t GetNumberOfBitsUsed( void ) const {return GetWriteOffset();}
		[[nodiscard]] inline BitSize_t GetWriteOffset( void ) const {return numberOfBitsUsed;}

		/* 返回流的长度（以字节为单位）*/
		[[nodiscard]] inline BitSize_t GetNumberOfBytesUsed( void ) const {return BITS_TO_BYTES( numberOfBitsUsed );}

		/* 返回流中已读取的位数 */
		[[nodiscard]] inline BitSize_t GetReadOffset( void ) const {return readOffset;}

		/* 设置读取位索引 */
		void SetReadOffset( const BitSize_t newReadOffset ) {readOffset=newReadOffset;}

		/* 返回流中尚未读取的剩余位数 */
		[[nodiscard]] inline BitSize_t GetNumberOfUnreadBits( void ) const {return numberOfBitsUsed - readOffset;}

		/*
		 * Makes a copy of the 内部使用 data for you _data will point to
		 * the stream. Partial bytes are left aligned.
		 * 参数[输出] _data The allocated copy of GetData()
		 * 返回值: The length in bits of the stream.
		 */
		BitSize_t CopyData( unsigned char** _data ) const;

		/*
		 * 内部使用
		 * 将stream设置为some initial data
		 */
		void SetData( unsigned char *inByteArray );

		/*
		 * 获取 data that BitStream is writing to / reading from
		 * Partial bytes are left aligned.
		 * 返回值: A pointer to the 内部使用 state
		 */
		inline unsigned char* GetData( void ) const {return data;}

		/*
		 * Write numberToWrite bits from the input source.
		 * 右对齐数据表示在不完整字节的情况下，位从右侧（位 0）对齐
		 * 而不是从左侧对齐（如常规
		 * 内部使用 representation) You would set this to true when
		 * 写入用户数据时），复制位流数据时应为 false，例如
		 * as writing one bitstream to another.
		 * 参数[输入] inByteArray The data
		 * 参数[输入] numberOfBitsToWrite The number of bits to write
		 * 参数[输入] rightAlignedBits if true data will be right aligned
		 */
		void WriteBits( const unsigned char* inByteArray, BitSize_t numberOfBitsToWrite, const bool rightAlignedBits = true );

		/*
		 * Align the bitstream to the byte boundary and then write the
		 * specified number of bits.
		 * 这是faster than WriteBits but
		 * wastes the bits to do the alignment and requires you to call
		 * ReadAlignedBits at the corresponding read position.
		 * 参数[输入] inByteArray The data
		 * 参数[输入] numberOfBytesToWrite The size of input.
		 */
		void WriteAlignedBytes( const unsigned char *inByteArray, const unsigned int numberOfBytesToWrite );

		/* 对位流中已有的字节进行字节序交换 */
		void EndianSwapBytes( int byteOffset, int length );

		/*
		 * Aligns the bitstream, writes inputLength, and writes input. Won't write beyond maxBytesToWrite
		 * 参数[输入] inByteArray The data
		 * 参数[输入] inputLength The size of input.
		 * 参数[输入] maxBytesToWrite Max bytes to write
		 */
		void WriteAlignedBytesSafe( const char *inByteArray, const unsigned int inputLength, const unsigned int maxBytesToWrite );

		/*
		 * Read bits, starting at the next aligned bits.
		 * Note that the modulus 8 starting offset of the sequence must be the same as
		 * was used with WriteBits. This will be a problem with packet
		 * coalescence unless you byte align the coalesced packets.
		 * 参数[输入] inOutByteArray The byte array larger than @em numberOfBytesToRead
		 * 参数[输入] numberOfBytesToRead The number of byte to read from the 内部使用 state
		 * 返回值: true if there is enough byte.
		 */
		bool ReadAlignedBytes( unsigned char *inOutByteArray, const unsigned int numberOfBytesToRead );

		/*
		 * Reads what was written by WriteAlignedBytesSafe.
		 * 参数[输入] inOutByteArray The data
		 * 参数[输入] maxBytesToRead Maximum number of bytes to read
		 * 返回值: 成功返回 true，失败返回 false。
		 */
		bool ReadAlignedBytesSafe( char *inOutByteArray, int &inputLength, const int maxBytesToRead );
		bool ReadAlignedBytesSafe( char *inOutByteArray, unsigned int &inputLength, const unsigned int maxBytesToRead );

		/*
		 * 与 ReadAlignedBytesSafe() but allocates the memory for you using new, rather than assuming it is safe to write to 相同
		 * 参数[输入] outByteArray outByteArray will be deleted if it is not a pointer to 0
		 * 返回值: 成功返回 true，失败返回 false。
		 */
		bool ReadAlignedBytesSafeAlloc( char **outByteArray, int &inputLength, const unsigned int maxBytesToRead );
		bool ReadAlignedBytesSafeAlloc( char **outByteArray, unsigned int &inputLength, const unsigned int maxBytesToRead );

		/*
		 * 将下一次写入和/或读取对齐到字节边界。
		 * 可用于为了效率而'浪费'位来进行字节对齐
		 * 也可用于强制合并的位流从字节
		 * 边界开始，使 WriteAlignedBits 和 ReadAlignedBits
		 * 在对齐时计算相同的偏移量。
		 */
		inline void AlignWriteToByteBoundary( void ) {numberOfBitsUsed += 8 - ( (( numberOfBitsUsed - 1 ) & 7) + 1 );}

		/*
		 * 将下一次写入和/或读取对齐到字节边界。
		 * 可用于为了效率而'浪费'位来进行字节对齐
		 * 也可用于强制合并的位流从字节
		 * 边界开始，使 WriteAlignedBits 和 ReadAlignedBits
		 * 在对齐时计算相同的偏移量。
		 */
		inline void AlignReadToByteBoundary( void ) {readOffset += 8 - ( (( readOffset - 1 ) & 7 ) + 1 );}

		/*
		 * Read numberOfBitsToRead bits to the output source.
		 * alignBitsToRight should be set to true to convert 内部使用
		 * bitstream data to userdata. It should be false if you used
		 * WriteBits with rightAlignedBits false
		 * 参数[输入] inOutByteArray The resulting bits array
		 * 参数[输入] numberOfBitsToRead The number of bits to read
		 * 参数[输入] alignBitsToRight if true bits will be right aligned.
		 * 返回值: true if there is enough bits to read
		 */
		bool ReadBits( unsigned char *inOutByteArray, BitSize_t numberOfBitsToRead, const bool alignBitsToRight = true );

		/* 写入一个 0 */
		void Write0( void );

		/* 写入一个 1 */
		void Write1( void );

		/* 读取 1 位，如果该位为 1 则返回 true，为 0 则返回 false。*/
		bool ReadBit( void );

		/*
		 * If we used the 构造函数 version with copy data off, this
		 * *makes sure it is set to on and the data pointed to is copied.
		 */
		void AssertCopyData( void );

		/*
		 * Use this if you pass a pointer copy to the 构造函数
		 * *(_copyData==false) and want to overallocate to prevent
		 * reallocation.
		 */
		void SetNumberOfBitsAllocated( const BitSize_t lengthInBits );

		/* 为写入 numberOfBitsToWrite 位数据做准备，必要时重新分配内存 */
		void AddBitsAndReallocate( const BitSize_t numberOfBitsToWrite );

		/*
		 * 内部使用
		 * 返回值: How many bits have been allocated internally
		 */
		BitSize_t GetNumberOfBitsAllocated(void) const;

		/* 读取字符串（非引用方式）。*/
		bool Read(char *varString);
		bool Read(unsigned char *varString);

		/* 写入零直到位流填充到指定字节数 */
		void PadWithZeroToByteLength( unsigned int bytes );

		/*
		 * 获取 number of leading zeros for a number
		 * 参数[输入] x Number to test
		 */
		static int NumberOfLeadingZeroes( uint8_t x );
		static int NumberOfLeadingZeroes( uint16_t x );
		static int NumberOfLeadingZeroes( uint32_t x );
		static int NumberOfLeadingZeroes( uint64_t x );
		static int NumberOfLeadingZeroes( int8_t x );
		static int NumberOfLeadingZeroes( int16_t x );
		static int NumberOfLeadingZeroes( int32_t x );
		static int NumberOfLeadingZeroes( int64_t x );

		/* 内部使用 Unrolled inner loop, for when performance is critical */
		void WriteAlignedVar8(const char *inByteArray);
		/* 内部使用 Unrolled inner loop, for when performance is critical */
		bool ReadAlignedVar8(char *inOutByteArray);
		/* 内部使用 Unrolled inner loop, for when performance is critical */
		void WriteAlignedVar16(const char *inByteArray);
		/* 内部使用 Unrolled inner loop, for when performance is critical */
		bool ReadAlignedVar16(char *inOutByteArray);
		/* 内部使用 Unrolled inner loop, for when performance is critical */
		void WriteAlignedVar32(const char *inByteArray);
		/* 内部使用 Unrolled inner loop, for when performance is critical */
		bool ReadAlignedVar32(char *inOutByteArray);

		inline void Write(const char * const inStringVar)
		{
			RakString::Serialize(inStringVar, this);
		}
		inline void Write(const wchar_t * const inStringVar)
		{
			RakWString::Serialize(inStringVar, this);
		}
		inline void Write(const unsigned char * const inTemplateVar)
		{
			Write((const char*)inTemplateVar);
		}
		inline void Write(char * const inTemplateVar)
		{
			Write((const char*)inTemplateVar);
		}
		inline void Write(unsigned char * const inTemplateVar)
		{
			Write((const char*)inTemplateVar);
		}
		inline void WriteCompressed(const char * const inStringVar)
		{
			RakString::SerializeCompressed(inStringVar,this,0,false);
		}
		inline void WriteCompressed(const wchar_t * const inStringVar)
		{
			RakWString::Serialize(inStringVar,this);
		}
		inline void WriteCompressed(const unsigned char * const inTemplateVar)
		{
			WriteCompressed((const char*) inTemplateVar);
		}
		inline void WriteCompressed(char * const inTemplateVar)
		{
			WriteCompressed((const char*) inTemplateVar);
		}
		inline void WriteCompressed(unsigned char * const inTemplateVar)
		{
			WriteCompressed((const char*) inTemplateVar);
		}

		/* ---- 成员函数模板特化声明 ----*/
		/* 用于 VC7 */
#if defined(_MSC_VER) && _MSC_VER == 1300
		/*
		 * Write a bool to a bitstream.
		 * 参数[输入] var 要写入的值
		 */
		template <>
			void Write(const bool &var);

		/*
		 * Write a systemAddress to a bitstream
		 * 参数[输入] var 要写入的值
		 */
		template <>
			void Write(const SystemAddress &var);

		/*
		 * Write a uint24_t to a bitstream
		 * 参数[输入] var 要写入的值
		 */
		template <>
		void Write(const uint24_t &var);

		/*
		 * Write a RakNetGUID to a bitsteam
		 * 参数[输入] var 要写入的值
		 */
		template <>
			void Write(const RakNetGuid &var);

		/*
		 * Write a string to a bitstream
		 * 参数[输入] var 要写入的值
		 */
		template <>
			void Write(const char* const &var);
		template <>
			void Write(const unsigned char* const &var);
		template <>
			void Write(char* const &var);
		template <>
			void Write(unsigned char* const &var);
		template <>
			void Write(const RakString &var);
		template <>
			void Write(const RakWString &var);

		/*
		 * 写入 systemAddress
		 * 若当前值与上一次的值不同
		 * 则写入当前值；否则仅写入一个比特
		 * 参数[输入] currentValue 要写入的当前值
		 * 参数[输入] lastValue The last value to compare against
		 */
		template <>
			void WriteDelta(const SystemAddress &currentValue, const SystemAddress &lastValue);

		template <>
		void WriteDelta(const uint24_t &currentValue, const uint24_t &lastValue);

		template <>
			void WriteDelta(const RakNetGUID &currentValue, const RakNetGUID &lastValue);

		/*
		 * 写入 bool delta
		 * 与直接调用 Write 相同
		 * 参数[输入] currentValue 要写入的当前值
		 * 参数[输入] lastValue The last value to compare against
		 */
		template <>
			void WriteDelta(const bool &currentValue, const bool &lastValue);

		template <>
			void WriteCompressed(const SystemAddress &var);

		template <>
		void WriteCompressed(const uint24_t &var);

		template <>
			void WriteCompressed(const RakNetGUID &var);

		template <>
			void WriteCompressed(const bool &var);

		/* 用于 -1 到 1 之间的值 */
		template <>
			void WriteCompressed(const float &var);

		/* 用于 -1 到 1 之间的值 */
		template <>
			void WriteCompressed(const double &var);

		/* 压缩字符串 */
		template <>
			void WriteCompressed(const char* var);
		template <>
			void WriteCompressed(const unsigned char* var);
		template <>
			void WriteCompressed(char* var);
		template <>
			void WriteCompressed(unsigned char* var);
		template <>
			void WriteCompressed(const RakString &var);
		template <>
			void WriteCompressed(const RakWString &var);

		/*
		 * 写入 bool delta
		 * 与直接调用 Write 相同
		 * 参数[输入] currentValue 要写入的当前值
		 * 参数[输入] lastValue The last value to compare against
		 */
		template <>
			void WriteCompressedDelta(const bool &currentValue, const bool &lastValue);

		/*
		 * 与 WriteCompressedDelta(bool currentValue, const templateType &lastValue) 功能相同
		 * 当第二个布尔参数未知时
		 */
		template <>
			void WriteCompressedDelta(const bool &currentValue);

		/*
		 * 从位流中读取一个布尔值。
		 * 参数[输入] var The value to read
		 * 返回值: 成功返回 true，失败返回 false。
		 */
		template <>
			bool Read(bool &var);

		/*
		 * Read a systemAddress from a bitstream.
		 * 参数[输入] var The value to read
		 * 返回值: 成功返回 true，失败返回 false。
		 */
		template <>
			bool Read(SystemAddress &var);

		template <>
		bool Read(uint24_t &var);

		template <>
			bool Read(RakNetGUID &var);

		/*
		 * Read a String from a bitstream.
		 * 参数[输入] var The value to read
		 * 返回值: 成功返回 true，失败返回 false。
		 */
		template <>
			bool Read(char *&var);
		template <>
			bool Read(wchar_t *&var);
		template <>
			bool Read(unsigned char *&var);
		template <>
			bool Read(RakString &var);
		template <>
			bool Read(RakWString &var);

		/*
		 * 从位流中读取一个布尔值。
		 * 参数[输入] var The value to read
		 * 返回值: 成功返回 true，失败返回 false。
		 */
		template <>
			bool ReadDelta(bool &var);

		template <>
			bool ReadCompressed(SystemAddress &var);

		template <>
		bool ReadCompressed(uint24_t &var);

		template <>
			bool ReadCompressed(RakNetGUID &var);

		template <>
			bool ReadCompressed(bool &var);

		template <>
			bool ReadCompressed(float &var);

		/*
		 * 用于 -1 到 1 之间的值
		 * 返回值: 成功返回 true，失败返回 false。
		 */
		template <>
		bool ReadCompressed(double &var);

		template <>
			bool ReadCompressed(char* &var);
		template <>
			bool ReadCompressed(wchar_t* &var);
		template <>
			bool ReadCompressed(unsigned char *&var);
		template <>
			bool ReadCompressed(RakString &var);
		template <>
			bool ReadCompressed(RakWString &var);

		/*
		 * 从位流中读取一个布尔值。
		 * 参数[输入] var The value to read
		 * 返回值: 成功返回 true，失败返回 false。
		 */
		template <>
			bool ReadCompressedDelta(bool &var);
#endif

		inline static bool DoEndianSwap() {
#ifndef __BITSTREAM_NATIVE_END
			return IsNetworkOrder()==false;
#else
			return false;
#endif
		}
		inline static bool IsBigEndian()
		{
			return IsNetworkOrder();
		}
		inline static bool IsNetworkOrder() {bool r = IsNetworkOrderInternal(); return r;}
		/* 非内联函数，在 PC 上由于 winsock 包含错误无法编译 */
		static bool IsNetworkOrderInternal();
		static void ReverseBytes(unsigned char *inByteArray, unsigned char *inOutByteArray, const unsigned int length);
		static void ReverseBytesInPlace(unsigned char *inOutData,const unsigned int length);

	private:

		BitStream( const BitStream &invalid) {
			static_cast<void>(invalid);
			RakAssert(0);
		}

		BitStream& operator = ( const BitStream& invalid ) {
			static_cast<void>(invalid);
			RakAssert(0);
			static BitStream i;
			return i;
		}

		/* 假设输入源指向原生类型，对其进行压缩并写入。*/
		void WriteCompressed( const unsigned char* inByteArray, const unsigned int size, const bool unsignedData );

		/* 假设输入源指向已压缩的原生类型，对其进行解压缩并读取。*/
		bool ReadCompressed( unsigned char* inOutByteArray,	const unsigned int size, const bool unsignedData );


		BitSize_t numberOfBitsUsed;

		BitSize_t numberOfBitsAllocated;

		BitSize_t readOffset;

		unsigned char *data;

		/* true if the 内部使用 buffer is copy of the data passed to the 构造函数 */
		bool copyData;

		/* 使用不到 BITSTREAM_STACK_ALLOCATION_SIZE 的 BitStream 使用栈而非堆来存储数据。超过此大小时会切换到堆存储 */
		unsigned char stackData[BITSTREAM_STACK_ALLOCATION_SIZE];
	};

		template <class templateType>
		inline bool BitStream::Serialize(bool writeToBitstream, templateType &inOutTemplateVar)
		{
			if (writeToBitstream)
				Write(inOutTemplateVar);
			else
				return Read(inOutTemplateVar);
			return true;
		}

		template <class templateType>
		inline bool BitStream::SerializeDelta(bool writeToBitstream, templateType &inOutCurrentValue, const templateType &lastValue)
		{
			if (writeToBitstream)
				WriteDelta(inOutCurrentValue, lastValue);
			else
				return ReadDelta(inOutCurrentValue);
			return true;
		}

		template <class templateType>
		inline bool BitStream::SerializeDelta(bool writeToBitstream, templateType &inOutCurrentValue)
		{
			if (writeToBitstream)
				WriteDelta(inOutCurrentValue);
			else
				return ReadDelta(inOutCurrentValue);
			return true;
		}

		template <class templateType>
		inline bool BitStream::SerializeCompressed(bool writeToBitstream, templateType &inOutTemplateVar)
		{
			if (writeToBitstream)
				WriteCompressed(inOutTemplateVar);
			else
				return ReadCompressed(inOutTemplateVar);
			return true;
		}

		template <class templateType>
		inline bool BitStream::SerializeCompressedDelta(bool writeToBitstream, templateType &inOutCurrentValue, const templateType &lastValue)
		{
			if (writeToBitstream)
				WriteCompressedDelta(inOutCurrentValue,lastValue);
			else
				return ReadCompressedDelta(inOutCurrentValue);
			return true;
		}
/* 此处停止 */
		template <class templateType>
		inline bool BitStream::SerializeCompressedDelta(bool writeToBitstream, templateType &inOutCurrentValue)
		{
			if (writeToBitstream)
				WriteCompressedDelta(inOutCurrentValue);
			else
				return ReadCompressedDelta(inOutCurrentValue);
			return true;
		}

		inline bool BitStream::Serialize(bool writeToBitstream, char* inOutByteArray, const unsigned int numberOfBytes )
		{
			if (writeToBitstream)
				Write(inOutByteArray, numberOfBytes);
			else
				return Read(inOutByteArray, numberOfBytes);
			return true;
		}
		
		template <class serializationType, class sourceType >
		bool BitStream::SerializeCasted( bool writeToBitstream, sourceType &value )
		{
			if (writeToBitstream) WriteCasted<serializationType>(value);
			else return ReadCasted<serializationType>(value);
			return true;
		}

		template <class templateType>
		bool BitStream::SerializeBitsFromIntegerRange( bool writeToBitstream, templateType &value, const templateType minimum, const templateType maximum, bool allowOutsideRange )
		{
			int requiredBits=BYTES_TO_BITS(sizeof(templateType))-NumberOfLeadingZeroes(templateType(maximum-minimum));
			return SerializeBitsFromIntegerRange(writeToBitstream,value,minimum,maximum,requiredBits,allowOutsideRange);
		}
		template <class templateType>
		bool BitStream::SerializeBitsFromIntegerRange( bool writeToBitstream, templateType &value, const templateType minimum, const templateType maximum, const int requiredBits, bool allowOutsideRange )
		{
			if (writeToBitstream) WriteBitsFromIntegerRange(value,minimum,maximum,requiredBits,allowOutsideRange);
			else return ReadBitsFromIntegerRange(value,minimum,maximum,requiredBits,allowOutsideRange);
			return true;
		}

		template <class templateType>
		inline bool BitStream::SerializeNormVector(bool writeToBitstream, templateType &x, templateType &y, templateType &z )
		{
			if (writeToBitstream)
				WriteNormVector(x,y,z);
			else
				return ReadNormVector(x,y,z);
			return true;
		}

		template <class templateType>
		inline bool BitStream::SerializeVector(bool writeToBitstream,  templateType &x, templateType &y, templateType &z )
		{
			if (writeToBitstream)
				WriteVector(x,y,z);
			else
				return ReadVector(x,y,z);
			return true;
		}

		template <class templateType>
		inline bool BitStream::SerializeNormQuat(bool writeToBitstream,  templateType &w, templateType &x, templateType &y, templateType &z)
		{
			if (writeToBitstream)
				WriteNormQuat(w,x,y,z);
			else
				return ReadNormQuat(w,x,y,z);
			return true;
		}

		template <class templateType>
		inline bool BitStream::SerializeOrthMatrix(
		bool writeToBitstream,
		templateType &m00, templateType &m01, templateType &m02,
		templateType &m10, templateType &m11, templateType &m12,
		templateType &m20, templateType &m21, templateType &m22 )
		{
			if (writeToBitstream)
				WriteOrthMatrix(m00,m01,m02,m10,m11,m12,m20,m21,m22);
			else
				return ReadOrthMatrix(m00,m01,m02,m10,m11,m12,m20,m21,m22);
			return true;
		}

		inline bool BitStream::SerializeBits(bool writeToBitstream, unsigned char* inOutByteArray, const BitSize_t numberOfBitsToSerialize, const bool rightAlignedBits )
		{
			if (writeToBitstream)
				WriteBits(inOutByteArray,numberOfBitsToSerialize,rightAlignedBits);
			else
				return ReadBits(inOutByteArray,numberOfBitsToSerialize,rightAlignedBits);
			return true;
		}

	template <class templateType>
		inline void BitStream::Write(const templateType &inTemplateVar)
	{
#ifdef _MSC_VER
#pragma warning(disable:4127) /* 条件表达式是常量 */
#endif
		if (sizeof(inTemplateVar)==1)
			WriteBits( ( unsigned char* ) & inTemplateVar, sizeof( templateType ) * 8, true );
		else
		{
#ifndef __BITSTREAM_NATIVE_END
			if (DoEndianSwap())
			{
				unsigned char output[sizeof(templateType)];
				ReverseBytes((unsigned char*)&inTemplateVar, output, sizeof(templateType));
				WriteBits( ( unsigned char* ) output, sizeof(templateType) * 8, true );
			}
			else
#endif
				WriteBits( ( unsigned char* ) & inTemplateVar, sizeof(templateType) * 8, true );
		}
	}

	template <class templateType>
	inline void BitStream::WritePtr(templateType *inTemplateVar)
	{
#ifdef _MSC_VER
#pragma warning(disable:4127) /* 条件表达式是常量 */
#endif
		if (sizeof(templateType)==1)
			WriteBits( ( unsigned char* ) inTemplateVar, sizeof( templateType ) * 8, true );
		else
		{
#ifndef __BITSTREAM_NATIVE_END
			if (DoEndianSwap())
			{
				unsigned char output[sizeof(templateType)];
				ReverseBytes((unsigned char*) inTemplateVar, output, sizeof(templateType));
				WriteBits( ( unsigned char* ) output, sizeof(templateType) * 8, true );
			}
			else
#endif
				WriteBits( ( unsigned char* ) inTemplateVar, sizeof(templateType) * 8, true );
		}
	}

	/*
	 * Write a bool to a bitstream.
	 * 参数[输入] inTemplateVar 要写入的值
	 */
	template <>
		inline void BitStream::Write(const bool &inTemplateVar)
		{
			if ( inTemplateVar )
				Write1();
			else
				Write0();
		}


	/*
	 * Write a systemAddress to a bitstream.
	 * 参数[输入] inTemplateVar 要写入的值
	 */
	template <>
		inline void BitStream::Write(const SystemAddress &inTemplateVar)
	{
		Write(inTemplateVar.GetIPVersion());
		if (inTemplateVar.GetIPVersion()==4)
		{
			/* 隐藏地址以防止路由器修改它 */
			SystemAddress var2=inTemplateVar;
			uint32_t binaryAddress=~inTemplateVar.address.addr4.sin_addr.s_addr;
			/* 不对地址或端口进行字节序交换 */
			WriteBits((unsigned char*)&binaryAddress, sizeof(binaryAddress)*8, true);
			unsigned short p = var2.GetPortNetworkOrder();
			WriteBits((unsigned char*)&p, sizeof(unsigned short)*8, true);
		}
		else
		{
#if RAKNET_SUPPORT_IPV6==1
			/* 不进行字节序交换 */
			WriteBits((const unsigned char*) &inTemplateVar.address.addr6, sizeof(inTemplateVar.address.addr6)*8, true);
#endif
		}
	}

	template <>
	inline void BitStream::Write(const uint24_t &inTemplateVar)
	{
		AlignWriteToByteBoundary();
		AddBitsAndReallocate(3*8);

		if (IsBigEndian()==false)
		{
			data[( numberOfBitsUsed >> 3 ) + 0] = ((unsigned char *)&inTemplateVar.val)[0];
			data[( numberOfBitsUsed >> 3 ) + 1] = ((unsigned char *)&inTemplateVar.val)[1];
			data[( numberOfBitsUsed >> 3 ) + 2] = ((unsigned char *)&inTemplateVar.val)[2];
		}
		else
		{
			data[( numberOfBitsUsed >> 3 ) + 0] = ((unsigned char *)&inTemplateVar.val)[3];
			data[( numberOfBitsUsed >> 3 ) + 1] = ((unsigned char *)&inTemplateVar.val)[2];
			data[( numberOfBitsUsed >> 3 ) + 2] = ((unsigned char *)&inTemplateVar.val)[1];
		}

		numberOfBitsUsed+=3*8;
	}

	template <>
		inline void BitStream::Write(const RakNetGUID &inTemplateVar)
		{
			Write(inTemplateVar.g);
		}

	/*
	 * Write a string to a bitstream.
	 * 参数[输入] var 要写入的值
	 */
	template <>
		inline void BitStream::Write(const RakString &inTemplateVar)
	{
		inTemplateVar.Serialize(this);
	}
	template <>
		inline void BitStream::Write(const RakWString &inTemplateVar)
	{
		inTemplateVar.Serialize(this);
	}
	template <>
		inline void BitStream::Write(const char * const &inStringVar)
	{
		RakString::Serialize(inStringVar, this);
	}
	template <>
		inline void BitStream::Write(const wchar_t * const &inStringVar)
	{
		RakWString::Serialize(inStringVar, this);
	}
	template <>
		inline void BitStream::Write(const unsigned char * const &inTemplateVar)
	{
		Write((const char*)inTemplateVar);
	}
	template <>
		inline void BitStream::Write(char * const &inTemplateVar)
	{
		Write((const char*)inTemplateVar);
	}
	template <>
		inline void BitStream::Write(unsigned char * const &inTemplateVar)
	{
		Write((const char*)inTemplateVar);
	}

	/*
	 * 将任意整数类型写入位流。
	 * 若当前值与上一次的值不同
	 * 则写入当前值；否则仅写入一个比特
	 * 参数[输入] currentValue 要写入的当前值
	 * 参数[输入] lastValue The last value to compare against
	 */
	template <class templateType>
		inline void BitStream::WriteDelta(const templateType &currentValue, const templateType &lastValue)
	{
		if (currentValue==lastValue)
		{
			Write(false);
		}
		else
		{
			Write(true);
			Write(currentValue);
		}
	}

	/*
	 * Write a bool delta. Same thing as just calling Write
	 * 参数[输入] currentValue 要写入的当前值
	 * 参数[输入] lastValue The last value to compare against
	 */
	template <>
		inline void BitStream::WriteDelta(const bool &currentValue, const bool &lastValue)
	{
		static_cast<void>(lastValue);

		Write(currentValue);
	}

	/*
	 * 当你不知道上一次的值是什么或没有上一次的值时，使用 WriteDelta。
	 * 参数[输入] currentValue 要写入的当前值
	 */
	template <class templateType>
		inline void BitStream::WriteDelta(const templateType &currentValue)
	{
		Write(true);
		Write(currentValue);
	}

	/*
	 * 将任意整数类型写入位流。
	 * 若需要字节序转换，请取消定义 __BITSTREAM_NATIVE_END。
	 * 对于浮点数，这是有损的，float 使用 2 字节，double 使用 4 字节。范围必须在 -1 到 +1 之间。
	 * 对于非浮点数，这是无损的，但仅在使用不到该类型一半位数时才有收益
	 * 如果未使用 __BITSTREAM_NATIVE_END，则对于大于 1 字节的类型，情况相反
	 * 参数[输入] inTemplateVar 要写入的值
	 */
	template <class templateType>
		inline void BitStream::WriteCompressed(const templateType &inTemplateVar)
	{
#ifdef _MSC_VER
#pragma warning(disable:4127) /* 条件表达式是常量 */
#endif
		if (sizeof(inTemplateVar)==1)
			WriteCompressed( ( unsigned char* ) & inTemplateVar, sizeof( templateType ) * 8, true );
		else
		{
#ifndef __BITSTREAM_NATIVE_END
#ifdef _MSC_VER
#pragma warning(disable:4244) /* '='：从 'unsigned long' 到 'unsigned short' 的转换，可能丢失数据 */
#endif

			if (DoEndianSwap())
			{
				unsigned char output[sizeof(templateType)];
				ReverseBytes((unsigned char*)&inTemplateVar, output, sizeof(templateType));
				WriteCompressed( ( unsigned char* ) output, sizeof(templateType) * 8, true );
			}
			else
#endif
				WriteCompressed( ( unsigned char* ) & inTemplateVar, sizeof(templateType) * 8, true );
		}
	}

	template <>
		inline void BitStream::WriteCompressed(const SystemAddress &inTemplateVar)
	{
		Write(inTemplateVar);
	}

	template <>
	inline void BitStream::WriteCompressed(const RakNetGUID &inTemplateVar)
	{
		Write(inTemplateVar);
	}

	template <>
	inline void BitStream::WriteCompressed(const uint24_t &var)
	{
		Write(var);
	}

	template <>
		inline void BitStream::WriteCompressed(const bool &inTemplateVar)
	{
		Write(inTemplateVar);
	}

	/* 用于 -1 到 1 之间的值 */
	template <>
		inline void BitStream::WriteCompressed(const float &inTemplateVar)
	{
		RakAssert(inTemplateVar > -1.01f && inTemplateVar < 1.01f);
		float varCopy=inTemplateVar;
		if (varCopy < -1.0f)
			varCopy=-1.0f;
		if (varCopy > 1.0f)
			varCopy=1.0f;
		Write(static_cast<unsigned short>((varCopy+1.0f)*32767.5f));
	}

	/* 用于 -1 到 1 之间的值 */
	template <>
		inline void BitStream::WriteCompressed(const double &inTemplateVar)
	{
		RakAssert(inTemplateVar > -1.01 && inTemplateVar < 1.01);
		double varCopy=inTemplateVar;
		if (varCopy < -1.0f)
			varCopy=-1.0f;
		if (varCopy > 1.0f)
			varCopy=1.0f;
		Write(static_cast<uint32_t>((varCopy+1.0)*2147483648.0));
	}

	/* 压缩字符串 */
	template <>
		inline void BitStream::WriteCompressed(const RakString &inTemplateVar)
	{
		inTemplateVar.SerializeCompressed(this,0,false);
	}
	template <>
	inline void BitStream::WriteCompressed(const RakWString &inTemplateVar)
	{
		inTemplateVar.Serialize(this);
	}
	template <>
		inline void BitStream::WriteCompressed(const char * const &inStringVar)
	{
		RakString::SerializeCompressed(inStringVar,this,0,false);
	}
	template <>
	inline void BitStream::WriteCompressed(const wchar_t * const &inStringVar)
	{
		RakWString::Serialize(inStringVar,this);
	}
	template <>
		inline void BitStream::WriteCompressed(const unsigned char * const &inTemplateVar)
	{
		WriteCompressed((const char*) inTemplateVar);
	}
	template <>
		inline void BitStream::WriteCompressed(char * const &inTemplateVar)
	{
		WriteCompressed((const char*) inTemplateVar);
	}
	template <>
		inline void BitStream::WriteCompressed(unsigned char * const &inTemplateVar)
	{
		WriteCompressed((const char*) inTemplateVar);
	}
	

	/*
	 * 将任意整数类型写入位流。
	 * 若当前值与上一次的值不同
	 * 则写入当前值；否则仅写入一个比特
	 * 对于浮点数，这是有损的，float 使用 2 字节，double 使用 4 字节。范围必须在 -1 到 +1 之间。
	 * 对于非浮点数，这是无损的，但仅在使用不到该类型一半位数时才有收益
	 * 如果未使用 __BITSTREAM_NATIVE_END，则对于大于 1 字节的类型，情况相反
	 * 参数[输入] currentValue 要写入的当前值
	 * 参数[输入] lastValue The last value to compare against
	 */
	template <class templateType>
		inline void BitStream::WriteCompressedDelta(const templateType &currentValue, const templateType &lastValue)
	{
		if (currentValue==lastValue)
		{
			Write(false);
		}
		else
		{
			Write(true);
			WriteCompressed(currentValue);
		}
	}

	/*
	 * Write a bool delta.  Same thing as just calling Write
	 * 参数[输入] currentValue 要写入的当前值
	 * 参数[输入] lastValue The last value to compare against
	 */
	template <>
		inline void BitStream::WriteCompressedDelta(const bool &currentValue, const bool &lastValue)
	{
		static_cast<void>(lastValue);

		Write(currentValue);
	}

	/*
	 * Save as WriteCompressedDelta(const templateType &currentValue, const templateType &lastValue)
	 * when we have an unknown second parameter
	 */
	template <class templateType>
		inline void BitStream::WriteCompressedDelta(const templateType &currentValue)
	{
		Write(true);
		WriteCompressed(currentValue);
	}

	/*
	 * 与 WriteCompressedDelta(bool currentValue, const templateType &lastValue) 功能相同
	 * 当第二个布尔参数未知时
	 */
	template <>
		inline void BitStream::WriteCompressedDelta(const bool &currentValue)
	{
		Write(currentValue);
	}

	/*
	 * Read any integral type from a bitstream.  Define __BITSTREAM_NATIVE_END if you need endian swapping.
	 * 参数[输入] outTemplateVar The value to read
	 */
	template <class templateType>
		inline bool BitStream::Read(templateType &outTemplateVar)
	{
#ifdef _MSC_VER
#pragma warning(disable:4127) /* 条件表达式是常量 */
#endif
		if (sizeof(outTemplateVar)==1)
			return ReadBits( ( unsigned char* ) &outTemplateVar, sizeof(templateType) * 8, true );
		else
		{
#ifndef __BITSTREAM_NATIVE_END
#ifdef _MSC_VER
#pragma warning(disable:4244) /* '='：从 'unsigned long' 到 'unsigned short' 的转换，可能丢失数据 */
#endif
			if (DoEndianSwap())
			{
				unsigned char output[sizeof(templateType)];
				if (ReadBits( ( unsigned char* ) output, sizeof(templateType) * 8, true ))
				{
					ReverseBytes(output, (unsigned char*)&outTemplateVar, sizeof(templateType));
					return true;
				}
				return false;
			}
			else
#endif
				return ReadBits( ( unsigned char* ) & outTemplateVar, sizeof(templateType) * 8, true );
		}
	}

	/*
	 * 从位流中读取一个布尔值。
	 * 参数[输入] outTemplateVar The value to read
	 */
	template <>
		inline bool BitStream::Read(bool &outTemplateVar)
	{
		if ( readOffset + 1 > numberOfBitsUsed )
			return false;

		if ( data[ readOffset >> 3 ] & ( 0x80 >> ( readOffset & 7 ) ) ) /* Is it faster to just write it out here? */
			outTemplateVar = true;
		else
			outTemplateVar = false;

		/* 在 Mac 上必须放在不同的行 */
		readOffset++;

		return true;
	}

	/*
	 * Read a systemAddress from a bitstream.
	 * 参数[输入] outTemplateVar The value to read
	 */
	template <>
		inline bool BitStream::Read(SystemAddress &outTemplateVar)
	{
		unsigned char ipVersion;
		Read(ipVersion);
		if (ipVersion==4)
		{
			outTemplateVar.address.addr4.sin_family=AF_INET;
			/* Read(var.binaryAddress); */
			/* 不对地址或端口进行字节序交换 */
			uint32_t binaryAddress;
			ReadBits( ( unsigned char* ) & binaryAddress, sizeof(binaryAddress) * 8, true );
			/* 取消隐藏 IP 地址（此操作是为了防止路由器修改它）*/
			outTemplateVar.address.addr4.sin_addr.s_addr=~binaryAddress;
			bool b = ReadBits(( unsigned char* ) & outTemplateVar.address.addr4.sin_port, sizeof(outTemplateVar.address.addr4.sin_port) * 8, true);
			outTemplateVar.debugPort=ntohs(outTemplateVar.address.addr4.sin_port);
			return b;
		}
		else
		{
#if RAKNET_SUPPORT_IPV6==1
			bool b = ReadBits((unsigned char*) &outTemplateVar.address.addr6, sizeof(outTemplateVar.address.addr6)*8, true);
			outTemplateVar.debugPort=ntohs(outTemplateVar.address.addr6.sin6_port);
			return b;
#else
			return false;
#endif
		}	
	}

	template <>
	inline bool BitStream::Read(uint24_t &outTemplateVar)
	{
		AlignReadToByteBoundary();
		if ( readOffset + 3*8 > numberOfBitsUsed )
			return false;

		if (IsBigEndian()==false)
		{
			((unsigned char *)&outTemplateVar.val)[0]=data[ (readOffset >> 3) + 0];
			((unsigned char *)&outTemplateVar.val)[1]=data[ (readOffset >> 3) + 1];
			((unsigned char *)&outTemplateVar.val)[2]=data[ (readOffset >> 3) + 2];
			((unsigned char *)&outTemplateVar.val)[3]=0;
		}
		else
		{

			((unsigned char *)&outTemplateVar.val)[3]=data[ (readOffset >> 3) + 0];
			((unsigned char *)&outTemplateVar.val)[2]=data[ (readOffset >> 3) + 1];
			((unsigned char *)&outTemplateVar.val)[1]=data[ (readOffset >> 3) + 2];
			((unsigned char *)&outTemplateVar.val)[0]=0;
		}

		readOffset+=3*8;
		return true;
	}

	template <>
	inline bool BitStream::Read(RakNetGUID &outTemplateVar)
	{
		return Read(outTemplateVar.g);
	}


	template <>
		inline bool BitStream::Read(RakString &outTemplateVar)
	{
		return outTemplateVar.Deserialize(this);
	}
	template <>
	inline bool BitStream::Read(RakWString &outTemplateVar)
	{
		return outTemplateVar.Deserialize(this);
	}
	template <>
		inline bool BitStream::Read(char *&varString)
	{
		return RakString::Deserialize(varString,this);
	}
	template <>
	inline bool BitStream::Read(wchar_t *&varString)
	{
		return RakWString::Deserialize(varString,this);
	}
	template <>
		inline bool BitStream::Read(unsigned char *&varString)
	{
		return RakString::Deserialize((char*) varString,this);
	}

	/*
	 * 从位流中读取任意整数类型。
	 * 如果写入的值与写入函数中用于比较的值不同，
	 * var 将被更新。否则将保留当前值。
	 * ReadDelta 仅在之前调用过 WriteDelta 后才有效
	 * 参数[输入] outTemplateVar The value to read
	 */
	template <class templateType>
		inline bool BitStream::ReadDelta(templateType &outTemplateVar)
	{
		bool dataWritten;
		bool success;
		success=Read(dataWritten);
		if (dataWritten)
			success=Read(outTemplateVar);
		return success;
	}

	/*
	 * 从位流中读取一个布尔值。
	 * 参数[输入] outTemplateVar The value to read
	 */
	template <>
		inline bool BitStream::ReadDelta(bool &outTemplateVar)
	{
		return Read(outTemplateVar);
	}

	/*
	 * 从位流中读取任意整数类型。
	 * 若需要字节序转换，请取消定义 __BITSTREAM_NATIVE_END。
	 * 对于浮点数，这是有损的，float 使用 2 字节，double 使用 4 字节。范围必须在 -1 到 +1 之间。
	 * 对于非浮点数，这是无损的，但仅在使用不到该类型一半位数时才有收益
	 * 如果未使用 __BITSTREAM_NATIVE_END，则对于大于 1 字节的类型，情况相反
	 * 参数[输入] outTemplateVar The value to read
	 */
	template <class templateType>
		inline bool BitStream::ReadCompressed(templateType &outTemplateVar)
	{
#ifdef _MSC_VER
#pragma warning(disable:4127) /* 条件表达式是常量 */
#endif
		if (sizeof(outTemplateVar)==1)
			return ReadCompressed( ( unsigned char* ) &outTemplateVar, sizeof(templateType) * 8, true );
		else
		{
#ifndef __BITSTREAM_NATIVE_END
			if (DoEndianSwap())
			{
				unsigned char output[sizeof(templateType)];
				if (ReadCompressed( ( unsigned char* ) output, sizeof(templateType) * 8, true ))
				{
					ReverseBytes(output, (unsigned char*)&outTemplateVar, sizeof(templateType));
					return true;
				}
				return false;
			}
			else
#endif
				return ReadCompressed( ( unsigned char* ) & outTemplateVar, sizeof(templateType) * 8, true );
		}
	}

	template <>
		inline bool BitStream::ReadCompressed(SystemAddress &outTemplateVar)
	{
		return Read(outTemplateVar);
	}

	template <>
	inline bool BitStream::ReadCompressed(uint24_t &outTemplateVar)
	{
		return Read(outTemplateVar);
	}

	template <>
	inline bool BitStream::ReadCompressed(RakNetGUID &outTemplateVar)
	{
		return Read(outTemplateVar);
	}

	template <>
		inline bool BitStream::ReadCompressed(bool &outTemplateVar)
	{
		return Read(outTemplateVar);
	}

	/* 用于 -1 到 1 之间的值 */
	template <>
		inline bool BitStream::ReadCompressed(float &outTemplateVar)
	{
		unsigned short compressedFloat;
		if (Read(compressedFloat))
		{
			outTemplateVar = (static_cast<float>(compressedFloat) / 32767.5f - 1.0f);
			return true;
		}
		return false;
	}

	/* 用于 -1 到 1 之间的值 */
	template <>
		inline bool BitStream::ReadCompressed(double &outTemplateVar)
	{
		uint32_t compressedFloat;
		if (Read(compressedFloat))
		{
			outTemplateVar = (static_cast<double>(compressedFloat) / 2147483648.0 - 1.0);
			return true;
		}
		return false;
	}

	/* 用于字符串 */
	template <>
		inline bool BitStream::ReadCompressed(RakString &outTemplateVar)
	{
		return outTemplateVar.DeserializeCompressed(this,false);
	}
	template <>
	inline bool BitStream::ReadCompressed(RakWString &outTemplateVar)
	{
		return outTemplateVar.Deserialize(this);
	}
	template <>
	inline bool BitStream::ReadCompressed(char *&outTemplateVar)
	{
		return RakString::DeserializeCompressed(outTemplateVar,this,false);
	}
	template <>
	inline bool BitStream::ReadCompressed(wchar_t *&outTemplateVar)
	{
		return RakWString::Deserialize(outTemplateVar,this);
	}
	template <>
	inline bool BitStream::ReadCompressed(unsigned char *&outTemplateVar)
	{
		return RakString::DeserializeCompressed((char*) outTemplateVar,this,false);
	}

	/*
	 * 从位流中读取任意整数类型。
	 * 如果写入的值与写入函数中用于比较的值不同，
	 * var 将被更新。否则将保留当前值。
	 * 当前值将被更新。
	 * 对于浮点数，这是有损的，float 使用 2 字节，double 使用 4 字节。范围必须在 -1 到 +1 之间。
	 * 对于非浮点数，这是无损的，但仅在使用不到该类型一半位数时才有收益
	 * 如果未使用 __BITSTREAM_NATIVE_END，则对于大于 1 字节的类型，情况相反
	 * ReadCompressedDelta 仅在之前调用过 WriteDelta 后才有效
	 * 参数[输入] outTemplateVar The value to read
	 */
	template <class templateType>
		inline bool BitStream::ReadCompressedDelta(templateType &outTemplateVar)
	{
		bool dataWritten;
		bool success;
		success=Read(dataWritten);
		if (dataWritten)
			success=ReadCompressed(outTemplateVar);
		return success;
	}

	/*
	 * 从位流中读取一个布尔值。
	 * 参数[输入] outTemplateVar The value to read
	 */
	template <>
		inline bool BitStream::ReadCompressedDelta(bool &outTemplateVar)
	{
		return Read(outTemplateVar);
	}

	template <class destinationType, class sourceType >
	void BitStream::WriteCasted( const sourceType &value )
	{
		destinationType val = (destinationType) value;
		Write(val);
	}

	template <class templateType>
	void BitStream::WriteBitsFromIntegerRange( const templateType value, const templateType minimum,const templateType maximum, bool allowOutsideRange )
	{
		int requiredBits=BYTES_TO_BITS(sizeof(templateType))-NumberOfLeadingZeroes(templateType(maximum-minimum));
		WriteBitsFromIntegerRange(value,minimum,maximum,requiredBits,allowOutsideRange);
	}
	template <class templateType>
	void BitStream::WriteBitsFromIntegerRange( const templateType value, const templateType minimum,const templateType maximum, const int requiredBits, bool allowOutsideRange )
	{
		RakAssert(maximum>=minimum);
		RakAssert(allowOutsideRange==true || (value>=minimum && value<=maximum));
		if (allowOutsideRange)
		{
			if (value<minimum || value>maximum)
			{
				Write(true);
				Write(value);
				return;
			}
			Write(false);
		}
		templateType valueOffMin=value-minimum;
		if (IsBigEndian()==true)
		{
			unsigned char output[sizeof(templateType)];
			ReverseBytes((unsigned char*)&valueOffMin, output, sizeof(templateType));
			WriteBits(output,requiredBits);
		}
		else
		{
			WriteBits((unsigned char*) &valueOffMin,requiredBits);
		}
	}

	template <class templateType> /* 此函数的 templateType 必须为 float 或 double 类型 */
		void BitStream::WriteNormVector( templateType x, templateType y, templateType z )
	{
#ifdef _DEBUG
		RakAssert(x <= 1.01 && y <= 1.01 && z <= 1.01 && x >= -1.01 && y >= -1.01 && z >= -1.01);
#endif

		WriteFloat16(static_cast<float>(x),-1.0f,1.0f);
		WriteFloat16(static_cast<float>(y),-1.0f,1.0f);
		WriteFloat16(static_cast<float>(z),-1.0f,1.0f);
	}

	template <class templateType> /* 此函数的 templateType 必须为 float 或 double 类型 */
		void BitStream::WriteVector( templateType x, templateType y, templateType z )
	{
		templateType magnitude = sqrt(x * x + y * y + z * z);
		Write(static_cast<float>(magnitude));
		if (magnitude > 0.00001f)
		{
			WriteCompressed(static_cast<float>(x/magnitude));
			WriteCompressed(static_cast<float>(y/magnitude));
			WriteCompressed(static_cast<float>(z/magnitude));
			/* Write((unsigned short)((x/magnitude+1.0f)*32767.5f)); */
			/* Write((unsigned short)((y/magnitude+1.0f)*32767.5f)); */
			/* Write((unsigned short)((z/magnitude+1.0f)*32767.5f)); */
		}
	}

	template <class templateType> /* 此函数的 templateType 必须为 float 或 double 类型 */
		void BitStream::WriteNormQuat( templateType w, templateType x, templateType y, templateType z)
	{
		Write(static_cast<bool>(w<0.0));
		Write(static_cast<bool>(x<0.0));
		Write(static_cast<bool>(y<0.0));
		Write(static_cast<bool>(z<0.0));
		Write(static_cast<unsigned short>(fabs(x)*65535.0));
		Write(static_cast<unsigned short>(fabs(y)*65535.0));
		Write(static_cast<unsigned short>(fabs(z)*65535.0));
		/* 省略 w 并在目标端计算 */
	}

	template <class templateType> /* 此函数的 templateType 必须为 float 或 double 类型 */
		void BitStream::WriteOrthMatrix(
		templateType m00, templateType m01, templateType m02,
		templateType m10, templateType m11, templateType m12,
		templateType m20, templateType m21, templateType m22 )
	{

		double qw;
		double qx;
		double qy;
		double qz;

		/* 将矩阵转换为四元数 */
		/* http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/ */
		float sum;
		sum = 1 + m00 + m11 + m22;
		if (sum < 0.0f) sum=0.0f;
		qw = sqrt( sum  ) / 2;
		sum = 1 + m00 - m11 - m22;
		if (sum < 0.0f) sum=0.0f;
		qx = sqrt( sum  ) / 2;
		sum = 1 - m00 + m11 - m22;
		if (sum < 0.0f) sum=0.0f;
		qy = sqrt( sum  ) / 2;
		sum = 1 - m00 - m11 + m22;
		if (sum < 0.0f) sum=0.0f;
		qz = sqrt( sum  ) / 2;
		if (qw < 0.0) qw=0.0;
		if (qx < 0.0) qx=0.0;
		if (qy < 0.0) qy=0.0;
		if (qz < 0.0) qz=0.0;
		qx = _copysign( static_cast<double>(qx), static_cast<double>(m21 - m12) );
		qy = _copysign( static_cast<double>(qy), static_cast<double>(m02 - m20) );
		qz = _copysign( static_cast<double>(qz), static_cast<double>(m10 - m01) );

		WriteNormQuat(qw,qx,qy,qz);
	}

	template <class serializationType, class sourceType >
	bool BitStream::ReadCasted( sourceType &value )
	{
		serializationType val;
		bool success = Read(val);
		value=(sourceType) val;
		return success;
	}

	template <class templateType>
	bool BitStream::ReadBitsFromIntegerRange( templateType &value, const templateType minimum, const templateType maximum, bool allowOutsideRange )
	{
		int requiredBits=BYTES_TO_BITS(sizeof(templateType))-NumberOfLeadingZeroes(templateType(maximum-minimum));
		return ReadBitsFromIntegerRange(value,minimum,maximum,requiredBits,allowOutsideRange);
	}
	template <class templateType>
	bool BitStream::ReadBitsFromIntegerRange( templateType &value, const templateType minimum, const templateType maximum, const int requiredBits, bool allowOutsideRange )
	{
		RakAssert(maximum>=minimum);
		if (allowOutsideRange)
		{
			bool isOutsideRange;
			Read(isOutsideRange);
			if (isOutsideRange)
				return Read(value);
		}
		unsigned char output[sizeof(templateType)];
		memset(output,0,sizeof(output));
		bool success = ReadBits(output,requiredBits);
		if (success)
		{
			if (IsBigEndian()==true)
				ReverseBytesInPlace(output,sizeof(output));
			memcpy(&value,output,sizeof(output));

			value+=minimum;
		}

		return success;
	}

	template <class templateType> /* 此函数的 templateType 必须为 float 或 double 类型 */
		bool BitStream::ReadNormVector( templateType &x, templateType &y, templateType &z )
	{
		float xIn,yIn,zIn;
		ReadFloat16(xIn,-1.0f,1.0f);
		ReadFloat16(yIn,-1.0f,1.0f);
		ReadFloat16(zIn,-1.0f,1.0f);
		x=xIn;
		y=yIn;
		z=zIn;
		return true;
	}

	template <class templateType> /* 此函数的 templateType 必须为 float 或 double 类型 */
		bool BitStream::ReadVector( templateType &x, templateType &y, templateType &z )
	{
		float magnitude;
		/* unsigned short sx,sy,sz; */
		if (!Read(magnitude))
			return false;
		if (magnitude>0.00001f)
		{
			/* Read(sx); */
			/* Read(sy); */
			/* if (!Read(sz)) */
			/* 	return false; */
			/* x=((float)sx / 32767.5f - 1.0f) * magnitude; */
			/* y=((float)sy / 32767.5f - 1.0f) * magnitude; */
			/* z=((float)sz / 32767.5f - 1.0f) * magnitude; */
			float cx=0.0f,cy=0.0f,cz=0.0f;
			ReadCompressed(cx);
			ReadCompressed(cy);
			if (!ReadCompressed(cz))
				return false;
			x=cx;
			y=cy;
			z=cz;
			x*=magnitude;
			y*=magnitude;
			z*=magnitude;
		}
		else
		{
			x=0.0;
			y=0.0;
			z=0.0;
		}
		return true;
	}

	template <class templateType> /* 此函数的 templateType 必须为 float 或 double 类型 */
		bool BitStream::ReadNormQuat( templateType &w, templateType &x, templateType &y, templateType &z)
	{
		bool cwNeg=false, cxNeg=false, cyNeg=false, czNeg=false;
		unsigned short cx,cy,cz;
		Read(cwNeg);
		Read(cxNeg);
		Read(cyNeg);
		Read(czNeg);
		Read(cx);
		Read(cy);
		if (!Read(cz))
			return false;

		/* 根据 x、y、z 计算 w */
		x=(templateType)(cx/65535.0);
		y=(templateType)(cy/65535.0);
		z=(templateType)(cz/65535.0);
		if (cxNeg) x=-x;
		if (cyNeg) y=-y;
		if (czNeg) z=-z;
		float difference = 1.0f - x*x - y*y - z*z;
		if (difference < 0.0f)
			difference=0.0f;
		w = (templateType)(sqrt(difference));
		if (cwNeg)
			w=-w;

		return true;
	}

	template <class templateType> /* 此函数的 templateType 必须为 float 或 double 类型 */
		bool BitStream::ReadOrthMatrix(
		templateType &m00, templateType &m01, templateType &m02,
		templateType &m10, templateType &m11, templateType &m12,
		templateType &m20, templateType &m21, templateType &m22 )
	{
		float qw,qx,qy,qz;
		if (!ReadNormQuat(qw,qx,qy,qz))
			return false;

		/* 四元数转正交旋转矩阵 */
		/* http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm */
		double sqw = static_cast<double>(qw)*static_cast<double>(qw);
		double sqx = static_cast<double>(qx)*static_cast<double>(qx);
		double sqy = static_cast<double>(qy)*static_cast<double>(qy);
		double sqz = static_cast<double>(qz)*static_cast<double>(qz);
		m00 =  (templateType)(sqx - sqy - sqz + sqw); /* 因为 sqw + sqx + sqy + sqz = 1 */
		m11 = (templateType)(-sqx + sqy - sqz + sqw);
		m22 = (templateType)(-sqx - sqy + sqz + sqw);

		double tmp1 = static_cast<double>(qx)*static_cast<double>(qy);
		double tmp2 = static_cast<double>(qz)*static_cast<double>(qw);
		m10 = (templateType)(2.0 * (tmp1 + tmp2));
		m01 = (templateType)(2.0 * (tmp1 - tmp2));

		tmp1 = static_cast<double>(qx)*static_cast<double>(qz);
		tmp2 = static_cast<double>(qy)*static_cast<double>(qw);
		m20 =(templateType)(2.0 * (tmp1 - tmp2));
		m02 = (templateType)(2.0 * (tmp1 + tmp2));
		tmp1 = static_cast<double>(qy)*static_cast<double>(qz);
		tmp2 = static_cast<double>(qx)*static_cast<double>(qw);
		m21 = (templateType)(2.0 * (tmp1 + tmp2));
		m12 = (templateType)(2.0 * (tmp1 - tmp2));

		return true;
	}

	template <class templateType>
	BitStream& operator<<(BitStream& out, templateType& c)
	{
		out.Write(c);
		return out;
	}
	template <class templateType>
	BitStream& operator>>(BitStream& in, templateType& c)
	{
		bool success = in.Read(c);
		static_cast<void>(success);

		RakAssert(success);
		return in;
	}

}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif
