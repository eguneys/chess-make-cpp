#include <iostream>
#include <string_view>

#include "bitboard.h"
#include "position.h"
#include "test.h"

int main() {
    std::cout << "Hello" << std::endl;

    Chess::Bitboards::init();

    //Chess::Position p;
    //p.set("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    //Chess::Piece pp = p.piece_on(Chess::E8);
    //std::cout << pp;
    //std::cout << Chess::Bitboards::pretty(p.pieces(Chess::Bishop));


    Chess::Position p;

    Test::test([](std::string_view value) {

        std::string SS = std::string{value};

    });

    std::cout << "done\n";

    return 0;
}



