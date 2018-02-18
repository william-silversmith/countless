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
#define Z_IN_OFF (X_DIM * Y_DIM)

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


void downsample(uint16_t* input, uint16_t* output) {
    
    uint16_t vals[8]; 
    uint16_t choose4 = 0;
    uint16_t choose3 = 0;
    uint16_t choose2 = 0;

    uint16_t p,q,r,s;

    uint16_t i;
    uint16_t x, y, z;
    uint32_t o_i = 0;

    uint16_t x1, y0, y1, z0, z1;

    for (z = 0; z < Z_DIM; z += 2) {   
        for (y = 0; y < Y_DIM; y += 2) {
            for (x = 0; x < X_DIM; x += 2) {
        
                y0 = y * Y_IN_OFF;
                z0 = z * Z_IN_OFF;
                x1 = x+1;
                y1 = (y+1) * Y_IN_OFF;
                z1 = (z+1) * Z_IN_OFF;

                vals[0] = input[ x + y0 + z0 ] + 1;
                vals[1] = input[ x1 + y0 + z0 ] + 1;
                vals[2] = input[ x + y1 + z0 ] + 1;
                vals[3] = input[ x1 + y1 + z0 ] + 1;
                vals[4] = input[ x + y0 + z1 ] + 1;
                vals[5] = input[ x1 + y0 + z1 ] + 1;
                vals[6] = input[ x + y1 + z1 ] + 1;
                vals[7] = input[ x1 + y1 + z1 ] + 1;

                // 8C4
                for (i = 0; i < 70 * 4; i += 4) {
                    p = vals[choose8_4[i + 0]];
                    q = vals[choose8_4[i + 1]];
                    r = vals[choose8_4[i + 2]];
                    s = vals[choose8_4[i + 3]];

                    p = p * (uint16_t)( (p == q) & (q == r) & (r == s) ); // PICK(P,Q,R,S)
                    choose4 = choose4 + (choose4 == 0) * p;
                }

                // 8C3
                for (i = 0; i < 56 * 3; i += 3) {
                    p = vals[choose8_3[i + 0]];
                    q = vals[choose8_3[i + 1]];
                    r = vals[choose8_3[i + 2]];

                    p = p * ( (p == q) & (q == r) ); // PICK(P,Q,R)
                    choose3 = choose3 + (choose3 == 0) * p;
                }

                // 7C2
                for (i = 0; i < 21 * 2; i += 2) {
                    p = vals[choose8_3[i + 0]];
                    q = vals[choose8_3[i + 1]];

                    p = p * (p == q); // PICK(P,Q)
                    choose2 = choose2 + (choose2 == 0) * p; // Logical OR
                }

                o_i = (x >> 1) + (y >> 1) * Y_OUT_OFF + (z >> 1) * Z_OUT_OFF;
                output[o_i] = choose4 + (choose4 == 0) * choose3;
                output[o_i] = output[o_i] + (output[o_i] == 0) * choose2;
                output[o_i] = output[o_i] + (output[o_i] == 0) * vals[7] - 1;
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


    printf("MVx/sec\tsec;\tMVx = %d\n", VOX_COUNT);

    int minibatch = 1;
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
    
    free(input);

    FILE *writePtr;
    writePtr = fopen("output.raw","wb");
    fwrite(output, sizeof(uint16_t), DS_VOX_COUNT, writePtr);
    fclose(writePtr);
    printf("Wrote output.raw\n");

    free(output);
    return 0;
}
