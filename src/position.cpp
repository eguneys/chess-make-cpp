
#include<cstring>
#include <sstream>

#include "position.h"

#include "bitboard.h"

using std::string;

namespace Chess {

    constexpr Piece char_to_piece(char c)
    {
        switch (c)
        {
        case 'P':
            return White_Pawn;
        case 'N':
            return White_Knight;
        case 'B':
            return White_Bishop;
        case 'R':
            return White_Rook;
        case 'Q':
            return White_Queen;
        case 'K':
            return White_King;
        case 'p':
            return Black_Pawn;
        case 'n':
            return Black_Knight;
        case 'b':
            return Black_Bishop;
        case 'r':
            return Black_Rook;
        case 'q':
            return Black_Queen;
        case 'k':
            return Black_King;
        default:
           return No_Piece;
        }
    }

    Position &Position::set(const std::string &FEN)
    {

        std::memset(this, 0, sizeof(Position));

        const char *ptr = FEN.data();
        const char *const end = ptr + FEN.size();

        Square sq = A8;

        // Fast path: parse until space
        while (ptr < end)
        {
            const char c = *ptr;
            if (c == ' ')
                break;
            ++ptr;

            if (c >= '1' && c <= '8')
            {
                sq = (Square)((int)sq + (c - '0'));
            }
            else if (c == '/')
            {
                sq = (Square)((int)sq + (2 * Down));
            }
            else
            {
                const Piece piece = char_to_piece(static_cast<unsigned char>(c));
                if (piece != No_Piece)
                {
                    put_piece(piece, sq);
                    ++sq;
                }
            }
        }

        // Skip spaces and parse side to move
        while (ptr < end && *ptr == ' ') {
            ++ptr;
        }
        if (ptr < end)
        {
            _side_to_move = (*ptr == 'w') ? White : Black;
            ++ptr;
        }

        return *this;
    }


    void Position::make_move(Move move) {
        Square from = move.from_sq();
        Square to = move.to_sq();

        move_piece(from, to);

        _side_to_move = ~_side_to_move;
    }
}