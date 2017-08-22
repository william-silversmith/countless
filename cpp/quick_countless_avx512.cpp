#include <chrono>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <type_traits>
#include <x86intrin.h>

inline void* my_malloc(std::size_t required_bytes)
{
    if (required_bytes == 0)
    {
        return nullptr;
    }

    void*  p1; // original block
    void** p2; // aligned block

    size_t alignment = 64;

    int offset = alignment - 1 + sizeof(void*);

    if ((p1 = (void*)std::malloc(required_bytes + offset)) == NULL)
    {
        throw std::bad_alloc();
    }

    p2     = (void**)(((size_t)(p1) + offset) & ~(alignment - 1));
    p2[-1] = p1;
    return p2;
}

#define SIMD_FLOAT __m512i

inline void find_mode(int const* d, int LDA, int* r)
{
    __m512i idx = {0x1ll,         0x200000003ll, 0x400000005ll, 0x600000007ll,
                   0x800000009ll, 0xa0000000bll, 0xc0000000dll, 0xe0000000fll};

#pragma unroll(16)
    for (int x = 0; x < 16; ++x)
    {

        __m512i A = _mm512_i32gather_epi32(idx, d, 8);
        __m512i B = _mm512_i32gather_epi32(idx, d + 1, 8);
        __m512i C = _mm512_i32gather_epi32(idx, d + LDA, 8);
        __m512i D = _mm512_i32gather_epi32(idx, d + LDA + 1, 8);

        auto mask_a_b = _mm512_cmp_epi32_mask(A, B, _MM_CMPINT_EQ);
        auto mask_b_c = _mm512_cmp_epi32_mask(B, C, _MM_CMPINT_EQ);
        auto mask_a_c = _mm512_cmp_epi32_mask(A, C, _MM_CMPINT_EQ);

        A = _mm512_mask_mov_epi32(B, mask_b_c, A);
        D = _mm512_mask_mov_epi32(C, mask_b_c | mask_a_b | mask_a_c, D);
        D = _mm512_mask_mov_epi32(A, mask_a_b | mask_b_c, D);

        _mm512_stream_si512(reinterpret_cast<__m512i*>(r), D);
        d += 32;
        r += 16;
    }
}

int main()
{
    int* img = reinterpret_cast<int*>(my_malloc(1024 * 1024 * 4));
    int* res = reinterpret_cast<int*>(my_malloc(512 * 512 * 4));

    auto begin = std::chrono::high_resolution_clock::now();

    int iters = 1000;

    for (int j = 0; j < iters; ++j)
    {
        for (int y = 0; y < 512; ++y)
        {
            find_mode(img + y * 2048, 1024, res + y * 512);
            find_mode(img + y * 2048 + 512, 1024, res + y * 512 + 256);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - begin)
            .count();
    auto secs = static_cast<double>(duration) / 1000 / 1000;

    std::cout << "Thoughput: "
              << static_cast<double>(iters) * 1024 * 1024 / secs << std::endl;
}
