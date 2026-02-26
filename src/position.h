#pragma once

#include <string>

#include "types.h"
#include "bitboard.h"
#include "moves.h"


namespace Chess {

    class Position {

        public:
        Position() = default;
        Position(Position&) = delete;
        Position& operator=(const Position&) = delete;

        Position& set(const std::string& FEN);


        Bitboard pieces(PieceType pt = All_Pieces) const;
        Bitboard pieces(Color c) const;
        Piece piece_on(Square s) const;
        Color color_on(Square s) const;
        bool empty(Square s) const;

        void put_piece(Piece pc, Square s);
        void remove_piece(Square s);
        void swap_piece(Square s, Piece pc);

        Color side_to_move() const;

        void make_move(Move move);

        private:
        void move_piece(Square from, Square to);

        Piece _pieces[Square_NB];
        Bitboard by_Type_BB[Piece_Type_NB];
        Bitboard by_Color_BB[Color_NB];
        Color _side_to_move;
    };

    inline Color Position::side_to_move() const { return _side_to_move; }

    inline Piece Position::piece_on(Square s) const {
        assert(is_ok(s));
        return _pieces[s];
    }

    inline Color Position::color_on(Square s) const {
        return color_of(piece_on(s));
    }

    inline bool Position::empty(Square s) const { return piece_on(s) == No_Piece; }

    inline Bitboard Position::pieces(PieceType pt) const { return by_Type_BB[pt]; }

    inline Bitboard Position::pieces(Color c) const { return by_Color_BB[c]; }


    inline void Position::put_piece(Piece pc, Square s) {
        _pieces[s] = pc;
        by_Type_BB[All_Pieces] |= by_Type_BB[typeof_piece(pc)] |= s;
        by_Color_BB[color_of(pc)] |= s;
    }

    inline void Position::remove_piece(Square s) {
        Piece pc = _pieces[s];
        by_Type_BB[All_Pieces] ^= s;
        by_Type_BB[typeof_piece(pc)] ^= s;
        by_Color_BB[color_of(pc)] ^= s;
        _pieces[s] = No_Piece;
    }

    inline void Position::move_piece(Square from, Square to) {
        Piece pc = _pieces[from];
        Bitboard fromTo = from | to;

        by_Type_BB[All_Pieces] ^= fromTo;
        by_Type_BB[typeof_piece(pc)] ^= fromTo;
        by_Color_BB[color_of(pc)] ^= fromTo;
        _pieces[from] = No_Piece;
        _pieces[to] = pc;
    }

    inline void Position::swap_piece(Square s, Piece pc) {
        remove_piece(s);
        put_piece(pc, s);
    }

}