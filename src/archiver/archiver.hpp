#pragma once

#include "cppl.hpp"

namespace archiver
{

// struct

class ArchiverCPU {
public:
    void archive (const std::vector <int>& data);
};

class AchiverGPU : public cppl::ClAccelerator {
    using cppl::ClAccelerator::device_;
    using cppl::ClAccelerator::context_;
    using cppl::ClAccelerator::cmd_queue_;
    using cppl::ClAccelerator::program_;

    cl::KernelFunctor <cl::Buffer,          // data_g
                       cl::Buffer,          // freq_table_g
                       cl::LocalSpaceArg,   // freq_table_l
                       unsigned,            // data_size
                       unsigned,            // freq_table_size
                       unsigned>            // begin_pattern
        calc_freq_tables_;

    cl::KernelFunctor <cl::Buffer,
                       unsigned,
                       unsigned>
        accumulate_freq_table_;

public:
    AchiverGPU (cl::Device device);

    using data_t = int;

    
    std::vector <int>
    calc_freq_table (const std::vector <data_t>& data, data_t min = 1, data_t max = 100);
    
    void
    archive (const std::vector <data_t>& data, data_t min = 1, data_t max = 100);
};

} // namespace archiver