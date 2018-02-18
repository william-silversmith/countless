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

#define X_OUT_OFF 1
#define Y_OUT_OFF (X_DIM / 2)
#define Z_OUT_OFF (X_DIM * Y_DIM / 4)

void test(int index, uint16_t* input, uint16_t* output) {
    int o_x = index % HALF_X;
    int o_z = floor(index / (Z_OFF / 4));
    int t = index - o_z * (Z_OFF / 4);
    int o_y = floor(t / (Y_OFF / 2));

    int i_i = o_x * 2 * X_OFF + o_y * 2 * Y_OFF + o_z * 2 * Z_OFF;

    printf("debug %d - %d %d %d %d %d %d %d %d\n", output[index],
        input[i_i], input[i_i+1], input[i_i+Y_OFF], input[i_i+Y_OFF+1], input[i_i+Z_OFF], input[i_i+Z_OFF+1], input[i_i+Z_OFF+Y_OFF], input[i_i+Z_OFF+Y_OFF+X_OFF]
    );
}

void downsample(uint16_t* input, uint16_t* output) {
    uint16_t x, y, z;
    
    uint32_t o_i = 0;

    uint16_t vals[8];
    
    uint16_t maxCount = 0;
    uint16_t maxVal;
    uint16_t curCount;
    uint16_t curVal;

    uint16_t x1, y0, y1, z0, z1;

    for (x = 0; x < X_DIM; x += 2) {
        for (y = 0; y < Y_DIM; y += 2) {
            for (z = 0; z < Z_DIM; z += 2) {
                y0 = y * Y_OFF;
                z0 = z * Z_OFF;
                x1 = x+1;
                y1 = (y+1) * Y_OFF;
                z1 = (z+1) * Z_OFF;

                vals[0] = input[ x + y0 + z0 ];
                vals[1] = input[ x1 + y0 + z0 ];
                vals[2] = input[ x + y1 + z0 ];
                vals[3] = input[ x1 + y1 + z0 ];
                vals[4] = input[ x + y0 + z1 ];
                vals[5] = input[ x1 + y0 + z1 ];
                vals[6] = input[ x + y1 + z1 ];
                vals[7] = input[ x1 + y1 + z1 ];
                
                maxCount = 0;
                for (uint8_t t = 0; t < 8; t++) {
                    curVal = vals[t];
                    curCount = 0;
                    for (uint8_t p = 0; p < 8; p++) {
                        curCount += (curVal == vals[p]);
                    }

                    if (curCount > maxCount) {
                        maxCount = curCount;
                        maxVal = curVal;
                    }
                }
                
                o_i = (x >> 1) + (y >> 1) * Y_OUT_OFF + (z >> 1) * Z_OUT_OFF;
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

    uint16_t* input = (uint16_t*)malloc(VOX_COUNT * sizeof(uint16_t));
    uint16_t* output = (uint16_t*)malloc(DS_VOX_COUNT * sizeof(uint16_t));

    printf("allocated arrays\n");

    FILE *readPtr = fopen(argv[1], "rb");
    startT = clock();
    fread(input, sizeof(uint16_t), VOX_COUNT, readPtr);
    endT = clock();
    float sec = (endT - startT) / (float)CLOCKS_PER_SEC;
    printf("finished reading!, time: %f\n", sec);
    fclose(readPtr);

    int minibatch = 1;
    printf("MVx/sec\tsec;\tMVx = %d;\tN = %d\n", VOX_COUNT, minibatch);

    for (int i = 0; i < 1; i++) {
        startT = clock();
        for (int j = 0; j < minibatch; j++) {
            downsample(input, output);    
        }
        endT = clock();
        sec = (endT - startT) / (float)CLOCKS_PER_SEC;
        float mvxs = VOX_COUNT / (1024.0 * 1024.0) / sec * (float)minibatch;
        printf("%.2f\t%.2f\n", mvxs, sec);
    }
    
    test(0, input, output);
    test(1, input, output);
    test(1000, input, output);
    test(234823, input, output);
    test(pow(2, 16) - 1 , input, output);
    
    free(input);

    FILE *writePtr;
    writePtr = fopen("output.raw","wb");
    fwrite(output, sizeof(uint16_t), DS_VOX_COUNT, writePtr);
    fclose(writePtr);
    printf("Wrote output.raw\n");

    free(output);
    return 0;
}
