#include <iostream>

#include "moves.h"

namespace Chess {


    Move Move::parse_uci(std::string &uci)
    {
        assert(uci.size() == 4 || uci.size() == 5);


        const char *ptr = uci.data();

        const char c_from_file = *ptr++;
        const char c_from_rank = *ptr++;
        const char c_to_file = *ptr++;
        const char c_to_rank = *ptr++;

        assert(c_from_rank >= '1' && c_from_rank <= '8');
        assert(c_to_rank >= '1' && c_to_rank <= '8');

        Rank from_rank = (Rank)(c_from_rank - '1');
        Rank to_rank = (Rank)(c_to_rank - '1');

        assert(c_from_file >= 'a' && c_from_file <= 'h');
        assert(c_to_file >= 'a' && c_to_file <= 'h');

        File from_file = (File)(c_from_file - 'a');
        File to_file = (File)(c_to_file - 'a');

        Square from = make_square(from_file, from_rank);
        Square to = make_square(to_file, to_rank);

        return Move(from, to);
    }
}