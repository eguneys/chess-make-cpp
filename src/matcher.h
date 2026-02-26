#pragma once

#include <unordered_map>
#include <functional>

#include "types.h"
#include "position.h"
#include "bitset.h"
#include "bitboard_extra.h"
#include "relation.h"

namespace Chess {


    struct PieceInstance {
        u64 position_id;
        Square square;
        Color color;
        PieceType type;
    };

    enum class FeatureID : u16
    {
        // ---------- Position-level ----------
        SIDE_TO_MOVE_WHITE,
        KING_IN_CHECK,
        MATERIAL_IMBALANCE,

        // ---------- Knight-instance ----------
        KNIGHT_OCCUPIES,
        KNIGHT_CAN_BE_CAPTURED_WITH_CHECK,
        KNIGHT_TAKES_KNIGHT_WITH_CHECK,
        KNIGHT_ONLY_DEFENDED_BY_BISHOP,
        KNIGHT_ATTACKED_BY_PAWN,
        KNIGHT_ON_OUTPOST,

        // ---------- Bishop-instance ----------
        BISHOP_ONLY_DEFENDED_BY_KNIGHT,
        BISHOP_ATTACKS_QUEEN,
        BISHOP_PINNED_TO_KING,

        // ---- Queen ----
        QUEEN_ONLY_DEFENDED_BY_ROOK,


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

        {FeatureID::KNIGHT_OCCUPIES,
         FeatureDomain::KnightInstance,
         "knight_occupies"},

        {FeatureID::KNIGHT_CAN_BE_CAPTURED_WITH_CHECK,
         FeatureDomain::KnightInstance,
         "knight_can_be_captured_with_check"},

        {FeatureID::KNIGHT_TAKES_KNIGHT_WITH_CHECK,
         FeatureDomain::KnightInstance,
         "knight_takes_knight_with_check"},


        {FeatureID::KNIGHT_ONLY_DEFENDED_BY_BISHOP,
         FeatureDomain::KnightInstance,
         "knight_only_defended_by_bishop"},

        {FeatureID::KNIGHT_ATTACKED_BY_PAWN,
         FeatureDomain::KnightInstance,
         "knight_attacked_by_pawn"},

        {FeatureID::BISHOP_ONLY_DEFENDED_BY_KNIGHT,
         FeatureDomain::BishopInstance,
         "bishop_only_defended_by_knight"},
        {FeatureID::BISHOP_ATTACKS_QUEEN,
         FeatureDomain::BishopInstance,
         "bishop_attacks_queen"},

        {FeatureID::QUEEN_ONLY_DEFENDED_BY_ROOK,
         FeatureDomain::QueenInstance,
         "bishop_only_defended_by_knight"},


    };

    struct FeatureStorage
    {
        // Position-level
        std::unordered_map<FeatureID, Bitset> position_features;

        // Knight-instance-level
        std::unordered_map<FeatureID, Bitset> knight_features;

        // Extendable
        std::unordered_map<FeatureID, Bitset> bishop_features;
        std::unordered_map<FeatureID, Bitset> queen_features;
    };

    using PositionFeatureFn = bool (*)(const Position&);
    using PieceFeatureFn = bool (*)(const Position&, const PieceInstance&);

    struct FeatureExtractor
    {
        FeatureID id;
        FeatureDomain domain;
        void *fn;
    };

    constexpr PieceFeatureFn bishop_attacks_queen = [](const Position &p, const PieceInstance &k)
    {
        Bitboard attacks = bishop_attacks(k.square, p.pieces());

        Bitboard qq = p.pieces(Queen) & p.pieces(~k.color);


        if (qq & attacks) {
            return true;
        }


        return false;
    };

    constexpr PieceFeatureFn bishop_only_defended_by_knight = [](const Position &p, const PieceInstance &k) {
        Bitboard defenders = attackers_to(p, k.square, k.color);

        if (!defenders) {
            return false;
        }

        Square only = pop_lsb(defenders);

        if (!defenders) {
            return typeof_piece(p.piece_on(only)) == Knight;
        }

        return false;
    };

    constexpr PieceFeatureFn queen_only_defended_by_rook = [](const Position &p, const PieceInstance &k) {
        Bitboard defenders = attackers_to(p, k.square, k.color);

        if (!defenders) {
            return false;
        }

        Square only = pop_lsb(defenders);

        if (!defenders) {
            return typeof_piece(p.piece_on(only)) == Rook;
        }

        return false;
    };



    constexpr PieceFeatureFn knight_occupies = [](const Position &p, const PieceInstance &k) {
        return true;
    };



    constexpr PieceFeatureFn knight_only_defended_by_bishop = [](const Position &p, const PieceInstance &k) {
        Bitboard defenders = attackers_to(p, k.square, k.color);

        Square only = pop_lsb(defenders);

        if (!defenders) {
            return typeof_piece(p.piece_on(only)) == Bishop;
        }

        return false;
    };

    constexpr PieceFeatureFn knight_attacked_by_pawn = [](const Position &p, const PieceInstance &k) {
        return false;
    };

    constexpr PieceFeatureFn knight_takes_knight_with_check = [](const Position &p, const PieceInstance &k) {
        Bitboard takes_knight = attacks_bb(Knight, k.square, p.pieces()) & p.pieces(Knight) & p.pieces(~k.color);

        while (takes_knight) {
            Square sq = pop_lsb(takes_knight);

            Bitboard with_check = attacks_bb(Knight, sq, p.pieces()) & p.pieces(~k.color) & p.pieces(King);

            return with_check != 0;
        }

        return false;
    };

    constexpr PieceFeatureFn knight_can_be_captured_with_check = [](const Position &p, const PieceInstance &k) {
        Bitboard takes_knight = attackers_to(p, k.square, ~k.color);

        while (takes_knight) {
            Square sq = pop_lsb(takes_knight);
            PieceType pc = typeof_piece(p.piece_on(sq));

            Bitboard with_check = (pc == Pawn ? pawn_attacks_bb(~k.color, sq) : attacks_bb(pc, k.square, p.pieces())) & p.pieces(k.color) & p.pieces(King);

            return with_check != 0;
        }

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
        {FeatureID::KNIGHT_OCCUPIES,
         FeatureDomain::KnightInstance,
         (void *)knight_occupies},
        {FeatureID::KNIGHT_CAN_BE_CAPTURED_WITH_CHECK,
         FeatureDomain::KnightInstance,
         (void *)knight_can_be_captured_with_check},
        {FeatureID::KNIGHT_TAKES_KNIGHT_WITH_CHECK,
         FeatureDomain::KnightInstance,
         (void *)knight_takes_knight_with_check},
        {FeatureID::KNIGHT_ATTACKED_BY_PAWN,
         FeatureDomain::KnightInstance,
         (void *)knight_attacked_by_pawn},
        {FeatureID::BISHOP_ONLY_DEFENDED_BY_KNIGHT,
         FeatureDomain::BishopInstance,
         (void *)bishop_only_defended_by_knight},
        {FeatureID::BISHOP_ATTACKS_QUEEN,
         FeatureDomain::BishopInstance,
         (void *)bishop_attacks_queen},
        {FeatureID::QUEEN_ONLY_DEFENDED_BY_ROOK,
         FeatureDomain::QueenInstance,
         (void *)queen_only_defended_by_rook},
    };

    class BitsetManager {

        public:
            void begin_first_pass();
            void push_position_first_pass(const Position &p, u64 position_id);
            void end_first_pass();
            void process_position_second_pass(const Position &p, u64 position_id);

            void full_query(std::function<void(u64)> materialize);

            private:

                void populate_relations_for_position(const Position &p, std::array<int64_t, 64> &square_to_piece);

                void process_position_features(const Position &p, uint64_t position_id);
                void process_knight_features(const Position &p, u64 position_id, Square sq, Color c, size_t knight_index);
                void process_bishop_features(const Position &p, u64 position_id, Square sq, Color c, size_t bishop_index);
                void process_queen_features(const Position &p, u64 position_id, Square sq, Color c, size_t queen_index);
                void allocate_features();
                FeatureStorage features;
                RelationStorage relations;

                u64 nb_positions;
                u64 nb_pieces;
                u64 current_piece_index;
                std::vector<PieceInstance> pieces;
            };
}