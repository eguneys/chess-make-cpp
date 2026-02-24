#pragma once

#include <cassert>
#include <string>

#include "types.h"


namespace Chess {

    
    enum MoveType: u16 {
        NORMAL
    };

    class Move
    {
    public:
        Move() = default;

        constexpr explicit Move(u16 d) : data(d) {}

        constexpr Move(Square from, Square to) : data((from << 6) + to) {}

        constexpr Square from_sq() const
        {
            assert(is_ok());
            return Square((data >> 6) & 0x3F);
        }

        constexpr Square to_sq() const
        {
            assert(is_ok());
            return Square(data & 0x3F);
        }



        constexpr MoveType type_of() const { return MoveType(data & (3 << 14)); }

        constexpr bool is_ok() const { return none().data != data && null().data != data; }

        static constexpr Move null() { return Move(65); }
        static constexpr Move none() { return Move(0); }

        static Move parse_uci(std::string& uci);

        constexpr u16 raw() const { return data; }

        private:

        u16 data;
    };
}