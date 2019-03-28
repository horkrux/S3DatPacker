#pragma once

int repack(char*, char*, char*);
static int determine_dat_type(dat_type*, FILE*);
static int gather_contents(FILE*, const char*);
static int write_main_header();
static int write_sub_header(const file_type, const int);
static int write_section_simple(const file_type, const char*, const int, char**);
static int write_section_menu(const char*, const int, char**, int16_t*, int16_t*);
static int write_section_texture(const char*, const int, char**, int8_t*);
static int write_section(const file_type, const char*, const int, char**, int*, int16_t*, int16_t*);