#pragma once

#include "position.h"
#include "bitset.h"
#include "types.h"

namespace Chess {

    struct KnightInstance {
        u64 position_index;
        Square square;
        Color color;
    };

    class BitsetManager {

        public:

        void begin_first_pass();

        void push(const Position& p);

        void end_first_pass();

        void process_bitset_i(const Position&p, const u64 index);

        private:
        u64 nb_knights;
        u64 nb_bishops;

        Bitset knight_only_defended_by_bishop;

        std::vector<KnightInstance> knights;

    };
}