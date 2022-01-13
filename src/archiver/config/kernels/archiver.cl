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

    data_g += group_id * data_size;
    freq_table_l += id_g * freq_table_size;

    // Feel zero local memory
    for (uint i = 0; i < freq_table_size; ++i) {
        freq_table_l[i] = 0;
    }

    // Feel freq table
    for (uint i = 0; i < data_size; ++i) {
        int symbol = data_g[i] - begin_pattern;
        ++freq_table_l[symbol];
    }

    // Copy freq table from local memory to global memory
    for (uint i = 0; i < freq_table_size; ++i) {
        freq_table_g[i] = freq_table_l[i];
    }
}