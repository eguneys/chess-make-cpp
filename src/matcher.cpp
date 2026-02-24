#include "matcher.h"
#include <iostream>

namespace Chess
{

    void BitsetManager::full_query(std::function<void(u64)> materialize) {
        Bitset candidate_knights = features.knight_features[FeatureID::KNIGHT_ONLY_DEFENDED_BY_BISHOP];

        Bitset positions(nb_positions);

        for (size_t k = 0; k < candidate_knights.size(); ++k) {
            if (candidate_knights.test(k)) {
                positions.set(knights[k].position_id);
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
            if (fn(p, KnightInstance{position_id, square, color}))
            {
                features.knight_features[ext.id].set(knight_index);
            }
        }
    }

    void BitsetManager::allocate_features(size_t num_positions,
                                          size_t num_knights)
    {
        for (auto &info : FEATURE_REGISTRY)
        {
            switch (info.domain)
            {
            case FeatureDomain::Position:
                features.position_features[info.id] = Bitset(num_positions);
                break;
            case FeatureDomain::KnightInstance:
                features.knight_features[info.id] = Bitset(num_knights);
                break;
            default:
                break;
            }
        }
    }

    void BitsetManager::begin_first_pass() {
        nb_knights = 0;
        nb_positions = 0;

        current_knight_index = 0;
    }

    void BitsetManager::push_position_first_pass(const Position &p, u64 position_id) {
        Color opponent = ~p.side_to_move();
        nb_knights += popcount(p.pieces(Knight) & p.pieces(opponent));
        nb_positions++;
    }
    void BitsetManager::end_first_pass() {
        allocate_features(nb_positions, nb_knights);
    }
    void BitsetManager::process_position_second_pass(const Position &p, u64 position_id) {

        process_position_features(p, position_id);

        Color opponent  = ~p.side_to_move();

        Bitboard kk = p.pieces(Knight) & p.pieces(opponent);

        while (kk) {
            Square sq = pop_lsb(kk);

            u64 knight_index = current_knight_index++;

            knights.push_back({
                position_id,
                sq,
                opponent
            });

            process_knight_features(p, position_id, sq, opponent, knight_index);
        }

        assert(current_knight_index == knights.size());
    }
}