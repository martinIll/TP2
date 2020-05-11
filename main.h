#include "./simple_bmp/simple_bmp.h"
#include <omp.h>
#define TAM 256
#define K_SIZE 41


int32_t getModule(int32_t x,int32_t y);

uint8_t linearFiltering(uint8_t,float,float);

void   convFiltering(int32_t x,int32_t y,uint16_t**,int32_t);

void kernel_setup (uint16_t **, int16_t);

int32_t getKernelSum(uint16_t **);