__kernel void
calc_freq_tables (__global int* data_g,
                  __global int* freq_table_g,
                  __local int* freq_table_l,
                  unsigned data_size,
                  unsigned freq_table_size,
                  unsigned begin_pattern)
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