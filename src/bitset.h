#include <vector>
#include <cstdint>
#include <cassert>
#include <immintrin.h>

namespace Chess {
class Bitset {
public:

    Bitset() = default;
    explicit Bitset(size_t bits)
        : nbits(bits),
          w((bits + 63) >> 6, 0ULL) {}

    size_t size() const { return nbits; }


    bool test(size_t i) const {
        assert(i < nbits);
        return (w[i>>6] >> (i & 63)) & 1ULL;
    }

    inline void set(size_t i)
    {
        assert(i < nbits);
        w[i >> 6] |= (1ULL << (i & 63));
    }

    inline void reset(size_t i) {
        assert(i < nbits);
        w[i >> 6] &= ~(1ULL << (i & 63));
    }

    void clear() {
        std::fill(w.begin(), w.end(), 0ULL);
    }

    friend Bitset operator&(const Bitset &a, const Bitset &b);
    friend Bitset& operator&=(Bitset& a, const Bitset& b);

    private:
    size_t nbits = 0;
    std::vector<uint64_t> w;
};

Bitset operator&(const Bitset &a, const Bitset &b);
Bitset &operator&=(Bitset &a, const Bitset &b);
}