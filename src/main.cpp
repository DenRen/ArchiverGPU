#include <iostream>
#include "archiver/archiver.hpp"
#include "archiver/print_lib.hpp"

int
get_bit (uint64_t value,
         unsigned pos) {
    return (value & (1ull << pos)) != 0;
}

// Print in real representation in member
std::ostream&
print_uint64 (uint64_t value, std::ostream& os = std::cout) {
    for (int i = 0; i < 64; i += 8) {
        for (int j = 0; j < 8; ++j) {
            os << get_bit (value, (i + 8) - (j + 1));
        }

        if ((i + 8) == 32) {
            os << "  ";
        } else {
            os << " ";
        }
    }

    return os;
} 

void
print_bits (const std::vector <uint64_t>& vec) {
    for (const auto& value : vec) {
        print_uint64 (value) << std::endl;
    }    
}

/*
10000111 10110010 10101111
10000111 10110010 10101111
10000111 10110010 10101111
10000111 10110010 10101111
10000111 10110010 10101111
10000111 10110010 10101110
*/

int main () {
    cppl::DeviceProvider deviceProvider;
    cl::Device device = deviceProvider.getDefaultDevice ();
    archiver::AchiverGPU arch {device};
    // archiver::ArchiverCPU arch;

    const unsigned size = 1000;
    std::vector <int> data (size / 2, 5);
    for (unsigned i = 0; i < size / 2; ++i) {
        data.push_back (2);
    }

    try {
        const int min = 1, max = 15;

        archiver::ArchiveGPU_t archive = arch.archive (data, min, max);
        const auto dearchive_data = arch.dearchive (archive);
        std::cout << dearchive_data << std::endl;
        std::cout << "Result: " << std::boolalpha << (data == dearchive_data) << std::endl;
    } catch (cl::Error& exc) {
        cppl::printError (exc);
    } catch (std::exception& exc) {
        std::cerr << exc.what () << std::endl;
    }
}