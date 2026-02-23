#include <string>

#include "bitboard.h"

namespace Chess
{


    u8 SquareDistance[Square_NB][Square_NB];

    Bitboard LineBB[Square_NB][Square_NB];
    Bitboard BetweenBB[Square_NB][Square_NB];
    Bitboard PseudoAttacks[Piece_Type_NB][Square_NB];
    Bitboard PawnAttacks[Color_NB][Square_NB];

    alignas(64) Magic Magics[Square_NB][2];


    namespace {
        Bitboard RookTable[0x19000];
        Bitboard BishopTable[0x1480];

        void init_magics(PieceType t, Bitboard table[], Magic magics[][2]);

        Bitboard safe_destination(Square s, int step) {
            Square to = Square(s + step);
            return is_ok(to) && distance(s, to) <= 2 ? square_bb(to): Bitboard(0);
        }
    }



    // Returns an ASCII representation of a bitboard suitable
    // to be printed to standard output. Useful for debugging.
    std::string Bitboards::pretty(Bitboard b)
    {

        std::string s = "\n";

        for (Rank r = Rank_8; r >= Rank_1; --r)
        {
            for (File f = File_A; f <= File_H; ++f)
                s += b & make_square(f, r) ? "o" : ".";

            s += "\n";
        }

        return s;
    }


    void Bitboards::init() {

        for (Square s1 = A1; s1 <= H8; ++s1)
            for (Square s2 = A1; s2 <= H8; ++s2)
                SquareDistance[s1][s2] = std::max(distance<File>(s1, s2), distance<Rank>(s1, s2));


        init_magics(Rook, RookTable, Magics);
        init_magics(Bishop, BishopTable, Magics);

        for (Square s1 = A1; s1 <= H8; ++s1) {
            PawnAttacks[White][s1] = pawn_attacks_bb<White>(square_bb(s1));
            PawnAttacks[Black][s1] = pawn_attacks_bb<Black>(square_bb(s1));


            for (int step: { -9, -8, -7, -1, 1, 7, 8, 9})
                PseudoAttacks[King][s1] |= safe_destination(s1, step);

            for (int step: { -17, -15, -10, -6, 6, 10, 15, 17})
                PseudoAttacks[Knight][s1] |= safe_destination(s1, step);
            

            PseudoAttacks[Queen][s1] = PseudoAttacks[Bishop][s1] = attacks_bb<Bishop>(s1, 0);
            PseudoAttacks[Queen][s1] |= PseudoAttacks[Rook][s1] = attacks_bb<Rook>(s1, 0);

            for (PieceType pt : { Bishop, Rook })
            for (Square s2 = A1; s2 <= H8; ++s2) {
                if (PseudoAttacks[pt][s1] & s2) {
                    LineBB[s1][s2] = (attacks_bb(pt, s1, 0) & attacks_bb(pt, s2, 0)) | s1 | s2;
                    BetweenBB[s1][s2] = (attacks_bb(pt, s1, square_bb(s2)) & attacks_bb(pt, s2, square_bb(s1)));
                }
                BetweenBB[s1][s2] |= s2;
            }


        }
    }


    namespace {

        Bitboard sliding_attack(PieceType pt, Square sq, Bitboard occupied) {
            Bitboard attacks = 0;
            Direction RookDirections[4] = { Up, Down, Left, Right };
            Direction BishopDirections[4] = { UpRight, UpLeft, DownRight, DownLeft };

            for (Direction d: (pt == Rook ? RookDirections : BishopDirections)) {
                Square s = sq;

                while (safe_destination(s, d)) {
                    attacks |= (s += d);
                    if (occupied & s) {
                        break;
                    }
                }
            }

            return attacks;
                
        }

        void init_magics(PieceType pt, Bitboard table[], Magic magics[][2])
        {
            Bitboard reference[4096];
            int size = 0;

            for (Square s = A1; s <= H8; ++s)
            {
                Bitboard edges = ((Bb_Rank_1 | Bb_Rank_8) & ~rank_bb(s)) | ((Bb_File_A | Bb_File_H) & ~file_bb(s));

                Magic &m = magics[s][pt - Bishop];
                m.mask = sliding_attack(pt, s, 0) & ~edges;

                m.attacks = s == A1 ? table : magics[s - 1][pt - Bishop].attacks + size;
                size = 0;

                Bitboard b = 0;

                do
                {

                    reference[size] = sliding_attack(pt, s, b);
                    m.attacks[pext(b, m.mask)] = reference[size];

                    size++;
                    b = (b - m.mask) & m.mask;
                } while (b);
            }
        }
}

}
