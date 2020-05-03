#include "./simple_bmp/simple_bmp.h"
#define TAM 256
#define RADIUS 1500
#define K 1.5
#define L 40
#define K_SIZE 42


int32_t getModule(int32_t x,int32_t y);

uint8_t linearFiltering(uint8_t );

void   convFiltering(int32_t x,int32_t y,uint16_t**,int32_t);

void kernel_setup (uint16_t **, int16_t);

int32_t getKernelSum(uint16_t **);