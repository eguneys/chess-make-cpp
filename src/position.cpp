
#include<cstring>
#include <sstream>

#include "position.h"

#include "bitboard.h"

using std::string;

namespace Chess {


    constexpr std::string_view PieceToChar(" PNBRQK  pnbrqk");

    Position& Position::set(const string& FEN) {

        unsigned char col, row, token;
        u8 idx;
        Square sq = A8;

        std::istringstream ss(FEN);

        std::memset(this, 0, sizeof(Position));

        ss >> std::noskipws;

        while ((ss >> token) && !isspace(token)) {
            if (isdigit(token)) {
                sq += (Direction)((token - '0') * Right);
            } else if (token == '/') {
                sq += (Direction)(2 * Down);
            } else if ((idx = PieceToChar.find(token)) != string::npos) {
                put_piece(Piece(idx), sq);
                ++sq;
            }
        }

        ss >> token;
        _side_to_move = (token == 'w' ? White : Black);

        ss >> token;

        return *this;
    }
}