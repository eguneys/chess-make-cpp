#include <iostream>
#include <string_view>

#include "bitboard.h"
#include "position.h"
#include "test.h"
#include "matcher.h"
#include "moves.h"

int main() {
    std::cout << "Hello" << std::endl;

    Chess::Bitboards::init();

    Chess::BitsetManager res;
    Test::LichessDbPuzzle db;

    //db.open_and_build_index("../data/single_out.csv");
    //db.open_and_build_index("../data/athousand_sorted.csv");
    db.open_and_build_index("../data/lichess_db_puzzle.csv");

    std::cout << "First Pass" << std::endl;

    res.begin_first_pass();

    std::string buffer;
    buffer.reserve(256);
    Chess::Position p;

    int total = 0;

    db.pass_FEN_and_first_UCI([&p, &res, &buffer, &total](const std::string_view FEN, const std::string_view UCI, const u64 index) {
        total++;
        buffer.assign(FEN);
        p.set(buffer);
        buffer.assign(UCI);
        p.make_move(Chess::Move::parse_uci(buffer));
        res.push_position_first_pass(p, index);
    });

    res.end_first_pass();

    std::cout << "Second Pass" << std::endl;
    db.pass_FEN_and_first_UCI([&res, &p, &buffer](const std::string_view FEN, const std::string_view UCI, const u64 index) {
        buffer.assign(FEN);
        p.set(buffer);
        buffer.assign(UCI);
        p.make_move(Chess::Move::parse_uci(buffer));
        res.process_position_second_pass(p, index);
    });

    int i = 0;
    res.full_query([&i, &db](u64 position_id)
                   { 
      i++;
      if (i > 16) {
        return;
      }
      Test::LichessPuzzle puzzle = db.get_full(position_id);
      std::cout << position_id << ":> " << puzzle.link << std::endl; });

    int percent = abs(((float)i / total) * 100);
    std::cout << "Total found: %" << percent << "[" << i << "/" << total << "] Done.\n";

    return 0;
}



