#pragma once

#include <cstdint>

using i64 = std::int64_t;
using u64 = std::uint64_t;
using i32 = std::int32_t;
using u16 = std::uint16_t;
using i8 = std::int8_t;
using u8 = std::uint8_t;

namespace Chess {


    using Bitboard = u64;

    enum Color {
        White,
        Black,
        Color_NB = 2
    };

    enum PieceType {
        No_Piece_Type,
        Pawn,
        Knight,
        Bishop,
        Rook,
        Queen,
        King,
        All_Pieces = 0,
        Piece_Type_NB = 8,
    };

    enum Piece {
        No_Piece,
        White_Pawn = Pawn,
        White_Knight,
        White_Bishop,
        White_Rook,
        White_Queen,
        White_King,
        Black_Pawn = Pawn + 8,
        Black_Knight,
        Black_Bishop,
        Black_Rook,
        Black_Queen,
        Black_King,
        Piece_NB = 16,
    };

    enum Square: int {
        A1, B1, C1, D1, E1, F1, G1, H1,
        A2, B2, C2, D2, E2, F2, G2, H2,
        A3, B3, C3, D3, E3, F3, G3, H3,
        A4, B4, C4, D4, E4, F4, G4, H4,
        A5, B5, C5, D5, E5, F5, G5, H5,
        A6, B6, C6, D6, E6, F6, G6, H6,
        A7, B7, C7, D7, E7, F7, G7, H7,
        A8, B8, C8, D8, E8, F8, G8, H8,
        Sq_None,
        Square_Zero = 0,
        Square_NB = 64
    };


    enum Direction: int {
        Up = 8,
        Right = 1,
        Down = -8,
        Left = -1,

        UpRight = Up + Right,
        UpLeft = Up + Left,
        DownRight = Down + Right,
        DownLeft = Down + Left,
    };

    enum File {
        File_A, File_B, File_C, File_D, File_E, File_F, File_G, File_H, File_Nb
    };

    enum Rank {
        Rank_1, Rank_2, Rank_3, Rank_4, Rank_5, Rank_6, Rank_7, Rank_8, Rank_Nb
    };

#define ENABLE_INC_OPERATORS_ON(T)                           \
    inline T &operator++(T &d) { return d = T(int(d) + 1); } \
    inline T &operator--(T &d) { return d = T(int(d) - 1); }

    ENABLE_INC_OPERATORS_ON(PieceType)
    ENABLE_INC_OPERATORS_ON(Square)
    ENABLE_INC_OPERATORS_ON(File)
    ENABLE_INC_OPERATORS_ON(Rank)

#undef ENABLE_INC_OPERATORS_ON

    constexpr Square operator+(Square s, Direction d) { return Square(int(s) + int(d)); }
    constexpr Square operator-(Square s, Direction d) { return Square(int(s) - int(d)); }
    inline Square& operator+=(Square&s, Direction d) { return s = s + d; }
    inline Square& operator-=(Square&s, Direction d) { return s = s - d; }

    constexpr Color operator~(Color c) { return Color(c ^ 1); }

    constexpr Square make_square(File f, Rank r) { return Square((r << 3) + f); }

    constexpr Piece make_piece(Color c, PieceType t) { return Piece((c << 3) + t); }

    constexpr PieceType typeof_piece(Piece p) { return PieceType(p & 7); }

    inline Color color_of(Piece pc) {
        return Color(pc >> 3);
    }

    constexpr bool is_ok(Square s) { return s >= A1 && s <= H8; }

    constexpr File file_of(Square s) { return File(int(s) & 7); }
    constexpr Rank rank_of(Square s) { return Rank(s >> 3); }

    constexpr Direction pawn_push(Color c) { return c == White ? Up : Down; }

    
}