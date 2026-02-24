#include <iostream>
#include <string_view>

#include "bitboard.h"
#include "position.h"
#include "test.h"
#include "matcher.h"

int main() {
    std::cout << "Hello" << std::endl;

    Chess::Bitboards::init();

    Chess::BitsetManager res;

    Test::LichessDbPuzzle db;

    db.open_and_build_index();

    std::cout << "First Pass" << std::endl;

    db.pass_FEN([](const std::string_view FEN, const u64 index) {
    });

    std::cout << "Second Pass" << std::endl;
    db.pass_FEN([](const std::string_view FEN, const u64 index) {
    });

    std::cout << db.get_full(0) << std::endl;
    std::cout << db.get_full(10) << std::endl;
    std::cout << db.get_full(11) << std::endl;

    std::cout << " Done.\n";

    return 0;
}



