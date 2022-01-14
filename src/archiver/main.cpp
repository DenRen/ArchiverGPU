#include <iostream>
#include "archiver.hpp"
#include "print_lib.hpp"

bool
get_bit (const std::vector <uint64_t>& vec,
         unsigned pos) {
    uint8_t* data = (uint8_t*) vec.data ();
    return ((*(data + pos / 8)) & (1ull << (7 - pos % 8))) != 0;
}

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
    archiver::ArchiverCPU arch;

    std::vector <int> data = {
        1, 1, 2, 4, 4, 5, 5, 6, 6, 2,
        1, 1, 2, 4, 4, 5, 5, 6, 6, 2,
        1, 1, 2, 4, 4, 5, 5, 6, 6, 2,
        1, 1, 2, 4, 4, 5, 5, 6, 6, 2,
        1, 1, 2, 4, 4, 5, 5, 6, 6, 2,
        1, 1, 2, 4, 4, 5, 5, 6, 6, 2,
    };
    // 0001 1111

    // std::vector <int> data (122*5*200 + 170, 2);
    // for (int i = 0; i < 170; ++i) {
    //     data.push_back (14);

    //     data.push_back (12);
    //     data.push_back (12);
    //     data.push_back (12);

    //     data.push_back (1);
    //     data.push_back (1);
    // }

    try {
        // cppl::DeviceProvider deviceProvider;
        // cl::Device device = deviceProvider.getDefaultDevice ();
        // archiver::AchiverGPU archGpu {device};

        auto [archived_data, num_bits, haff_tree] = arch.archive (data, 1, 15);
        print_bits (archived_data);


    } catch (cl::Error& exc) {
        cppl::printError (exc);
    } catch (std::exception& exc) {
        std::cerr << exc.what () << std::endl;
    }
}