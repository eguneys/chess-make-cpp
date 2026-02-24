#pragma once

#include <unordered_map>
#include <functional>

#include "types.h"
#include "position.h"
#include "bitset.h"

namespace Chess {


    struct KnightInstance {
        u64 position_id;
        Square square;
        Color color;
    };



    enum class FeatureDomain
    {
        Position,       // indexed by position_id
        KnightInstance, // indexed by knight_instance_id
        BishopInstance,
        RookInstance
    };

    enum class FeatureID : uint16_t
    {
        // ---------- Position-level ----------
        SIDE_TO_MOVE_WHITE,
        KING_IN_CHECK,
        MATERIAL_IMBALANCE,

        // ---------- Knight-instance ----------
        KNIGHT_ONLY_DEFENDED_BY_BISHOP,
        KNIGHT_ATTACKED_BY_PAWN,
        KNIGHT_ON_OUTPOST,

        // ---------- Bishop-instance ----------
        BISHOP_PINNED_TO_KING,

        FEATURE_COUNT
    };

    struct FeatureInfo
    {
        FeatureID id;
        FeatureDomain domain;
        const char *name;
    };

    constexpr FeatureInfo FEATURE_REGISTRY[] = {
        {FeatureID::SIDE_TO_MOVE_WHITE,
         FeatureDomain::Position,
         "side_to_move_white"},

        {FeatureID::KNIGHT_ONLY_DEFENDED_BY_BISHOP,
         FeatureDomain::KnightInstance,
         "knight_only_defended_by_bishop"},

        {FeatureID::KNIGHT_ATTACKED_BY_PAWN,
         FeatureDomain::KnightInstance,
         "knight_attacked_by_pawn"},
    };

    struct FeatureStorage
    {
        // Position-level
        std::unordered_map<FeatureID, Bitset> position_features;

        // Knight-instance-level
        std::unordered_map<FeatureID, Bitset> knight_features;

        // Extendable
        std::unordered_map<FeatureID, Bitset> bishop_features;
    };

    using PositionFeatureFn = bool (*)(const Position&);
    using KnightFeatureFn = bool (*)(const Position&, const KnightInstance&);

    struct FeatureExtractor
    {
        FeatureID id;
        FeatureDomain domain;
        void *fn;
    };

    constexpr KnightFeatureFn knight_only_defended_by_bishop = [](const Position &p, const KnightInstance &k) {
        return true;
    };
    constexpr KnightFeatureFn knight_attacked_by_pawn = [](const Position &p, const KnightInstance &k) {
        return false;
    };





    constexpr FeatureExtractor EXTRACTORS[] = {
        {FeatureID::SIDE_TO_MOVE_WHITE,
         FeatureDomain::Position,
         (void *)+[](const Position &p)
         {
             return p.side_to_move() == White;
         }},
        {FeatureID::KNIGHT_ONLY_DEFENDED_BY_BISHOP,
         FeatureDomain::KnightInstance,
         (void *)knight_only_defended_by_bishop},
        {FeatureID::KNIGHT_ATTACKED_BY_PAWN,
         FeatureDomain::KnightInstance,
         (void *)knight_attacked_by_pawn}};
    
    class BitsetManager {

        public:
            void begin_first_pass();
            void push_position_first_pass(const Position &p, u64 position_id);
            void end_first_pass();
            void process_position_second_pass(const Position &p, u64 position_id);

            void full_query(std::function<void(u64)> materialize);

            private:
                void process_position_features(const Position &p, uint64_t position_id);
                void process_knight_features(const Position &p, u64 position_id, Square sq, Color c, size_t knight_index);
                void allocate_features(size_t num_positions, size_t num_knights);
                FeatureStorage features;

                u64 nb_positions;
                u64 nb_knights;
                u64 current_knight_index;
                std::vector<KnightInstance> knights;
            };
}