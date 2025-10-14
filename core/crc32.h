#include <stdint.h>

#if defined(__GNUC__)
	#include <x86intrin.h>
#else
	#include <intrin.h>
#endif

uint64_t sse42_crc32(const uint64_t *buffer, size_t len) {
    const uint8_t* p = static_cast<const uint8_t*>(data);

    uint32_t crcA = 0xFFFFFFFFu;        // conventional seed
    uint32_t crcB = 0xDEADBEEFu;        // second, independent seed

    // Process 8-byte chunks safely
    while (len >= 8) {
        uint64_t v;
        std::memcpy(&v, p, 8);          // avoids alignment UB
        crcA = static_cast<uint32_t>(_mm_crc32_u64(crcA, v));
        crcB = static_cast<uint32_t>(_mm_crc32_u64(crcB, ~v)); // diversify stream
        p   += 8;
        len -= 8;
    }
    // Tail bytes
    while (len--) {
        uint8_t b = *p++;
        crcA = _mm_crc32_u8(crcA, b);
        crcB = _mm_crc32_u8(crcB, ~b);
    }

    crcA = ~crcA;                        // conventional final XOR
    crcB = ~crcB;

    // Merge two 32-bit CRCs into one 64-bit fingerprint
    return (static_cast<uint64_t>(crcA) << 32) | crcB;
}
