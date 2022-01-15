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

    // {
    //     1, 1, 2, 4, 4, 5, 5, 6, 6, 2,
    //     1, 1, 2, 4, 4, 5, 5, 6, 6, 2,
    //     1, 1, 2, 4, 4, 5, 5, 6, 6, 2,
    //     1, 1, 2, 4, 4, 5, 5, 6, 6, 2,
    //     1, 1, 2, 4, 4, 5, 5, 6, 6, 2,
    //     1, 1, 2, 4, 4, 5, 5, 6, 6, 2,
    // };
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

        const int min = 1, max = 15;

        auto [archived_data, num_bits, haff_tree] = arch.archive (data, min, max);
        // print_bits (archived_data);

        // std::vector <int> data_decoded = arch.dearchive (archived_data, num_bits, haff_tree, min);
        
        // for (int i = 0; i < data.size (); i += data.size () / 6) {
        //     for (int j = i; j < i + data.size () / 6; ++j) {
        //         std::cout << data_decoded[j] << " ";
        //     }

        //     std::cout << std::endl;
        // }
        // std::cout << std::endl << "Result: " << (data_decoded) << std::endl;

    } catch (cl::Error& exc) {
        cppl::printError (exc);
    } catch (std::exception& exc) {
        std::cerr << exc.what () << std::endl;
    }
}