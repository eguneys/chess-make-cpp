#include "bitset.h"

namespace Chess {

    inline void bitset_and_avx2(
        uint64_t *__restrict dst, 
        const uint64_t *__restrict a, 
        const uint64_t *__restrict b, 
        size_t n_words) {
        size_t i = 0;

        constexpr size_t STRIDE = 4; // 4 uint64 = 256 bits

        size_t limit = n_words & ~(STRIDE - 1);

        for (; i < limit; i += STRIDE)
        {
            __m256i va = _mm256_loadu_si256((__m256i const *)(a + i));
            __m256i vb = _mm256_loadu_si256((__m256i const *)(b + i));
            __m256i vc = _mm256_and_si256(va, vb);
            _mm256_storeu_si256((__m256i *)(dst + i), vc);
        }

        // Scalar tail
        for (; i < n_words; ++i)
            dst[i] = a[i] & b[i];
    }

    inline Bitset operator&(const Bitset &a, const Bitset &b)
    {
        Bitset out;
        out.w.resize(a.w.size());

        bitset_and_avx2(
            out.w.data(),
            a.w.data(),
            b.w.data(),
            a.w.size());

        return out;
    }

    /*

    Faster
    Bitset result = knight_only_defended_by_bishop;
    result &= knight_attacked_by_pawn;
    result &= side_to_move_white;
    */
    inline Bitset& operator&=(Bitset& a, const Bitset& b) {
    bitset_and_avx2(
        a.w.data(),
        a.w.data(),
        b.w.data(),
        a.w.size()
    );
    return a;
}
}