#include "MirEngine/Core/Types/Count.hpp"

#include <cassert>
#include <unordered_map>

int main()
{
    constexpr mir::Count empty{};
    constexpr mir::Count five{5};
    constexpr mir::Count ten{10};

    static_assert(empty.empty());
    static_assert(five.value() == 5);
    static_assert(five < ten);
    static_assert(ten > five);

    assert(empty.empty());
    assert(five.value() == 5);

    std::unordered_map<mir::Count, int> values;
    values.emplace(five, 42);
    assert(values.at(five) == 42);

    return 0;
}
