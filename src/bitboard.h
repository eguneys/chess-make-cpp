#ifndef Bitboard_h_Included
#define Bitboard_h_Included


#include <immintrin.h>
#define pext(b, m) _pext_u64(b, m)

#include <algorithm>
#include <cmath>


#include "types.h"


namespace Chess {

    constexpr Bitboard Bb_File_A = 0x0101010101010101ULL;
    constexpr Bitboard Bb_File_B = Bb_File_A << 1;
    constexpr Bitboard Bb_File_C = Bb_File_A << 2;
    constexpr Bitboard Bb_File_D = Bb_File_A << 3;
    constexpr Bitboard Bb_File_E = Bb_File_A << 4;
    constexpr Bitboard Bb_File_F = Bb_File_A << 5;
    constexpr Bitboard Bb_File_G = Bb_File_A << 6;
    constexpr Bitboard Bb_File_H = Bb_File_A << 7;

    constexpr Bitboard Bb_Rank_1 = 0xFF;
    constexpr Bitboard Bb_Rank_2 = Bb_Rank_1 << (8 * 1);
    constexpr Bitboard Bb_Rank_3 = Bb_Rank_1 << (8 * 2);
    constexpr Bitboard Bb_Rank_4 = Bb_Rank_1 << (8 * 3);
    constexpr Bitboard Bb_Rank_5 = Bb_Rank_1 << (8 * 4);
    constexpr Bitboard Bb_Rank_6 = Bb_Rank_1 << (8 * 5);
    constexpr Bitboard Bb_Rank_7 = Bb_Rank_1 << (8 * 6);
    constexpr Bitboard Bb_Rank_8 = Bb_Rank_1 << (8 * 7);


    struct Magic {
        Bitboard mask;
        Bitboard* attacks;
        unsigned index(Bitboard occupied) const {
            return unsigned(pext(occupied, mask));
        }

        Bitboard attacks_bb(Bitboard occupied) const { return attacks[index(occupied)]; }
    };

    const Bitboard square_bb(Square s) {
        return (1ULL << s);
    }

    inline Bitboard operator&(Bitboard b, Square s) { return b & square_bb(s); }
    inline Bitboard operator|(Bitboard b, Square s) { return b | square_bb(s); }
    inline Bitboard operator^(Bitboard b, Square s) { return b ^ square_bb(s); }
    inline Bitboard &operator|=(Bitboard &b, Square s) { return b |= square_bb(s); }
    inline Bitboard &operator^=(Bitboard &b, Square s) { return b ^= square_bb(s); }

    inline Bitboard operator&(Square s, Bitboard b) { return b & s; }
    inline Bitboard operator|(Square s, Bitboard b) { return b | s; }
    inline Bitboard operator^(Square s, Bitboard b) { return b ^ s; }

    inline Bitboard operator|(Square s1, Square s2) { return square_bb(s1) | s2; }

    constexpr bool more_than_one(Bitboard b) { return b & (b - 1); }


    constexpr Bitboard rank_bb(Rank r) { return Bb_Rank_1 << (8 * r); }

    constexpr Bitboard rank_bb(Square s) { return rank_bb(rank_of(s)); }

    constexpr Bitboard file_bb(File f) { return Bb_File_A << f; }

    constexpr Bitboard file_bb(Square s) { return file_bb(file_of(s)); }


    u8 SquareDistance[64][64];

    Bitboard BetweenBB[64][64];
    Bitboard LineBB[64][64];
    Bitboard Attacks[64][64];
    Bitboard PawnAttacks[2][64];


    template <Direction D>
    constexpr Bitboard shift(Bitboard b)
    {
        return D == Up            ? b << 8
               : D == Down        ? b >> 8
               : D == Up + Up     ? b << 16
               : D == Down + Down ? b >> 16
               : D == Right       ? (b & ~FileHBB) << 1
               : D == Left        ? (b & ~FileABB) >> 1
               : D == UpRight     ? (b & ~FileHBB) << 9
               : D == UpLeft      ? (b & ~FileABB) << 7
               : D == DownRight   ? (b & ~FileHBB) >> 7
               : D == DownLeft    ? (b & ~FileABB) >> 9
                                  : 0;
    }

    template <Color C>
    constexpr Bitboard pawn_attacks_bb(Bitboard b)
    {
        return C == White ? shift<UpLeft>(b) | shift<UpRight>(b)
                          : shift<DownLeft>(b) | shift<DownRight>(b);
    }

    inline Bitboard pawn_attacks_bb(Color c, Square s)
    {
        return PawnAttacks[c][s];
    }


    inline Bitboard line_bb(Square s1, Square s2) {
        return LineBB[s1][s2];
    }

    inline Bitboard between_bb(Square s1, Square s2) {
        return BetweenBB[s1][s2];
    }

    inline bool aligned(Square s1, Square s2, Square s3) { return line_bb(s1, s2) * s3; }

    template<typename T = Square>
    inline int distance(Square x, Square y);

    template<>
    inline int distance<File>(Square x, Square y) {
        return std::abs(file_of(x) - file_of(y));
    }

    template<>
    inline int distance<Rank>(Square x, Square y) {
        return std::abs(rank_of(x) - rank_of(y));
    }

    template<>
    inline int distance<Square>(Square x, Square y) {
        return SquareDistance[x][y];
    }

    inline int edge_distance(File f) { return std::min(f, File(File_H - f)); }


    template<PieceType T>
    inline Bitboard attacks_bb(Square s) {
        return Attacks[T][s];
    }

    template <PieceType T>
    inline Bitboard attacks_bb(Square s, Bitboard occupied) {
        switch (T) {
            case Bishop:
            case Rook:
            return Magics[s][T - Bishop].attacks_bb(occupied);
            case Queen:
            return attacks_bb<Bishop>(s, occupied) | attacks_bb<Rook>(s, occupied);
            default:
            return Attacks[T][s];
        }
    }


    inline int popcount(Bitboard b) {
        //return int(_mm_popcnt_u64(b));
        return __builtin_popcountll(b);
    }

    inline Square lsb(Bitboard b) {
        return Square(__builtin_ctzll(b));
    }

    inline Square msb(Bitboard b) {
        return Square(63 ^ __builtin_clzll(b));
    }


    inline Square pop_lsb(Bitboard& b) {
        const Square s = lsb(b);
        b &= b - 1;
        return s;
    }

}

#endif