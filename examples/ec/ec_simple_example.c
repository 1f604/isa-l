/*
    TODO:
        1. Load file from disk
        2. Separate out the code into separate encoder and repairer programs
        3. Load parity shards from disk for the recovery
        4. Write blog post on memory requirements for encode and decode processes



*/
#include "eccmaker_common.h"






int usage(void)
{
    fprintf(stderr,
        "Usage: ec_simple_example [options]\n"
        "  -h        Help\n"
        "  -f <val>  Path to file to encode\n"
        "  -k <val>  Number of source fragments\n"
        "  -p <val>  Number of parity fragments\n"
        "  -e <val>  Exhaustive testing for 1 missing fragment\n"
        "  -r <val>  Randomly test up to r missing fragments\n"
        "  -n <val>  Number of times to repeat each random test\n");
    exit(0);
}











int main(int argc, char *argv[])
{
    srand(time(NULL));
    int k = 10, p = 4, len = 8;	// Default params
    int random_test = 0;
    int random_repeat = 1;
    int exhaustive_test = 0;
	char* filepath = NULL;

    // Fragment buffer pointers
    u8 *frag_ptrs_encode[MMAX] = {0};
    u8 *frag_ptrs_encode_update[MMAX] = {0};


    // Coefficient matrices
    u8 *encode_matrix;
    u8 *g_tbls;

    int c;
    while ((c = getopt(argc, argv, "f:k:p:l:e:r:n:h")) != -1) {
        switch (c) {
        case 'f':
            filepath = strdup(optarg);
            break;
        case 'k':
            k = atoi(optarg);
            break;
        case 'p':
            p = atoi(optarg);
            break;
        case 'e':
            exhaustive_test = atoi(optarg);
            break;
        case 'r':
            random_test = atoi(optarg);
            break;
        case 'n':
            random_repeat = atoi(optarg);
            break;
        case 'h':
        default:
            usage();
            break;
        }
    }
    int m = k + p;

    // Check for valid parameters
    if (m > MMAX || k > KMAX || m < 0 || p < 1 || k < 1) {
        printf(" Input test parameter error m=%d, k=%d, p=%d\n",
               m, k, p);
        usage();
    }
    // Check for filename
    if (NULL == filepath) {
        puts("Error: You must specify a file to encode.");
        exit(-1);
    }

    printf("Encoding file: %s\n", filepath);

    printf("ec_simple_example:\n");

    // Allocate coding matrices
    encode_matrix = calloc(m * k, sizeof(u8));
    g_tbls = calloc(k * p * 32, sizeof(u8));

    if (encode_matrix == NULL || g_tbls == NULL) {
        printf("Test failure! Error with calloc\n");
        exit(-1);
    }
    // Allocate the src & parity buffers
    for (int i = 0; i < m; i++) {
        if (NULL == (frag_ptrs_encode[i] = calloc(len, sizeof(u8)))) {
            printf("alloc 1 error: Fail\n");
            exit(-1);
        }
        if (NULL == (frag_ptrs_encode_update[i] = calloc(len, sizeof(u8)))) {
            printf("alloc 2 error: Fail\n");
            exit(-1);
        }
    }

    // Fill sources with random data
    for (int i = 0; i < k; i++)
        for (int j = 0; j < len; j++){
            frag_ptrs_encode[i][j] = rand();
            frag_ptrs_encode_update[i][j] = frag_ptrs_encode[i][j];
        }

    print_matrix("Source matrix 1", (const u8**)frag_ptrs_encode, k, len);
    print_matrix("Source matrix 2", (const u8**)frag_ptrs_encode_update, k, len);

    printf(" encode (m,k,p)=(%d,%d,%d) len=%d\n", m, k, p, len);

    // Pick an encode matrix. A Cauchy matrix is a good choice as even
    // large k are always invertable keeping the recovery rule simple.
    gf_gen_cauchy1_matrix(encode_matrix, m, k);

    // Initialize g_tbls from encode matrix
    ec_init_tables(k, p, &encode_matrix[k * k], g_tbls);

    // Generate EC parity blocks from sources
    ec_encode_data(len, k, p, g_tbls, (const u8* const *)frag_ptrs_encode, &frag_ptrs_encode[k]);
    print_matrix("Source + Parity matrix encode 1", (const u8**)frag_ptrs_encode, m, len);

    // Generate EC parity blocks using progressive encoding
    for (int i = 0; i < k; i++){
        ec_encode_data_update(len, k, p, i, (const u8*)g_tbls, (const u8*)frag_ptrs_encode_update[i], &frag_ptrs_encode_update[k]);
    }


    print_matrix("Source + Parity matrix encode 2", (const u8**)frag_ptrs_encode, m, len);
    print_matrix("Source + Parity matrix encode_update", (const u8**)frag_ptrs_encode_update, m, len);

    // Check that encode and encode_update produce the same results
    puts("=== check the two versions are identical ==");
    for (int row = 0; row < m; row++) {
        if (memcmp(frag_ptrs_encode[row], frag_ptrs_encode_update[row], len)) {
            printf("Not identical: row %d\n", row);
            exit(-1);
        }
    }
    puts("Identical.");

    int nerrs = 1;
    if (exhaustive_test) {
        nerrs = 2;
        printf("======================== Exhaustive Testing 1 missing fragment ========================\n");
        test_exhaustive(k, m, p, len, (const u8*)encode_matrix, (u8 const * const * const)frag_ptrs_encode);
        test_exhaustive(k, m, p, len, (const u8*)encode_matrix, (u8 const * const * const)frag_ptrs_encode_update);
    }

    for (; nerrs <= random_test; nerrs++){
        printf("======================== Random Testing %d missing fragments ========================\n", nerrs);
        for (int j = 0; j < random_repeat; j++){
            test_random(k, m, p, nerrs, len, (const u8*)encode_matrix, (u8 const * const * const)frag_ptrs_encode);
            test_random(k, m, p, nerrs, len, (const u8*)encode_matrix, (u8 const * const * const)frag_ptrs_encode_update);
        }
    }



}
