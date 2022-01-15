#include <random>
#include <gtest/gtest.h>
#include "archiver.hpp"

using archiver::data_t;

template <typename RandEngine, typename T>
std::vector <T>
genRandomVector (RandEngine rand_gen,
                 std::size_t size,
                 T min,
                 T max) {
    T delta = max - min;
    std::vector <T> vec;
    vec.reserve (size);
    
    while (size--) {
        T rand_val = rand_gen () % delta + min;
        vec.emplace_back (std::move (rand_val));
    }

    return vec;
}

template <typename RandEngine, typename T>
T get_rand (RandEngine rand_gen,
            T min,
            T max) {
    return rand_gen () % (max - min) + min;
}

TEST (ACHIVER_CPU_TEST, RANDOM_GENERATED) {
    return;
    archiver::ArchiverCPU arch;

    const unsigned min_alphabet_size = 2;
    const unsigned max_alphabet_size = 255;

    const unsigned min_data_size = 5;
    const unsigned max_data_size = 2'000'000;

    std::random_device rd;
    std::mt19937 mersenne {rd ()};

    const data_t min = 13;
    int num_repeats = 10;

    auto gen_rand = [&mersenne] (auto min, auto max) {
        return get_rand (mersenne, min, max);
    };

    while (num_repeats--) {
        const auto size = gen_rand (min_data_size, max_data_size);
        const auto alphabet_size = gen_rand (min_alphabet_size, max_alphabet_size);
        const data_t max = min + alphabet_size;
        
        const std::vector <data_t> data = genRandomVector (mersenne, size, min, max);

        auto [archived_data, num_bits, haff_tree] = arch.archive (data, min, max);
        const std::vector <data_t> data_decoded = arch.dearchive (archived_data, num_bits, haff_tree, min);

        ASSERT_EQ (data, data_decoded);
    }
}

TEST (ACHIVER_GPU_TEST, RANDOM_GENERATED) {
    cppl::DeviceProvider deviceProvider;
    cl::Device device = deviceProvider.getDefaultDevice ();
    archiver::AchiverGPU arch {device};

    const unsigned min_alphabet_size = 2;
    const unsigned max_alphabet_size = 32;

    const unsigned min_data_size =   100'000;
    const unsigned max_data_size = 2'000'000;

    std::random_device rd;
    std::mt19937 mersenne {rd ()};

    const data_t min = 13;
    int num_repeats = 1000;

    auto gen_rand = [&mersenne] (auto min, auto max) {
        return get_rand (mersenne, min, max);
    };

    while (num_repeats--) {
        const auto size = gen_rand (min_data_size, max_data_size);
        const auto alphabet_size = gen_rand (min_alphabet_size, max_alphabet_size);
        const data_t max = min + alphabet_size;
        
        const std::vector <data_t> data = genRandomVector (mersenne, size, min, max);

        archiver::ArchiveGPU_t archive = arch.archive (data, min, max);
        const std::vector <data_t> data_decoded = arch.dearchive (archive);

        ASSERT_EQ (data, data_decoded);
    }
}