#include "types.h"
#include "matcher.h"
#include <iostream>
#include <array>

namespace Chess
{

    void BitsetManager::full_query(std::function<void(u64)> materialize) {
        Bitset candidate_bishops =
            features.bishop_features[FeatureID::BISHOP_ONLY_DEFENDED_BY_KNIGHT] &
            features.bishop_features[FeatureID::BISHOP_ATTACKS_QUEEN];

        Bitset positions(nb_positions);

        for (size_t k = 0; k < candidate_bishops.size(); ++k) {
            if (candidate_bishops.test(k)) {
                assert(pieces[k].type == Bishop);
                positions.set(pieces[k].position_id);
            }
        }

        Bitset final_positions = positions & features.position_features[FeatureID::SIDE_TO_MOVE_WHITE];


        for (size_t k = 0; k < nb_positions; k++) {
            if (final_positions.test(k)) {
                materialize(k);
            }
        }
    }

    void BitsetManager::process_position_features(
        const Position &p,
        uint64_t position_id)
    {
        for (const auto &ext : EXTRACTORS)
        {
            if (ext.domain != FeatureDomain::Position)
                continue;

            auto fn = reinterpret_cast<PositionFeatureFn>(ext.fn);

            if (fn(p))
            {
                features.position_features[ext.id].set(position_id);
            }
        }
    }


    void BitsetManager::process_bishop_features(
        const Position &p,
        u64 position_id,
        Square square,
        Color color,
        size_t bishop_index)
    {
        for (auto &ext : EXTRACTORS)
        {
            if (ext.domain != FeatureDomain::BishopInstance)
                continue;

            auto fn = reinterpret_cast<BishopFeatureFn>(ext.fn);
            if (fn(p, PieceInstance{position_id, square, color}))
            {
                features.bishop_features[ext.id].set(bishop_index);
            }
        }
    }

    void BitsetManager::process_knight_features(
        const Position &p,
        u64 position_id,
        Square square,
        Color color,
        size_t knight_index)
    {
        for (auto &ext : EXTRACTORS)
        {
            if (ext.domain != FeatureDomain::KnightInstance)
                continue;

            auto fn = reinterpret_cast<KnightFeatureFn>(ext.fn);
            if (fn(p, PieceInstance{position_id, square, color}))
            {
                features.knight_features[ext.id].set(knight_index);
            }
        }
    }

    void BitsetManager::allocate_features()
    {
        for (auto &info : FEATURE_REGISTRY)
        {
            switch (info.domain)
            {
            case FeatureDomain::Position:
                features.position_features[info.id] = Bitset(nb_positions);
                break;
            case FeatureDomain::KnightInstance:
                features.knight_features[info.id] = Bitset(nb_pieces);

            case FeatureDomain::BishopInstance:
                features.bishop_features[info.id] = Bitset(nb_pieces);
                break;
            default:
                break;
            }
        }
    }

    void BitsetManager::begin_first_pass() {
        nb_positions = 0;
        current_piece_index = 0;
        nb_pieces = 0;
    }

    void BitsetManager::push_position_first_pass(const Position &p, u64 position_id) {
        Color opponent = ~p.side_to_move();

        nb_pieces += popcount(p.pieces());
        nb_positions++;
    }
    void BitsetManager::end_first_pass() {
        allocate_features();
    }
    void BitsetManager::process_position_second_pass(const Position &p, u64 position_id) {

        process_position_features(p, position_id);

        Color opponent  = ~p.side_to_move();

        Bitboard kk = p.pieces(Knight) & p.pieces(opponent);

        while (kk) {
            Square sq = pop_lsb(kk);

            u64 piece_index = current_piece_index++;

            pieces.push_back({
                position_id,
                sq,
                opponent,
                Knight
            });

            process_knight_features(p, position_id, sq, opponent, piece_index);
        }


        Bitboard bb = p.pieces(Bishop) & p.pieces(opponent);

        while (bb) {
            Square sq = pop_lsb(bb);

            u64 piece_index = current_piece_index++;

            pieces.push_back({
                position_id,
                sq,
                opponent,
                Bishop
            });

            process_bishop_features(p, position_id, sq, opponent, piece_index);
        }



        std::array<int64_t, 64> square_to_piece;
        square_to_piece.fill(-1LL);

        Bitboard occ = p.pieces();

        while (occ) {
            Square sq = pop_lsb(occ);

            Piece piece = p.piece_on(sq);
            PieceType pt = typeof_piece(piece);
            Color c = color_of(piece);

            u64 piece_id = pieces.size();
            pieces.push_back({
                position_id,
                sq,
                c,
                pt
            });

            square_to_piece[sq] = piece_id;
        }

        populate_relations_for_position(p, square_to_piece);
    }

    void BitsetManager::populate_relations_for_position(const Position &p,
                                                        std::array<int64_t, 64> &square_to_piece)
    {
        Bitboard bishops = p.pieces(Bishop);


        while (bishops) {
            Square b_sq = pop_lsb(bishops);
            Color c = p.color_on(b_sq);

            u64 bishop_id = square_to_piece[b_sq];
            assert(bishop_id != -1);
            assert(pieces[bishop_id].type == Bishop);


            Bitboard attackers = attackers_to(p, b_sq, c);

            Bitboard knight_attackers =
                attackers & p.pieces(Knight) & p.pieces(c);

            while (knight_attackers) {
                Square k_sq = pop_lsb(knight_attackers);
                u64 knight_id = square_to_piece[k_sq];
                assert(knight_id != -1);
                assert(pieces[knight_id].type == Knight);

                relations.knight_defends_bishop.add(knight_id, bishop_id);
            }
        }
    }
}