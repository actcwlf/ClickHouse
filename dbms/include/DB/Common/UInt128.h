#pragma once

#include <DB/Common/HashTable/Hash.h>
#include <DB/IO/ReadHelpers.h>
#include <DB/IO/WriteHelpers.h>


namespace DB
{

/// Для агрегации по SipHash или конкатенации нескольких полей.
struct UInt128
{
	UInt64 first;
	UInt64 second;

	bool operator== (const UInt128 rhs) const { return first == rhs.first && second == rhs.second; }
	bool operator!= (const UInt128 rhs) const { return first != rhs.first || second != rhs.second; }

	bool operator== (const UInt64 rhs) const { return first == rhs && second == 0; }
	bool operator!= (const UInt64 rhs) const { return first != rhs || second != 0; }

	UInt128 & operator= (const UInt64 rhs) { first = rhs; second = 0; return *this; }
};

struct UInt128Hash
{
	DefaultHash<UInt64> hash64;
	size_t operator()(UInt128 x) const { return hash64(hash64(x.first) ^ x.second); }
};

struct UInt128HashCRC32
{
	size_t operator()(UInt128 x) const
	{
		UInt64 crc = -1ULL;
		asm("crc32q %[x], %[crc]\n" : [crc] "+r" (crc) : [x] "rm" (x.first));
		asm("crc32q %[x], %[crc]\n" : [crc] "+r" (crc) : [x] "rm" (x.second));
		return crc;
	}
};

struct UInt128TrivialHash
{
	size_t operator()(UInt128 x) const { return x.first; }
};

inline void readBinary(UInt128 & x, ReadBuffer & buf) { readPODBinary(x, buf); }
inline void writeBinary(const UInt128 & x, WriteBuffer & buf) { writePODBinary(x, buf); }


/** Используется при агрегации, для укладки большого количества ключей постоянной длины в хэш-таблицу.
  */
struct UInt256
{
	UInt64 a;
	UInt64 b;
	UInt64 c;
	UInt64 d;

	bool operator== (const UInt256 rhs) const
	{
		/// Замечание: не проверено, что так лучше.
		return 0xFFFF == _mm_movemask_epi8(_mm_and_si128(
			_mm_cmpeq_epi8(
				_mm_loadu_si128(reinterpret_cast<const __m128i *>(&a)),
				_mm_loadu_si128(reinterpret_cast<const __m128i *>(&rhs.a))),
			_mm_cmpeq_epi8(
				_mm_loadu_si128(reinterpret_cast<const __m128i *>(&c)),
				_mm_loadu_si128(reinterpret_cast<const __m128i *>(&rhs.c)))));
	}

	bool operator!= (const UInt256 rhs) const { return !operator==(rhs); }

	bool operator== (const UInt64 rhs) const { return a == rhs && b == 0 && c == 0 && d == 0; }
	bool operator!= (const UInt64 rhs) const { return !operator==(rhs); }

	UInt256 & operator= (const UInt64 rhs) { a = rhs; b = 0; c = 0; d = 0; return *this; }
};

struct UInt256HashCRC32
{
	size_t operator()(UInt256 x) const
	{
		UInt64 crc = -1ULL;
		asm("crc32q %[x], %[crc]\n" : [crc] "+r" (crc) : [x] "rm" (x.a));
		asm("crc32q %[x], %[crc]\n" : [crc] "+r" (crc) : [x] "rm" (x.b));
		asm("crc32q %[x], %[crc]\n" : [crc] "+r" (crc) : [x] "rm" (x.c));
		asm("crc32q %[x], %[crc]\n" : [crc] "+r" (crc) : [x] "rm" (x.d));
		return crc;
	}
};

inline void readBinary(UInt256 & x, ReadBuffer & buf) { readPODBinary(x, buf); }
inline void writeBinary(const UInt256 & x, WriteBuffer & buf) { writePODBinary(x, buf); }

}
