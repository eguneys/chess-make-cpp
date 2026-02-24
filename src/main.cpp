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

    //db.open_and_build_index("../data/single_out.csv");
    db.open_and_build_index("../data/athousand_sorted.csv");
    //db.open_and_build_index("../data/single_out.csv");

    std::cout << "First Pass" << std::endl;

    res.begin_first_pass();

    std::string buffer;
    buffer.reserve(256);
    Chess::Position p;

    db.pass_FEN([&p, &res, &buffer](const std::string_view FEN, const u64 index) {
        buffer.assign(FEN);
        p.set(buffer);
        res.push_position_first_pass(p, index);
    });

    res.end_first_pass();

    std::cout << "Second Pass" << std::endl;
    db.pass_FEN([&res, &p, &buffer](const std::string_view FEN, const u64 index) {
        buffer.assign(FEN);
        p.set(buffer);
        res.process_position_second_pass(p, index);
    });

    int i = 0;
    res.full_query([&i, &db](u64 position_id)
                   { 
        i++;
        if (i > 8) {
            return;
        }
                    std::cout << position_id << " > " << db.get_full(position_id) << std::endl; });

    std::cout << "Total found: " << i << " Done.\n";

    return 0;
}



