#include <iostream>
#include "archiver.hpp"
#include "print_lib.hpp"

bool
get_bit (const std::vector <uint64_t>& vec,
         unsigned pos) {
    uint8_t* data = (uint8_t*) vec.data ();
    return ((*(data + pos / 8)) & (1ull << (7 - pos % 8))) != 0;
}

void
print_bits (const std::vector <uint64_t>& vec,
            unsigned num_bits) {
    for (unsigned i = 0; i < vec.size () * 64; ++i) {
        std::cout << (int) get_bit (vec, i);
        
        if ((i + 1) % 64 == 0) {
            std::cout << std::endl;
        } else if ((i + 1) % 8 == 0) {
            std::cout << " ";
        }
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
        print_bits (archived_data, num_bits);
        

    } catch (cl::Error& exc) {
        cppl::printError (exc);
    } catch (std::exception& exc) {
        std::cerr << exc.what () << std::endl;
    }
}