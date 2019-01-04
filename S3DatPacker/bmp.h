#pragma once

#include "stdafx.h"

bool write_bitmap(char*, uint16_t, uint16_t, uint16_t*, char*, const pixel_format, const int);
int compress_bitmap(unsigned char*, char*, const file_type);
int gather_palette_colors(int16_t*, const char*);