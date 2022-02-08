#include "constants.h"

char* const vram = (char*)0xa0000;
const int scrn_w = 320;
const int scrn_h = 200;
const int g_size = 0xffff;
uint32_t mem_size;
Queue mouse_buff = { 0 };
Queue keyboard_buff = { 0 };
Mouse mouse = {0, 150, 50};