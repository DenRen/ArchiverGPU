#include <map>
#include <vector>
#include <stack>

#include "archiver.hpp"
#include "print_lib.hpp"

namespace archiver
{

struct node_t {
    bool root;
    int left, right;
    int value;
};

struct freq_t {
    int val = -1;
    int freq = -1;

    freq_t (int val, int freq) :
        val (val),
        freq (freq)
    {}

/*
    operator std::pair <int, int> () {
        return {val, freq};
    }*/
};
void sort (std::vector <freq_t>& vec) {
    std::sort (vec.begin (), vec.end (),
        [] (const auto& lhs, const auto& rhs) {
            return lhs.freq > rhs.freq;
        });
}

struct code_t {
    int len; // In bits
    uint64_t bits;
};

void fill_codes (const std::vector <node_t>& tree,
                 std::vector <code_t>& codes,
                 std::vector <int>& vals);

void
ArchiverCPU::archive (const std::vector <int>& data) {
    // Fill frequency values
    std::map <int, int> freq_map;
    for (const auto& value : data) {
        ++freq_map[value];
    }

    // Create sorted sequence
    std::vector <freq_t> freq_vec;
    for (const auto& pair : freq_map) {
        freq_vec.emplace_back (pair.first, pair.second);
    }
    std::cout << freq_vec << std::endl << std::endl;

    // Build tree
    std::vector <node_t> tree;
    tree.reserve (freq_vec.size ());
    for (auto& freq : freq_vec) {
        node_t node = {
            .root = true,
            .left = -1, .right = -1,
            .value = freq.val
        };
        freq.val = tree.size (); // Change value to position
        tree.push_back (node);
    }

    while (freq_vec.size () > 1) {
        sort (freq_vec);
        std::cout << "freq vec: " << freq_vec << std::endl;

        const auto end = freq_vec.end ();
        auto min1 = *(end - 1);
        auto min2 = *(end - 2);
        freq_vec.pop_back ();
        freq_vec.pop_back ();

        node_t node = {
            .root = false,
            .left = min1.val,
            .right = min2.val,
            .value = -1
        };

        tree.push_back (node);

        freq_t freq (tree.size () - 1, min1.freq + min2.freq);
        freq_vec.push_back (freq);

        sort (freq_vec);
    }
    std::cout << tree << std::endl;

    // Generte code
    std::vector <code_t> codes;
    std::vector <int> vals;
    fill_codes (tree, codes, vals);

    for (std::size_t i = 0; i < codes.size (); ++i) {
        std::cout << "value " << vals[i] << ": ";

        code_t code = codes[i];
        for (std::size_t j = 0; j < code.len; ++j) {
            std::cout << (1ull & code.bits);
            code.bits >>= 1;
        }
        std::cout << std::endl;
    }
}

std::size_t fill_bits (const std::vector <bool>& stack) {
    std::size_t res = 0;
    if (stack.size () > 8 * sizeof (res)) {
        throw std::invalid_argument ("stack.size () > 8 * sizeof (res)");
    }

    std::size_t i = 0;
    do {
        res <<= 1;
        res |= stack[i];
    } while (++i != stack.size ());

    return res;
}

void fill_codes (const std::vector <node_t>& tree,
                 std::size_t pos, bool is_right,
                 std::vector <code_t>& codes,
                 std::vector <int>& vals,
                 std::vector <bool>& stack) {
    stack.push_back (is_right);

    const node_t& node = tree[pos];
    if (node.root) {

        code_t code = {
            .len = static_cast <int> (stack.size ()),
            .bits = fill_bits (stack)
        };
        codes.push_back (code);
        vals.push_back (node.value);
    } else {
        fill_codes (tree, node.left, false, codes, vals, stack);
        fill_codes (tree, node.right, true, codes, vals, stack);
    }

    stack.pop_back ();
}

void fill_codes (const std::vector <node_t>& tree,
                 std::vector <code_t>& codes,
                 std::vector <int>& vals) {
    node_t root = tree[tree.size () - 1];
    std::vector <bool> stack;

    fill_codes (tree, root.left, false, codes, vals, stack);
    fill_codes (tree, root.right, true, codes, vals, stack);
}

AchiverGPU::AchiverGPU (cl::Device device) :
    cppl::ClAccelerator (device, "kernels/archiver.cl"),
    calc_freq_tables_ (program_, "calc_freq_tables")
{}

std::pair <int, int>
calc_size_per_work_group (int total_size,
                          int work_group_size,
                          int num_cu) {
    int num_work_item = work_group_size * num_cu;
    int size_per_work_item = total_size / num_work_item;

    if (total_size % num_work_item == 0) {
        return {size_per_work_item * work_group_size, 0};
    }

    ++size_per_work_item;
    int fill_size = size_per_work_item * num_work_item - total_size;
    return {size_per_work_item * work_group_size, fill_size};
}

void
AchiverGPU::calc_freq_table (const std::vector <data_t>& data,
                             data_t min,
                             data_t max) {
    // Calc number alphabets in local memory
    using alphabet_t = cl_uint;                 // Counter for single symbol
    const auto alphabet_size = max - min + 1;
    const auto alphabet_mem_size = alphabet_size * sizeof (alphabet_t);

    const auto local_mem_size = device_.getInfo <CL_DEVICE_LOCAL_MEM_SIZE> ();
    const auto num_alphabet_in_local_mem = local_mem_size / alphabet_mem_size;

    // Slice num work group if it great then max size in work group
    const auto max_work_group_size = device_.getInfo <CL_DEVICE_MAX_WORK_GROUP_SIZE> ();
    const auto work_group_size = std::min (num_alphabet_in_local_mem, max_work_group_size);
    const auto freq_tables_local_buf_size = alphabet_mem_size * work_group_size;
    std::cout << work_group_size << std::endl;
    // Calculate size of part data per work group
    const auto num_cu = device_.getInfo <CL_DEVICE_MAX_COMPUTE_UNITS> ();
    const auto data_size = data.size ();
    const auto [data_size_wg, fill_size] = calc_size_per_work_group (data_size,
                                                                     work_group_size, num_cu);
    std::cout << data_size_wg << " " << fill_size << std::endl;
    cl::Buffer data_buf;
    if (fill_size == 0) {
        // Send data to the device
        data_buf = sendBuffer (data, CL_MEM_READ_ONLY, CL_FALSE);
    } else {
        std::vector feeling_data (fill_size, min);

        const auto data_mem_size = data_size * sizeof (data[0]);
        data_buf = cl::Buffer (context_, CL_MEM_READ_ONLY, data_mem_size + fill_size);

        // Send data
        cmd_queue_.enqueueWriteBuffer (data_buf, CL_FALSE,
                                       0, data_mem_size, data.data (),
                                       nullptr, nullptr);
        // Send fill data
        cmd_queue_.enqueueWriteBuffer (data_buf, CL_FALSE,
                                       data_mem_size, fill_size, feeling_data.data (),
                                       nullptr, nullptr);
    }

    // Create buffer for frequency tables for each workgroup
    cl::Buffer freq_tables_buf {context_, CL_MEM_READ_WRITE, alphabet_mem_size * num_cu};
    cl::LocalSpaceArg freq_tables_local_buf { .size_ = freq_tables_local_buf_size };

    // Start calc freq table
    cl::NDRange global (work_group_size * num_cu);
    cl::NDRange local (work_group_size);
    cl::EnqueueArgs args {cmd_queue_, global, local};
    
    const auto data_size_wi = data_size_wg / work_group_size;
    calc_freq_tables_ (args, data_buf, freq_tables_buf, freq_tables_local_buf,
                       data_size_wi, alphabet_size, min);

    std::vector <int> freq_table (alphabet_size * num_cu);
    cl::copy (cmd_queue_, freq_tables_buf, freq_table.begin (), freq_table.end ());
    std::cout << "freq_table: " << freq_table << std::endl;
}


} // namespace archiver


namespace std
{

ostream&
operator << (std::ostream& os, const archiver::freq_t& freq) {
    return os << "{" << freq.val << "->" << freq.freq << "}";
}

ostream&
operator << (std::ostream& os, const archiver::node_t& n) {
    return os << "{" << std::boolalpha << n.root << ", left: " << n.left
              << ", right: " << n.right << ", value: " << n.value << "}" << std::endl;
}
}