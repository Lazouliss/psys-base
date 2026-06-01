#include "stdint.h"

const uint16_t* VGABASE = (uint16_t*)0xB8000;

const uint16_t* ptr_mem(uint32_t lig, uint32_t col)
{
    return VGABASE + (lig * 80 + col);
}
