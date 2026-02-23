#include "bitboard.h"
#include <string>

namespace Chess {


    namespace {
        Bitboard RookTable[0x19000];
        Bitboard BishopTable[0x1480];

        void init_magics(PieceType t, Bitboard table[], Magic magics[][2]);

        Bitboard safe_destination(Square s, int step) {
            Square to = Square(s + step);
            return is_ok(to) && distance(s, to) <= 2 ? square_bb(to): Bitboard(0);
        }

        std::string pretty(Bitboard b)
        {

            std::string s = "+---+---+---+---+---+---+---+---+\n";

            for (Rank r = Rank_8; r >= Rank_1; --r)
            {
                for (File f = File_A; f <= File_H; ++f)
                    s += b & make_square(f, r) ? "| X " : "|   ";

                s += "| " + std::to_string(1 + r) + "\n+---+---+---+---+---+---+---+---+\n";
            }
            s += "  a   b   c   d   e   f   g   h\n";

            return s;
        }
    }

}