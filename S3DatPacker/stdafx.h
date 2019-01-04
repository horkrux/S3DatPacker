// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <tchar.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <direct.h>

#pragma pack(push)
#pragma pack(1)



typedef enum {
	GFX,
	SND
} dat_type;

typedef enum {
	TEXT
	,TEXTURE
	, MENU
	, SPRITE
	, CISPRITE
	, SHADOW
	, ANIMATION
	, PALETTE
} file_type;

typedef enum {
	RGB555,
	RGB565
} pixel_format;

typedef struct
{
	uint32_t magic;
	uint32_t unk0; //C
	uint32_t unk1; //0
	uint32_t header_size;
	uint32_t unk2; //20
	uint32_t unk3; //40
	uint32_t unk4; //0
	uint32_t unk5; //10
	uint32_t bits_red;
	uint32_t bits_green;
	uint32_t bits_blue;
	uint32_t unk6; //0
	uint32_t file_size;
	uint32_t text_ptr;
	uint32_t textures_ptr;
	uint32_t menu_ptr;
	uint32_t sprites_ptr;
	uint32_t cisprites_ptr;
	uint32_t shadows_ptr;
	uint32_t animations_ptr;
	uint32_t palette_ptr;
} s3_dat_main_header;

typedef struct
{
	uint32_t magic; //textures: 2412, palette: 2607, menu: 11306, sprites: 106, cisprites: 3112, shadows: 5982, animations: 21702
	uint16_t header_size;
	uint16_t offset_count;
} s3_dat_subheader;

typedef struct {
	uint32_t magic; //1904
	uint32_t header_size;
	uint16_t string_count;
	uint16_t language_count;
} text_subheader;

typedef struct {
	uint32_t magic; //2607
	uint16_t header_size;
	uint16_t offset_count;
	uint32_t palette_size; //256
} palette_subheader;

typedef struct {
	uint32_t magic; //1402
	uint16_t unk0;
	uint8_t unk1;
	uint8_t offset_count;
} s3_dat_subsubheader;

typedef struct
{
	s3_dat_main_header main_header;
	text_subheader text_header;
	s3_dat_subheader textures_header;
	s3_dat_subheader menu_header;
	s3_dat_subheader sprites_header;
	s3_dat_subheader cisprites_header;
	s3_dat_subheader shadows_header;
	s3_dat_subheader animation_header;
	palette_subheader palette_header;
} s3_dat_file_format;


#pragma pack(pop)