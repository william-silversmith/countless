#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <tgmath.h>

#define X_DIM 1024  
#define Y_DIM 1024

#define VOX_COUNT (X_DIM * Y_DIM)
#define DS_VOX_COUNT (VOX_COUNT / 4)

#define X_OFF 1
#define Y_OFF (X_DIM)

#define HALF_X (X_DIM / 2)
#define HALF_Y (Y_DIM / 2)

void downsample(uint8_t *restrict input, uint8_t *restrict output) {
    unsigned int x, y;
    
    unsigned int i_i = 0;
    unsigned int o_i = 0;

    unsigned short a, b, c, d;

    unsigned short ab, bc, ac;

    for (y = 0; y < HALF_Y; y++, i_i += Y_OFF) {
        for (x = 0; x < HALF_X; x++, i_i += 2, o_i += 1) {
            a = input[i_i];
            b = input[i_i + X_OFF];
            c = input[i_i + X_OFF + Y_OFF];
            d = input[i_i + Y_OFF];

            ab = a * (a == b | c == a);
            bc = b * (b == c);

            a = ab | bc;

            // output[o_i] = (a | (d & ((a != 0) - 1)));
            output[o_i] = a + (a == 0) * d;
        }
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: ./countless_if FILE.raw");
        return -1;
    }

    clock_t startT, endT;

    printf("start %lu\n", sizeof(int));

    uint8_t* input = (uint8_t*)malloc(VOX_COUNT * sizeof(uint8_t));
    uint8_t* output = (uint8_t*)malloc(DS_VOX_COUNT * sizeof(uint8_t));

    printf("allocated arrays\n");

    FILE *readPtr = fopen(argv[1], "rb");
    startT = clock();
    fread(input, sizeof(uint8_t), VOX_COUNT, readPtr);
    endT = clock();
    float sec = (endT - startT) / (float)CLOCKS_PER_SEC;
    printf("finished reading!, time: %f\n", sec);
    fclose(readPtr);

    int multipler = 5000;
    float acc = 0;
    for (int j = 0; j < 1; j++) {
        startT = clock();
        for (int i = 0; i < multipler; i++) {
            downsample(input, output);
        }
        endT = clock();
        sec = (endT - startT) / (float)CLOCKS_PER_SEC;
        float cur = VOX_COUNT / (1000000) / sec * (float)multipler;
        acc += cur;
        printf("MPx/sec time: %f\n", acc / (j + 1));
    }

    free(input);

    FILE *writePtr;
    writePtr = fopen("countless_output.bin", "wb");
    fwrite(output, sizeof(uint8_t), DS_VOX_COUNT, writePtr);
    fclose(writePtr);

    free(output);
    return 0;
}
