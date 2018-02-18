/* Counting 2x2x2 Downsample
 *
 * Contributed by Chris Jordan, March 2017
 */

#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <tgmath.h>

#define X_DIM 1024
#define Y_DIM 1024
#define Z_DIM 64

#define VOX_COUNT (X_DIM * Y_DIM * Z_DIM)
#define DS_VOX_COUNT (VOX_COUNT / 8)

#define X_OFF 1
#define Y_OFF (X_DIM)
#define Z_OFF (X_DIM * Y_DIM)

#define HALF_X (X_DIM / 2)
#define HALF_Y (Y_DIM / 2)
#define HALF_Z (Z_DIM / 2)

void test(int index, uint32_t* input, uint32_t* output) {
    int o_x = index % HALF_X;
    int o_z = floor(index / (Z_OFF / 4));
    int t = index - o_z * (Z_OFF / 4);
    int o_y = floor(t / (Y_OFF / 2));

    int i_i = o_x * 2 * X_OFF + o_y * 2 * Y_OFF + o_z * 2 * Z_OFF;

    printf("debug %d - %d %d %d %d %d %d %d %d\n", output[index],
        input[i_i], input[i_i+1], input[i_i+Y_OFF], input[i_i+Y_OFF+1], input[i_i+Z_OFF], input[i_i+Z_OFF+1], input[i_i+Z_OFF+Y_OFF], input[i_i+Z_OFF+Y_OFF+X_OFF]
    );
}

void downsample(uint32_t* input, uint32_t* output) {
    int x, y, z;
    
    int i_i = 0;
    int o_i = 0;

    int vals[8];
    
    int maxCount = 0;
    int maxVal;
    int curCount;
    int curVal;

    for (z = 0; z < HALF_Z; z++, i_i += Z_OFF) {
        for (y = 0; y < HALF_Y; y++, i_i += Y_OFF) {
            for (x = 0; x < HALF_X; x++, i_i += 2, o_i += 1) {
                vals[0] = input[i_i];
                vals[1] = input[i_i + X_OFF];
                vals[2] = input[i_i + X_OFF + Y_OFF];
                vals[3] = input[i_i + X_OFF + Z_OFF];
                vals[4] = input[i_i + X_OFF + Y_OFF + Z_OFF];
                vals[5] = input[i_i + Y_OFF];
                vals[6] = input[i_i + Z_OFF];
                vals[7] = input[i_i + Y_OFF + Z_OFF];
                
                maxCount = 0;
                for (int t = 0; t < 8; t++) {
                    curVal = vals[t];
                    curCount = 0;
                    for (int p = 0; p < 8; p++) {
                        curCount += (curVal == vals[p]);
                    }

                    if (curCount > maxCount) {
                        maxCount = curCount;
                        maxVal = curVal;
                    }
                }
                
                output[o_i] = maxVal;
            }
        }
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("error, missing path to input");
        return -1;
    }


    clock_t startT, endT;

    printf("start %lu\n", sizeof(int));

    uint32_t* input = (uint32_t*)malloc(VOX_COUNT * sizeof(uint32_t));
    uint32_t* output = (uint32_t*)malloc(DS_VOX_COUNT * sizeof(uint32_t));

    printf("allocated arrays\n");

    FILE *readPtr = fopen(argv[1], "rb");
    startT = clock();
    fread(input, sizeof(uint32_t), VOX_COUNT, readPtr);
    endT = clock();
    float sec = (endT - startT) / (float)CLOCKS_PER_SEC;
    printf("finished reading!, time: %f\n", sec);
    fclose(readPtr);

    float acc = 0;
    for (int i = 0; i < 1; i++) {
        startT = clock();
        downsample(input, output);
        endT = clock();
        sec = (endT - startT) / (float)CLOCKS_PER_SEC;
        float cur = VOX_COUNT / (1000000) / sec;
        acc += cur;
        printf("MV/sec time: %f\n", acc / (i + 1));
    }

    
    test(0, input, output);
    test(1, input, output);
    test(1000, input, output);
    test(234823, input, output);
    test(125829120, input, output);
    test(39469608, input, output);
    test(129829120, input, output);

    free(input);

    FILE *writePtr;
    writePtr = fopen("output.bin","wb");
    fwrite(output, sizeof(uint32_t), DS_VOX_COUNT, writePtr);
    fclose(writePtr);

    free(output);
    return 0;
}
