typedef int data_t;

__kernel void
calc_freq_tables (__global data_t* data_g,
                  __global int* freq_table_g,
                  __local int* freq_table_l,
                  unsigned data_size,
                  unsigned freq_table_size,
                  data_t begin_pattern)
{
    unsigned id_g = get_global_id (0);
    unsigned id_l = get_local_id (0);
    unsigned group_id = get_group_id (0);

    int* save_freq_table_l = freq_table_l;

    data_g += id_g * data_size;
    freq_table_l += id_l * freq_table_size;

    // Feel zero local memory
    for (uint i = 0; i < freq_table_size*0+10; ++i) {
        freq_table_l[i] = 0;
    }

    // Feel freq table
    for (uint i = 0; i < data_size; ++i) {
        int symbol = data_g[i] - begin_pattern;
        ++freq_table_l[symbol];
    }

    barrier (CLK_LOCAL_MEM_FENCE);
    freq_table_g += group_id * freq_table_size;

    unsigned local_size = get_local_size (0);
    for (int local_pos = id_l; local_pos < freq_table_size; local_pos += local_size) {
        int* freq_cur = save_freq_table_l + local_pos;

        unsigned sum = 0;
        for (int i = 0; i < local_size; ++i) {
            sum += *freq_cur;
            freq_cur += freq_table_size;
        }

        freq_table_g[local_pos] = sum;
    }
}

__kernel void
accumulate_freq_table (__global int* freq_table_g,
                       unsigned freq_table_size,
                       unsigned number_table)
{
    unsigned id_g = get_global_id (0);

    __global int* cur_freq_save = freq_table_g + id_g;
    freq_table_g += id_g + freq_table_size * (number_table - 1);

    int sum = 0;
    for (int i = number_table; i != 0; --i) {
        sum += *freq_table_g;
        freq_table_g -= freq_table_size;
    }

    *cur_freq_save = sum;
}

struct code_t {
    int len;
    uint bits;
};

inline uint
bits2bytes (uint num_bits) {
    return num_bits / 8 + ((num_bits % 8) != 0);
}

__kernel void
archive (__global data_t* data_g,
         __global struct code_t* codes_buf_g,
         __local  struct code_t* codes_buf_l,
         __global uint* lens_table_g,
         __local uint* buf_l,
         uint data_size,
         uint max_data_size,
         uint codes_buf_size,
         data_t min_value)
{
    uint id_l = get_local_id (0);
    uint id_g = get_global_id (0);

    // Copy codes table to local memory
    if (id_l == 0) {
        for (uint i = 0; i < codes_buf_size; ++i) {
            codes_buf_l[i] = codes_buf_g[i];
        }
    }
    barrier (CLK_LOCAL_MEM_FENCE);

    buf_l += data_size * id_l;
    buf_l[0] = 0;

    uint data_pos_begin = data_size * id_g;
    uint data_pos_end = data_pos_begin + data_size;
    if (data_pos_end > max_data_size) {
        data_pos_end = max_data_size;
    }

    uint pos = 0;
    for (uint data_pos = data_pos_begin;
         data_pos < data_pos_end;
         ++data_pos)
    {
        int value = data_g[data_pos];
        struct code_t code = codes_buf_l[value - min_value];

        buf_l[pos / 32    ] |= code.bits << (pos % 32);
        buf_l[pos / 32 + 1]  = code.bits >> (32 - pos % 32);

        pos += code.len;
    }

    lens_table_g[id_g] = pos;
    barrier (CLK_GLOBAL_MEM_FENCE);

    uint pos_g = 0;
    for (uint i = 0; i < id_g; ++i) {
        uint len = lens_table_g[i];
        pos_g += bits2bytes (len);
    }

    uchar* res_data_g = ((uchar*) data_g) + pos_g;
    uchar* res_data_l = (uchar*) buf_l;
    uint size_res = bits2bytes (pos);
    for (uint i = 0; i < size_res; ++i) {
        res_data_g[i] = res_data_l[i];
    }
}

struct node_t {
    bool leaf;
    int left, right;
    int value;
};
struct pos_len_t {
    uint pos;
    uint num_bits;
};

__kernel void
dearchive (__global uchar* archived_data_g,
           __global struct node_t* haff_tree_g,
           __local  struct node_t* haff_tree_l,
           __global data_t* data_g,
           __global struct pos_len_t* pos_len_g,
           data_t min_value,
           uint data_size,
           uint haff_tree_size)
{
    uint id_l = get_local_id (0);
    uint id_g = get_global_id (0);

    // Read haff tree to local memory
    if (id_l == 0) {
        for (int i = 0; i < haff_tree_size; ++i) {
            haff_tree_l[i] = haff_tree_g[i];
        }
    }
    barrier (CLK_LOCAL_MEM_FENCE);

    struct pos_len_t pos_len = pos_len_g[id_g];
    archived_data_g += pos_len.pos;
    const uint num_bits = pos_len.num_bits;
    data_g += data_size * id_g;

    const uint root = haff_tree_size - 1;
    struct node_t node = haff_tree_l[root];
    
    for (uint pos = 0; pos < num_bits; pos += 8) {
        uchar bits = archived_data_g[pos / 8];

        uint pos_end = min (pos + 8, num_bits);
        for (int i = pos; i < pos_end; ++i) {
            uint bit = (bits & (1u << (i - pos))) != 0;
            uint tree_pos = bit ? node.right : node.left;

            node = haff_tree_l[tree_pos];
            if (node.leaf) {
                *data_g++ = node.value + min_value;
                node = haff_tree_l[root];
            }
        }
    }
}