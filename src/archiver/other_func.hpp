#pragma once

#include <vector>
#include <iostream>
#include <fstream>

unsigned
round_down_pow2 (unsigned num) // Number to be rounded
{
    return 1 << (8 * sizeof (num) - std::countl_zero (num) - 1);
} // round_down_pow2 (int num)

static std::string
readSource (std::string fileName) {
    using stream = std::istreambuf_iterator <std::string::traits_type::char_type>;

    std::ifstream file (fileName);
    return std::string (stream (file), stream ());
}

template <typename T>
bool operator != (const std::vector <T>& lhs,
                  const std::vector <T>& rhs)
{
    const std::size_t size = lhs.size ();
    if (size != rhs.size ()) {
        return true;
    }

    for (std::size_t i = 0; i < size; ++i) {
        if (lhs[i] != rhs[i]) {
            return true;
        }
    }

    return false;
} // bool operator != (const std::vector <T>& lhs, const std::vector <T>& rhs)

template <typename T>
bool operator == (const std::vector <T>& lhs,
                  const std::vector <T>& rhs)
{
    const std::size_t size = lhs.size ();
    if (size != rhs.size ()) {
        return false;
    }

    for (std::size_t i = 0; i < size; ++i) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }

    return true;
} // bool operator == (const std::vector <T>& lhs, const std::vector <T>& rhs)

template <typename CharT, typename Traits>
std::basic_ostream <CharT, Traits>&
tab (std::basic_ostream <CharT, Traits>& os) // Output stream
{
    return os.put (os.widen ('\t'));
} // tab (std::basic_ostream <CharT, Traits>& os)