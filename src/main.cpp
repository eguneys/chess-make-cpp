#include <iostream>
#include "bitboard.h"

int main() {
    std::cout << "Hello" << std::endl;

    Chess::Bitboards::init();

    std::cout << Chess::Bitboards::pretty(Chess::square_bb(Chess::H8));
    
    Chess::Bitboard occ = Chess::attacks_bb(Chess::Bishop, Chess::D1, 0);

    occ = Chess::attacks_bb<Chess::Queen>(Chess::D4, Chess::square_bb(Chess::D5));

    std::cout << Chess::Bitboards::pretty(occ);

    return 0;
}