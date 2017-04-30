typedef struct output_s{
    char address[64];
    unsigned long address_length;
}output;

typedef struct tx_outputs_s{
    char hash[64];
    output *outs;
    unsigned long long outs_length;
}tx_outputs;
