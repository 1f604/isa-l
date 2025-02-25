#ifndef ECCMAKER_H
#define ECCMAKER_H





#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <assert.h>
#include <time.h>
#include <sys/random.h>
#include "erasure_code.h"	// use <isa-l.h> instead when linking against installed


#define MMAX 255
#define KMAX 255
typedef unsigned char u8;



u8** calloc_matrix(int num_rows, int row_length);
void free_matrix(u8** matrix, int num_rows);

void print_array(
    const char* name,
    const u8* array,
    int num_elements
);


void choose_without_replacement(
    // I've thoroughly tested this function and ensured that it works.
    // this function is limited to arrays of up to 255 elements due to generate_byte, see below
    #define ELEMENT_TYPE u8
    // the #define macro is to make this function generic, so feel free to replace u8 with some other type.
    const ELEMENT_TYPE* input_array,
    ELEMENT_TYPE* output_array, // caller must allocate this, but does not need to initialize it
    int input_array_length, // number of elements in input and output array
    int output_array_length, // number of elements in input and output array
    int m, // k + p
    int elements_to_pick // number of elements to randomly pick from the input array
);


int gf_gen_decode_matrix_simple(
                    const u8 * encode_matrix,
                    const u8 * frag_err_list,
                    u8 * decode_matrix, // output matrix, modified by function
                    u8 * decode_index,  // output matrix, modified by function
                    int nerrs,
                    int k,
                    int m);

void test_exhaustive(
            int k,
            int m,
            int p,
            int len,
            const u8 *encode_matrix,
            u8 const * const * const frag_ptrs);


void test_random(
            int k,
            int m,
            int p,
            int nerrs,
            int len,
            const u8 *encode_matrix,
            u8 const * const * const frag_ptrs);

int test_helper(
            int k,
            int m,
            int p,
            int nerrs,
            int len,
            const u8 *encode_matrix,
            const u8 *frag_err_list,
            u8 const * const * const frag_ptrs);

void print_matrix(
    const char* name,
    const u8** matrix,
    int rows,
    int cols);







void test_exhaustive(
            int k,
            int m,
            int p,
            int len,
            const u8 *encode_matrix,
            u8 const * const * const frag_ptrs);

void test_random(
            int k,
            int m,
            int p,
            int nerrs,
            int len,
            const u8 *encode_matrix,
            u8 const * const * const frag_ptrs);

/**
 * Usage:
 *
 * u8** output_buffer = calloc_matrix(p, len);
 * recover_fragments_progressive(k,m,p,nerrs, len, encode_matrix, frag_err_list, output_buffer, frag_ptrs);
 * free_matrix(output_buffer);
 *
*/
int recover_fragments_progressive(
            int k,
            int m,
            int p,
            int nerrs,
            int len,
            const u8 *encode_matrix,
            const u8 *frag_err_list,
            u8** output_buffer, // this is where the recovered shards will be stored
            u8 const * const * const frag_ptrs);



void recover_data(int k, int m, int p, int nerrs, int len,
                    const u8 *encode_matrix,                    // input vector
                    const u8 * const * const frag_ptrs,         // input matrix
                    const u8 *frag_err_list,                    // input vector
                    u8** output_matrix,                         // output matrix, MUST BE ZERO-INITIALIZED BY CALLER!!!!!!!!!!!
                    int use_progressive
);


int test_helper(
            int k,
            int m,
            int p,
            int nerrs,
            int len,
            const u8 *encode_matrix,
            const u8 *frag_err_list,
            u8 const * const * const frag_ptrs);




#endif 
