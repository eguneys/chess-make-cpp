#include "matcher.h"
#include <iostream>

namespace Chess
{

    void BitsetManager::begin_first_pass() {
        nb_knights = 0;
        nb_bishops = 0;
    }

    void BitsetManager::push(const Position& p) {
        nb_knights += popcount(p.pieces(Knight) & p.pieces(~p.side_to_move()));
        nb_bishops += popcount(p.pieces(Bishop) & p.pieces(~p.side_to_move()));
    }


    void BitsetManager::end_first_pass() {
        knight_only_defended_by_bishop = Bitset(nb_knights);
        knights.reserve(nb_knights);
    }


    void BitsetManager::process_bitset_i(const Position&p, const u64 index) {

    }

}