#pragma once
#include "image.h"

uint8_t grass_init(void); 
void grass_reset(void); 
void grass_generate(void); 
void grass_draw_back(struct Image* image); 
void grass_draw_front(struct Image* image); 