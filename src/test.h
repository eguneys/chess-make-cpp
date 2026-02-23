#pragma once

#include <functional>
#include <string_view>

namespace Test {

    int test(std::function<void(std::string_view)> processor);
}