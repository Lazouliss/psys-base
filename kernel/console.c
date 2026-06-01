#include "stdint.h"
#include "stdbool.h"

const uint16_t* VGABASE = (uint16_t*)0xB8000;

const uint16_t* ptr_mem(uint32_t lig, uint32_t col)
{
    return VGABASE + (lig * 80 + col);
}

void ecrit_car(uint32_t lig, uint32_t col, char c, uint8_t ct, uint8_t cf, bool cl)
{
    uint16_t* ptr = (uint16_t*)ptr_mem(lig, col);
    *ptr = (c << 8) | (cl << 7) | (cf << 4) | (ct); 
}