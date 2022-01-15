#pragma once

#include <tuple>
#include "cppl.hpp"

namespace archiver
{

using data_t = int;

struct code_t {
    int len;        // In bits
    uint32_t bits;
};

struct node_t {
    bool leaf;
    int left, right;
    int value;
};

class ArchiverCPU {
    std::vector <int>
    calc_freq_table_impl (const std::vector <data_t>& data,
                          data_t min,
                          data_t max);

    std::tuple <std::vector <uint8_t>, unsigned>
    archive_impl (const std::vector <data_t>& data,
                  const std::vector <code_t>& codes_table,
                  data_t min);

public:
    std::tuple <std::vector <uint8_t>, unsigned, std::vector <node_t>>
    archive (const std::vector <data_t>& data,
                  data_t min = 1,
                  data_t max = 100);


    std::vector <data_t>
    dearchive (const std::vector <uint8_t>& data,
               unsigned num_bits,
               const std::vector <node_t>& haff_tree,
               data_t min);
};

struct ArchiveGPU_data_t {
    unsigned num_parts_;
    unsigned size_part_;                // In data_t input data
    std::vector <unsigned> lens_;       // In bits encoded data
    std::vector <uint8_t> coded_data_;

    ArchiveGPU_data_t (unsigned num_parts,
                       unsigned size_part,
                       std::vector <unsigned> lens,
                       std::vector <uint8_t> coded_data);
};

struct ArchiveGPU_t {
    ArchiveGPU_data_t data_;
    std::vector <node_t> haff_tree_;
    data_t min_value_;

    ArchiveGPU_t (ArchiveGPU_data_t data,
                  std::vector <node_t> haff_tree,
                  data_t min_value);
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

    cl::KernelFunctor <cl::Buffer,          // freq_table_g
                       unsigned,            // freq_table_size
                       unsigned>            // number_table
        accumulate_freq_table_;

    cl::KernelFunctor <cl::Buffer,          // data_t*          data_g
                       cl::Buffer,          // struct code_t*   codes_buf_g
                       cl::LocalSpaceArg,   // struct code_t*   codes_buf_l
                       cl::Buffer,          // unsigned*        lens_table_g
                       cl::LocalSpaceArg,   // unsigned*        buf_l
                       unsigned,            // unsigned         data_size
                       unsigned,            // unsigned         max_data_size
                       unsigned,            // unsigned         codes_buf_size
                       data_t>              // data_t           min_value
        archive_;
    
    cl::KernelFunctor <cl::Buffer,          // uchar*           archived_data_g
                       cl::Buffer,          // struct node_t*   haff_tree_g
                       cl::LocalSpaceArg,   // struct node_t*   haff_tree_l
                       cl::Buffer,          // data_t*          data_g
                       cl::Buffer,          // uint*            num_bits_g
                       data_t,              // data_t           min_value
                       unsigned,            // unsigned         data_size
                       unsigned>            // unsigned         haff_tree_size
        dearchive_;

    std::vector <int>
    calc_freq_table_impl (cl::Buffer& data_buf,
                          const std::vector <data_t>& data,
                          data_t min,
                          data_t max);

    ArchiveGPU_data_t
    archive_impl (cl::Buffer& data_buf,
                  const std::vector <data_t>& data,
                  data_t min_value,
                  const std::vector <code_t>& codes_table);

public:
    AchiverGPU (cl::Device device);
    
    std::vector <int>
    calc_freq_table (const std::vector <data_t>& data,
                     data_t min_value = 1,
                     data_t max_value = 100);
    
    ArchiveGPU_t
    archive (const std::vector <data_t>& data,
             data_t min_value = 1,
             data_t max_value = 100);

    std::vector <data_t>
    dearchive (const ArchiveGPU_t& archive);
};

} // namespace archiver

namespace std
{

ostream&
operator << (ostream& os, const archiver::code_t& code);

}