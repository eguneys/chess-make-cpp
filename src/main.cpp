#include <iostream>
#include <string_view>
#include <sstream>

#include "bitboard.h"
#include "position.h"
#include "test.h"
#include "matcher.h"
#include "moves.h"
#include "file_io.h"





int adhoc_validate_puzzle_solution(Test::LichessPuzzle puzzle) {

    Chess::Position p;

    p.set(puzzle.FEN);

    std::stringstream iss(puzzle.moves);

    std::string first_move;
    std::string second_move;

    iss >> first_move;
    iss >> second_move;

    p.make_move(Chess::Move::parse_uci(first_move));

    Chess::Move move = Chess::Move::parse_uci(second_move);

    Chess::Piece a = p.piece_on(move.from_sq());
    Chess::Piece b = p.piece_on(move.to_sq());

    if (Chess::typeof_piece(b) == Chess::Knight) {
        return 1;
    }
    return 0;
}


int main() {
    std::cout << "Hello" << std::endl;

    Chess::Bitboards::init();

    Chess::BitsetManager res;
    Test::LichessDbPuzzle db;

    //db.open_and_build_index("../data/single_out.csv");
    //db.open_and_build_index("../data/athousand_sorted.csv");
    db.open_and_build_index("../data/lichess_db_puzzle.csv");
    //db.open_and_build_index("../data/test.log");

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


    //Util::FileAppender logger("../data/test.log", true);
    Util::FileAppender logger("../data/test2.log", true);
    logger.clear();

    int yes = 0;
    int no = 0;
    int i = 0;
    res.full_query([&i, &db, &no, &yes, &logger](u64 position_id)
                   {
                       Test::LichessPuzzle puzzle = db.get_full(position_id);
                       logger.writeLine(puzzle.full);

                       int result = 0;//adhoc_validate_puzzle_solution(puzzle);

                       if (result == 0)
                       {
                           no++;
                       }
                       else
                       {
                           yes++;
                       }

                       i++;
                       if (result == 0)
                       {
                           if (no > 16)
                           {
                               return;
                           }
                           std::cout << position_id << ":> " << puzzle.link << std::endl;
                       }
                   });

    int percent = abs(((float)i / total) * 100);
    std::cout << "Total found: %" << percent << "[" << i << "/" << total << "] Done.\n";
    std::cout << "No Yes:> " << no << "/" << yes << " Done.\n";

    return 0;
}
