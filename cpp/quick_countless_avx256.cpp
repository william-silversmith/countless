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

inline void find_mode(int const* __restrict d, int LDA, int* __restrict r,
                      int len)
{
    __m256i idx = {0x1ll, 0x200000003ll, 0x400000005ll, 0x600000007ll};

    static constexpr int unroll_factor = 64;

    for (int l = 0; l < len / (unroll_factor * 16); ++l)
    {
#pragma unroll(unroll_factor)
        for (int x = 0; x < unroll_factor; ++x)
        {

            __m256i A = _mm256_i32gather_epi32(d, idx, 8);
            __m256i B = _mm256_i32gather_epi32(d + 1, idx, 8);
            __m256i C = _mm256_i32gather_epi32(d + LDA, idx, 8);
            __m256i D = _mm256_i32gather_epi32(d + LDA + 1, idx, 8);

            auto mask_a_b = _mm256_cmpeq_epi32(A, B);
            auto mask_b_c = _mm256_cmpeq_epi32(B, C);
            auto mask_a_c = _mm256_cmpeq_epi32(A, C);

            A = _mm256_and_si256(A, mask_a_c);
            B = _mm256_and_si256(B, mask_b_c);
            C = _mm256_and_si256(C, mask_a_c);

            auto mask_abc = _mm256_or_si256(mask_a_b, mask_a_c);
            mask_abc      = _mm256_or_si256(mask_abc, mask_b_c);
            D             = _mm256_andnot_si256(mask_abc, D);

            A = _mm256_or_si256(A, B);
            A = _mm256_or_si256(A, C);
            A = _mm256_or_si256(A, D);

            _mm256_stream_si256(reinterpret_cast<__m256i*>(r), A);
            d += 16;
            r += 8;
        }
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
            find_mode(img + y * 2048, 1024, res + y * 512, 1024);
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
