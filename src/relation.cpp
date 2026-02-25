#include "relation.h"

namespace Chess
{

    template <typename T, typename U>
    Bitset project_right(
        const Relation<T, U> &rel,
        const Bitset &left_filter,
        u64 num_u)
    {
        Bitset result(num_u);

        for (const auto &e : rel.data())
        {
            if (left_filter.test(e.t))
            {
                result.set(e.u);
            }
        }
        return result;
    }


    template <typename T, typename U>
    Bitset project_left(
        const Relation<T, U> &rel,
        const Bitset &right_filter,
        u64 num_u)
    {
        Bitset result(num_u);

        for (const auto &e : rel.data())
        {
            if (right_filter.test(e.u))
            {
                result.set(e.t);
            }
        }
        return result;
    }
}