#include <map>
#include <vector>
#include <stack>

#include "archiver.hpp"
#include "print_lib.hpp"

#define PRINT(obj) std::cout << #obj ": " << obj << std::endl

namespace archiver
{

// ============================\\
// Static function for archiver -----------------------------------------------
// ============================//

struct freq_t {
    int pos = -1;
    int freq = -1;

    freq_t (int pos, int freq) :
        pos (pos),
        freq (freq)
    {}
};

static std::uint32_t
fill_bits (const std::vector <bool>& stack) {
    std::size_t res = 0;
    if (stack.size () > 8 * sizeof (res)) {
        throw std::invalid_argument ("stack.size () > 8 * sizeof (res)");
    }

    std::uint32_t i = 0;
    do {
        res <<= 1;
        res |= stack[stack.size () - i - 1];
    } while (++i != stack.size ());

    return res;
}

static void
fill_codes (const std::vector <node_t>& tree,
            std::size_t pos, bool is_right,
            std::vector <code_t>& codes,
            std::vector <int>& vals,
            std::vector <bool>& stack) {
    stack.push_back (is_right);

    const node_t& node = tree[pos];
    if (node.leaf) {

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

static void
fill_codes (const std::vector <node_t>& tree,
            std::vector <code_t>& codes,
            std::vector <int>& vals) {
    node_t root = tree[tree.size () - 1];
    std::vector <bool> stack;

    fill_codes (tree, root.left, false, codes, vals, stack);
    fill_codes (tree, root.right, true, codes, vals, stack);
}

static void
sort (std::vector <freq_t>& vec) {
    std::sort (vec.begin (), vec.end (),
        [] (const auto& lhs, const auto& rhs) {
            return lhs.freq > rhs.freq;
        });
}

// TODO: optimize
static std::vector <node_t>
calc_haff_tree (const std::vector <int>& freq_table) {
    PRINT (freq_table);

    std::vector <freq_t> freq_vec;
    freq_vec.reserve (freq_table.size ());

    // Fill leafs in tree
    std::vector <node_t> tree;
    tree.reserve (freq_table.size ());
    for (int i = 0; i < freq_table.size (); ++i) {
        if (freq_table[i] == 0) {
            continue;
        }

        node_t node = {
            .leaf = true,
            .left = -1, .right = -1,
            .value = i
        };
        tree.push_back (node);

        // Push position and frequency in freq_vec
        freq_t freq_item {(int) tree.size () - 1, freq_table[i]};
        freq_vec.push_back (freq_item);
    }

    // Build Haffman-tree
    while (freq_vec.size () > 1) {
        sort (freq_vec);
        PRINT (freq_vec);

        const auto end = freq_vec.end ();
        auto min1 = *(end - 1);
        auto min2 = *(end - 2);
        freq_vec.pop_back ();
        freq_vec.pop_back ();

        node_t node = {
            .leaf = false,
            .left = min1.pos,
            .right = min2.pos,
            .value = -1
        };

        tree.push_back (node);

        freq_t freq (tree.size () - 1, min1.freq + min2.freq);
        freq_vec.push_back (freq);
    }
    PRINT (tree);

    return tree;
}

static void
fill_codes (const std::vector <node_t>& tree,
            std::size_t pos, bool is_right,
            std::vector <std::pair <int, code_t>>& codes,
            std::vector <bool>& stack) {
    stack.push_back (is_right);

    const node_t& node = tree[pos];
    if (node.leaf) {

        code_t code = {
            .len = static_cast <int> (stack.size ()),
            .bits = fill_bits (stack)
        };
        codes.emplace_back (node.value, code);
    } else {
        fill_codes (tree, node.left, false, codes, stack);
        fill_codes (tree, node.right, true, codes, stack);
    }

    stack.pop_back ();
}

static void
fill_codes (const std::vector <node_t>& tree,
            std::vector <std::pair <int, code_t>>& codes) {
    node_t root = tree[tree.size () - 1];
    std::vector <bool> stack;

    fill_codes (tree, root.left, false, codes, stack);
    fill_codes (tree, root.right, true, codes, stack);
}

std::vector <code_t>
calc_codes_table (const std::vector <node_t>& freq_tree,
                  int alphabet_size) {
    std::vector <std::pair <int, code_t>> codes;
    fill_codes (freq_tree, codes);

    PRINT (codes);
    // Sort by value
    std::sort (codes.begin (), codes.end (),
        [] (const auto& lhs, const auto& rhs) {
            return lhs.first < rhs.first;
        });

    PRINT (codes);

    // Create full table
    std::vector <code_t> codes_table (alphabet_size, code_t {0});
    for (const auto& [pos, code] : codes) {
        codes_table[pos] = code;
    }

    PRINT (codes_table);
    return codes_table;
}

// ==========\\
// AchiverCPU -----------------------------------------------------------------
// ==========//

std::vector <int>
ArchiverCPU::calc_freq_table_impl (const std::vector <data_t>& data,
                                   data_t min,
                                   data_t max) {
    std::map <data_t, int> freq_map;
    for (const auto& value : data) {
        ++freq_map[value];
    }

    const auto alphabet_size = max - min + 1;
    std::vector <int> freq_table (alphabet_size);
    for (const auto& [value, freq] : freq_map) {
        freq_table[value - min] = freq;
    }

    return freq_table;
}

std::tuple <std::vector <uint64_t>, unsigned>
ArchiverCPU::archive_impl (const std::vector <data_t>& data,
                           const std::vector <code_t>& codes_table,
                           data_t min) {
                               PRINT (data.size ());
    const std::size_t max_size_coded = data.size () * sizeof (data_t) / sizeof (uint64_t) +
                                                     (sizeof (data_t) % sizeof (uint64_t) != 0);
    std::vector <uint64_t> coded (max_size_coded);
    PRINT (max_size_coded);
    unsigned pos = 0;
    for (const data_t& value : data) {
        code_t code = codes_table[value - min];
        uint64_t code_val = code.bits;
        coded[pos / 64] |= code_val << (pos % 64);
        pos += code.len;
    }

    coded.resize (pos / 64 + (pos % 64 != 0));
    coded.shrink_to_fit ();

    return {coded, pos};
}

std::tuple <std::vector <uint64_t>, unsigned , std::vector <node_t>>
ArchiverCPU::archive (const std::vector <data_t>& data,
                      data_t min,
                      data_t max) {
    const auto alphabet_size = max - min + 1;
    std::vector <int> freq_table = calc_freq_table_impl (data, min, max);
    std::vector <node_t> haff_tree = calc_haff_tree (freq_table);
    std::vector <code_t> codes_table = calc_codes_table (haff_tree, alphabet_size);

    const auto [archived_data, num_bits] = archive_impl (data, codes_table, min);

    return {archived_data, num_bits, haff_tree};
}

std::vector <data_t>
ArchiverCPU::dearchive (const std::vector <uint64_t>& data,
                        unsigned num_bits,
                        const std::vector <node_t>& haff_tree,
                        data_t min) {
    // std::vector <data_t> data;


    return {};
}

// ==========\\
// AchiverGPU -----------------------------------------------------------------
// ==========//

AchiverGPU::AchiverGPU (cl::Device device) :
    cppl::ClAccelerator (device, "kernels/archiver.cl"),
    calc_freq_tables_ (program_, "calc_freq_tables"),
    accumulate_freq_table_ (program_, "accumulate_freq_table")
{}

static std::pair <int, int>
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

std::vector <int>
AchiverGPU::calc_freq_table_impl (cl::Buffer& data_buf,
                                  const std::vector <data_t>& data,
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
    PRINT (work_group_size);
    // Calculate size of part data per work group
    const auto num_cu = device_.getInfo <CL_DEVICE_MAX_COMPUTE_UNITS> ();
    const auto data_size = data.size ();
    const auto [data_size_wg, fill_size] = calc_size_per_work_group (data_size,
                                                                     work_group_size, num_cu);
    PRINT (data_size_wg);
    PRINT (fill_size);

    // Send data buffer and filling data if necessary
    std::vector feeling_data (fill_size, min);
    if (fill_size == 0) {
        // Send data to the device
        data_buf = sendBuffer (data, CL_MEM_READ_ONLY, CL_FALSE);
    } else {
        const auto data_mem_size = data_size * sizeof (data[0]);
        const auto fill_mem_size = fill_size * sizeof (feeling_data[0]);
        data_buf = cl::Buffer (context_, CL_MEM_READ_ONLY, data_mem_size + fill_mem_size);

        // Send data
        cmd_queue_.enqueueWriteBuffer (data_buf, CL_FALSE,
                                       0, data_mem_size, data.data (),
                                       nullptr, nullptr);
        // Send fill data
        cmd_queue_.enqueueWriteBuffer (data_buf, CL_FALSE,
                                       data_mem_size, fill_mem_size, feeling_data.data (),
                                       nullptr, nullptr);
    }

    // Create buffer for frequency tables for each work group
    cl::Buffer freq_tables_buf {context_, CL_MEM_READ_WRITE, alphabet_mem_size * num_cu};
    cl::LocalSpaceArg freq_tables_local_buf { .size_ = freq_tables_local_buf_size };

    // Calc freq tables, accumulate then in local memory and write to global space
    // for each work group
    {
        cl::NDRange global (work_group_size * num_cu);
        cl::NDRange local (work_group_size);
        cl::EnqueueArgs args {cmd_queue_, global, local};

        const auto data_size_wi = data_size_wg / work_group_size;
        calc_freq_tables_ (args, data_buf, freq_tables_buf, freq_tables_local_buf,
                           data_size_wi, alphabet_size, min);
    }

    // Accumulate freq tables in global space and write result in first freq table
    {
        cl::NDRange global (alphabet_size);
        cl::EnqueueArgs args {cmd_queue_, global};
        accumulate_freq_table_ (args, freq_tables_buf, alphabet_size, num_cu);
    }

    // Read first freq table in global space and decrease on number of filled elements
    std::vector <int> freq_table (alphabet_size);
    cl::copy (cmd_queue_, freq_tables_buf, freq_table.begin (), freq_table.end ());
    freq_table[0] -= fill_size;
    return freq_table; // todo minus fill data
} // AchiverGPU::calc_freq_table_impl (cl::Buffer& data_buf, const std::vector <data_t>& data,
  //                                   data_t min, data_t max)

std::vector <int>
AchiverGPU::calc_freq_table (const std::vector <data_t>& data,
                             data_t min,
                             data_t max) {
    cl::Buffer data_buf;
    return calc_freq_table_impl (data_buf, data, min, max);
} // AchiverGPU::calc_freq_table (const std::vector <data_t>& data, data_t min, data_t max)

void
AchiverGPU::archive (const std::vector <data_t>& data,
                     data_t min,
                     data_t max) {
    const auto alphabet_size = max - min + 1;
    cl::Buffer data_buf;
    std::vector <int> freq_table = calc_freq_table_impl (data_buf, data, min, max);
    std::vector <node_t> freq_tree = calc_haff_tree (freq_table);
    std::vector <code_t> codes_table = calc_codes_table (freq_tree, alphabet_size);
}

} // namespace archiver

int
get_bit (uint32_t value,
         unsigned pos) {
    return (value & (1ull << pos)) != 0;
}

namespace std
{

ostream&
operator << (ostream& os, const archiver::code_t& code) {
    for (std::size_t j = 0; j < code.len; ++j) {
        os << get_bit (code.bits, code.len - 1 - j);
    }

    return os;
}

ostream&
operator << (ostream& os, const archiver::freq_t& freq) {
    return os << "{" << freq.pos << "->" << freq.freq << "}";
}

ostream&
operator << (ostream& os, const archiver::node_t& n) {
    return os << "{" << std::boolalpha << n.leaf << ", left: " << n.left
              << ", right: " << n.right << ", value: " << n.value << "}" << std::endl;
}

}