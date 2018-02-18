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

#define X_IN_OFF 1
#define Y_IN_OFF (X_DIM)
#define Z_IN_OFF (X_IN_OFF * Y_IN_OFF)

#define X_OUT_OFF 1
#define Y_OUT_OFF (Y_IN_OFF / 2)
#define Z_OUT_OFF (Z_IN_OFF / 4)


int8_t choose8_4[] = { 
    0,1,2,3, 0,1,2,4, 0,1,2,5, 0,1,2,6, 0,1,2,7, 0,1,3,4, 0,1,3,5, 
    0,1,3,6, 0,1,3,7, 0,1,4,5, 0,1,4,6, 0,1,4,7, 0,1,5,6, 0,1,5,7, 
    0,1,6,7, 0,2,3,4, 0,2,3,5, 0,2,3,6, 0,2,3,7, 0,2,4,5, 0,2,4,6, 
    0,2,4,7, 0,2,5,6, 0,2,5,7, 0,2,6,7, 0,3,4,5, 0,3,4,6, 0,3,4,7, 
    0,3,5,6, 0,3,5,7, 0,3,6,7, 0,4,5,6, 0,4,5,7, 0,4,6,7, 0,5,6,7, 
    1,2,3,4, 1,2,3,5, 1,2,3,6, 1,2,3,7, 1,2,4,5, 1,2,4,6, 1,2,4,7, 
    1,2,5,6, 1,2,5,7, 1,2,6,7, 1,3,4,5, 1,3,4,6, 1,3,4,7, 1,3,5,6, 
    1,3,5,7, 1,3,6,7, 1,4,5,6, 1,4,5,7, 1,4,6,7, 1,5,6,7, 2,3,4,5, 
    2,3,4,6, 2,3,4,7, 2,3,5,6, 2,3,5,7, 2,3,6,7, 2,4,5,6, 2,4,5,7, 
    2,4,6,7, 2,5,6,7, 3,4,5,6, 3,4,5,7, 3,4,6,7, 3,5,6,7, 4,5,6,7 
};

int8_t choose8_3[] = {
    0,1,2, 0,1,3, 0,1,4, 0,1,5, 0,1,6, 0,1,7, 0,2,3, 0,2,4, 0,2,5, 
    0,2,6, 0,2,7, 0,3,4, 0,3,5, 0,3,6, 0,3,7, 0,4,5, 0,4,6, 0,4,7, 
    0,5,6, 0,5,7, 0,6,7, 1,2,3, 1,2,4, 1,2,5, 1,2,6, 1,2,7, 1,3,4, 
    1,3,5, 1,3,6, 1,3,7, 1,4,5, 1,4,6, 1,4,7, 1,5,6, 1,5,7, 1,6,7, 
    2,3,4, 2,3,5, 2,3,6, 2,3,7, 2,4,5, 2,4,6, 2,4,7, 2,5,6, 2,5,7, 
    2,6,7, 3,4,5, 3,4,6, 3,4,7, 3,5,6, 3,5,7, 3,6,7, 4,5,6, 4,5,7, 
    4,6,7, 5,6,7
};

int8_t choose7_2[] = {
    0,1, 0,2, 0,3, 0,4, 0,5, 0,6, 1,2, 1,3, 1,4, 1,5, 1,6, 2,3, 2,4, 
    2,5, 2,6, 3,4, 3,5, 3,6, 4,5, 4,6, 5,6
};


void downsample(uint8_t* input, uint8_t* output) {
    
    uint8_t vals[8]; 
    uint8_t choose4 = 0;
    uint8_t choose3 = 0;
    uint8_t choose2 = 0;

    uint8_t p,q,r,s;

    uint16_t i;
    uint16_t o_i = 0;

    for (uint16_t x = 0; x < X_DIM; x += 2) {
        for (uint16_t y = 0; y < Y_DIM; y += 2) {
            for (uint16_t z = 0; z < Z_DIM; z += 2, o_i++) {
                vals[0] = input[ x + Y_IN_OFF * y + Z_IN_OFF * z ];
                vals[1] = input[ (x+1) + Y_IN_OFF * y + Z_IN_OFF * z ];
                vals[2] = input[ x + Y_IN_OFF * (y+1) + Z_IN_OFF * z ];
                vals[3] = input[ (x+1) + Y_IN_OFF * (y+1) + Z_IN_OFF * z ];
                vals[4] = input[ x + Y_IN_OFF * y + Z_IN_OFF * (z+1) ];
                vals[5] = input[ (x+1) + Y_IN_OFF * y + Z_IN_OFF * (z+1) ];
                vals[6] = input[ x + Y_IN_OFF * (y+1) + Z_IN_OFF * (z+1) ];
                vals[7] = input[ (x+1) + Y_IN_OFF * (y+1) + Z_IN_OFF * (z+1) ];

                // 8C4
                for (i = 0; i < 70 * 4; i += 4) {
                    p = vals[choose8_4[i + 0]];
                    q = vals[choose8_4[i + 1]];
                    r = vals[choose8_4[i + 2]];
                    s = vals[choose8_4[i + 3]];

                    p = p * ( (p == q) & (q == r) & (r == s) );
                    choose4 = choose4 + (choose4 == 0) * p;
                }

                // 8C3
                for (i = 0; i < 56 * 3; i += 3) {
                    p = vals[choose8_3[i + 0]];
                    q = vals[choose8_3[i + 1]];
                    r = vals[choose8_3[i + 2]];

                    p = p * ( (p == q) & (q == r) );
                    choose3 = choose3 + (choose3 == 0) * p;
                }

                // 7C2
                for (i = 0; i < 21 * 2; i += 2) {
                    p = vals[choose8_3[i + 0]];
                    q = vals[choose8_3[i + 1]];

                    p = p * (p == q);
                    choose2 = choose2 + (choose2 == 0) * p;
                }

                output[o_i] = choose4 + (choose4 == 0) * choose3;
                output[o_i] = output[o_i] + (output[o_i] == 0) * choose2;
                output[o_i] = output[o_i] + (output[o_i] == 0) * vals[7];
            }
        }
    }
}


int main(int argc, char **argv) {
    // if (argc < 2) {
    //     printf("error, missing path to input");
    //     return -1;
    // }

    clock_t startT, endT;

    printf("start %lu\n", sizeof(int));

    uint8_t* input = (uint8_t*)malloc(VOX_COUNT * sizeof(uint8_t));
    uint8_t* output = (uint8_t*)malloc(DS_VOX_COUNT * sizeof(uint8_t));

    printf("allocated arrays\n");

    // FILE *readPtr = fopen(argv[1], "rb");
    // startT = clock();
    // fread(input, sizeof(uint8_t), VOX_COUNT, readPtr);
    // endT = clock();
    float sec;// = (endT - startT) / (float)CLOCKS_PER_SEC;
    // printf("finished reading!, time: %f\n", sec);
    // fclose(readPtr);

    
    int multipler = 10;
    float acc = 0;
    for (int j = 0; j < 2; j++) {
        startT = clock();
        for (int i = 0; i < multipler; i++) {
            downsample(input, output);
        }
        endT = clock();
        sec = (endT - startT) / (float)CLOCKS_PER_SEC;
        float cur = VOX_COUNT / (1000000) / sec * (float)multipler;
        acc += cur;
        printf("MV/sec time: %f\n", acc / (j + 1));
    }
    

    free(input);

    FILE *writePtr;
    writePtr = fopen("output.bin","wb");
    fwrite(output, sizeof(uint8_t), DS_VOX_COUNT, writePtr);
    fclose(writePtr);

    free(output);
    return 0;
}
