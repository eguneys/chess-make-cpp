#pragma once

#include <iostream>
#include "types.h"
#include "bitset.h"

namespace Chess {

    enum class FeatureDomain
    {
        Position,       // indexed by position_id
        KnightInstance, // indexed by knight_instance_id
        BishopInstance,
        RookInstance,
        QueenInstance
    };



    struct PositionTag {};
    struct KnightTag {};
    struct BishopTag {};
    struct RookTag {};
    struct QueenTag {};
    struct MoveTag {};


    /*

    template <typename Tag>
    struct EntityTraits;

    template <>
    struct EntityTraits<KnightTag> {
       static bool matches(const PieceInstance& p) {
           return p.type == Knight;
       }
    };

    template <>
    struct EntityTraits<BishopTag> {
       static bool matches(const PieceInstance& p) {
           return p.type == Bishop;
       }
    };

    */

    template <typename T, typename U>
    struct RelationEdge {
        u64 l;
        u64 r;
    };


    template <typename LTag, typename RTag>
    class Relation {
        public:
        using Edge = RelationEdge<LTag, RTag>;

        void add(u64 t_id, u64 u_id) {
            edges.push_back({t_id, u_id});
        }

        const std::vector<Edge>& data() const {
            return edges;
        }

        u64 size() const {
            return edges.size();
        }

        private:
        std::vector<Edge> edges;
    };


    template <typename T, typename U>
    Bitset project_right(
        const Relation<T, U>& rel,
        const Bitset& left_filter,
        u64 num_u);

    template <typename T, typename U>
    Bitset project_left(
        const Relation<T, U>& rel,
        const Bitset& right_filter,
        u64 num_u);


    enum class RelationID: u16 {
        Knight_Defends_Bishop,
        Bishop_Defends_Knight,
        Knight_Attacks_Knight,
    };

    struct RelationInfo {
        RelationID id;
        FeatureDomain left;
        FeatureDomain right;
        const char* name;
    };


    constexpr RelationInfo RELATION_REGISTRY[] = {
        {
            RelationID::Knight_Defends_Bishop,
            FeatureDomain::KnightInstance,
            FeatureDomain::BishopInstance,
            "knight_defends_bishop"
        }
    };


    struct RelationStorage {
        Relation<KnightTag, BishopTag> knight_defends_bishop;
        Relation<QueenTag, QueenTag> queen_attacks_queen;
    };


    class RelationRegistry {
        public:
        RelationStorage storage;

        const RelationInfo& info(RelationID id) const {
            for (const auto& r: RELATION_REGISTRY) {
                if (r.id == id) {
                    return r;
                }
            }
            std::abort(); //invalid RelationID
        }
    };




    template <typename T, typename U>
    Bitset project_left(
        const Relation<T, U> &rel,
        const Bitset &right_filter,
        u64 num_u)
    {
        Bitset result(num_u);

        for (const auto &e : rel.data())
        {
            if (right_filter.test(e.l))
            {
                result.set(e.r);
            }
        }
        return result;
    }



    template <typename T, typename U>
    Bitset project_right(
        const Relation<T, U> &rel,
        const Bitset &left_filter,
        u64 num_u)
    {
        Bitset result(num_u);

        for (const auto &e : rel.data())
        {
            if (left_filter.test(e.r))
            {
                result.set(e.l);
            }
        }
        return result;
    }
}