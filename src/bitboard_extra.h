#pragma once

#include "bitboard.h"
#include "position.h"
#include "types.h"

namespace Chess
{

    inline Bitboard pawn_attacks(Color c, Square sq) {
        return pawn_attacks_bb(c, sq);
    }

    inline Bitboard knight_attacks(Square sq) {
        return attacks_bb<Knight>(sq);
    }
    inline Bitboard king_attacks(Square sq) {
        return attacks_bb<King>(sq);
    }


    inline Bitboard bishop_attacks(Square sq, Bitboard occupied) {
        return attacks_bb<Bishop>(sq, occupied);
    }
    inline Bitboard rook_attacks(Square sq, Bitboard occupied) {
        return attacks_bb<Rook>(sq, occupied);
    }
    inline Bitboard queen_attacks(Square sq, Bitboard occupied) {
        return attacks_bb<Queen>(sq, occupied);
    }






    inline Bitboard attackers_to(
        const Position &p,
        Square sq,
        Color c)
    {
        Bitboard occ = p.pieces();

        Bitboard attackers = 0;

        attackers |= pawn_attacks(~c, sq) & p.pieces(Pawn) & p.pieces(c);
        attackers |= knight_attacks(sq) & p.pieces(Knight) & p.pieces(c);
        attackers |= bishop_attacks(sq, occ) & p.pieces(Bishop) & p.pieces(c);
        attackers |= rook_attacks(sq, occ) & p.pieces(Rook) & p.pieces(c);
        attackers |= queen_attacks(sq, occ) & p.pieces(Queen) & p.pieces(c);
        attackers |= king_attacks(sq) & p.pieces(King) & p.pieces(c);

        return attackers;
    }
}