#include "stdafx.h"
#include "bmp.h"
#include "unpack.h"
#include "repack.h"

pixel_format pixelformat;
FILE *file = NULL;
FILE *test_file = NULL;
FILE* filelist = NULL;

const char* languages[8] = { "german/", "english/", "italian/", "french/", "polish/", "spanish/", "korean/", "schinese/" };

int unpack_gfx(char*, char*, char*, s3_dat_file_format);

void exit_exe(void)
{
	if (file)
		fclose(file);
	if (test_file)
		fclose(test_file);
	if (filelist)
		fclose(filelist);
}

int read_image(uint32_t address, uint32_t offset, char* file_name, char* name, file_type file_type, int16_t* offset_x, int16_t* offset_y)
{
	//"meta" reversing and parsing code from https://github.com/jsettlers/settlers-remake

	FILE* empty_file;
	uint16_t x;
	uint16_t y;
	uint16_t input;
	uint8_t cia_input;
	//uint8_t blue;
	//uint8_t green;
	//uint8_t red;
	uint16_t currentMeta;
	int16_t posx;
	int16_t posy;

	int rgb24 = 0;

	fseek(file, address + offset, 0);
	switch (file_type) {
	case TEXTURE:
		fread(&x, sizeof(x), 1, file);
		fread(&y, sizeof(y), 1, file);
		fread(offset_x, 1, 1, file);
		fread(offset_y, 1, 1, file);
		break;
	case MENU:
		fread(&x, sizeof(x), 1, file);
		fread(&y, sizeof(y), 1, file);
		fread(offset_x, sizeof(posx), 1, file);
		fread(offset_y, sizeof(posy), 1, file);
		fseek(file, 1 + ((ftell(file) + 1) % 2), SEEK_CUR);
		break;
	case SPRITE:
		fseek(file, 4, SEEK_CUR);
		fread(&x, sizeof(x), 1, file);
		fread(&y, sizeof(y), 1, file);
		fread(offset_x, sizeof(posx), 1, file);
		fread(offset_y, sizeof(posy), 1, file);
		fseek(file, 1 + ((ftell(file) + 1) % 2), SEEK_CUR);
		break;
	case CISPRITE:
		rgb24 = 1;
		fseek(file, 4, SEEK_CUR);
		fread(&x, sizeof(x), 1, file);
		fread(&y, sizeof(y), 1, file);
		fread(offset_x, sizeof(posx), 1, file);
		fread(offset_y, sizeof(posy), 1, file);
		fseek(file, 1 + ((ftell(file) + 1) % 2), SEEK_CUR);
		break;
	case SHADOW:
		fseek(file, 4, SEEK_CUR);
		fread(&x, sizeof(x), 1, file);
		fread(&y, sizeof(y), 1, file);
		fread(offset_x, sizeof(posx), 1, file);
		fread(offset_y, sizeof(posy), 1, file);
		fseek(file, 1 + ((ftell(file) + 1) % 2), SEEK_CUR);
		break;
	case ANIMATION:
		break;
	case PALETTE:
		x = 256;
		y = 8;
		break;
	default:
		break;
	}

	//check if size is valid
	if (x == 0 || y == 0) {
		empty_file = fopen(file_name, "wb");
		fclose(empty_file);
		return 2;
	}

	uint16_t* color_buffer;

	if (file_type == CISPRITE) {
		color_buffer = malloc(sizeof(uint16_t)*x*y);
		for (int i = 0; i < x*y; i++) {
			if (pixelformat == RGB555) {
				color_buffer[i] = 0x7c1f;
			}
			else {
				color_buffer[i] = 0xf81f;
			}
		}
	}
	else {
		color_buffer = calloc(x*y, 2);
	}

	if (file_type != PALETTE) {
		for (int i = 0; i < y; i++) {

			bool newLine = false;
			int idx = 0;
			while (!newLine) {
				if (fread(&currentMeta, sizeof(currentMeta), 1, file) < 1) {
					empty_file = fopen(file_name, "wb");
					fclose(empty_file);
					free(color_buffer);
					return 2;
				}
				int sequenceLength = currentMeta & 0xff;
				int skip = (currentMeta & 0x7f00) >> 8;
				newLine = (currentMeta & 0x8000) != 0;
				int skipend = idx + skip;
				while (idx < skipend) {
					idx++;
				}
				int readPartEnd = idx + sequenceLength;
				
				while (idx < readPartEnd) {
					if (file_type == SHADOW) {
						switch (pixelformat) {
						case RGB555:
							input = 0x7c1f;
							break;
						case RGB565:
							input = 0xf81f;
							break;
						default:
							break;
						}

					}
					else if (file_type == CISPRITE) {
						if (fread(&cia_input, 1, 1, file) < 1) {
							empty_file = fopen(file_name, "wb");
							fclose(empty_file);
							free(color_buffer);
							return 2;
						}
						input = cia_input;
					}
					else {
						fread(&input, sizeof(input), 1, file);
					}


					//switch (pixelformat) {
					//	case RGB555:
					//		red = (input & 0x7C00) >> 7;
					//		green = (input & 0x03E0) >> 2;
					//		blue = (input & 0x1F) << 3;

					//		blue |= blue >> 5;
					//		green |= green >> 5;
					//		red |= red >> 5;
					//		break;
					//	case RGB565:
					//		blue = (input & 0x1f) << 3;
					//		green = (input & 0x7e0) >> 3;
					//		red = (input & 0xf800) >> 8;

					//		blue |= blue >> 5;
					//		green |= green >> 6;
					//		red |= red >> 5;
					//		break;
					//	default:
					//		return 0;
					//}

					color_buffer[idx + i * x] = input;
					idx++;
				}
			}

		}
	}
	else {
		for (int i = 0; i < x*y; i++) {
			fread(&input, sizeof(input), 1, file);
			color_buffer[i] = input;
		}
	}
	
	int success = write_bitmap(file_name, x, y, color_buffer, name, pixelformat, rgb24);
	free(color_buffer);
	return success;
}

int main(int argc, char **argv)
{
	s3_dat_file_format file_data;

	atexit(exit_exe);

	printf("S3DatPacker v0.5\n");

	if (argc < 3)
	{
		printf("Usage: S3DatPacker <option> <filename>\n");
		printf("Options:\n-u Unpack dat archive\n-r Repack dat archive\n");
		return 1;
	}

	char* filename = strrchr(argv[2], '\\');
	if (!filename) {
		filename = argv[2];
	}
	else {
		filename++;
	}

	char extract_folder[256];
	strcpy(extract_folder, filename);
	strcat(extract_folder, ".extract");
	_mkdir(extract_folder);

	//FILE* temp = NULL;

	char file_name[256];
	strcpy(file_name, extract_folder);
	strcat(file_name, "/filelist.xml");

	if (!strcmp(argv[1], "-u")) {
		file = fopen(argv[2], "rb");

		if (!file)
		{
			printf("Cannot open file \"%s\": %s", argv[2], strerror(errno));
			return 1;
		}

		fread(&file_data.main_header, sizeof(s3_dat_main_header), 1, file);
		if (file_data.main_header.magic != 0x41304) {
			printf("%s: Unsupported file\n", argv[2]);
			return 1;
		}

		fseek(file, file_data.main_header.textures_ptr, 0);
		fread(&file_data.textures_header, sizeof(s3_dat_subheader), 1, file);
		fseek(file, file_data.main_header.menu_ptr, 0);
		fread(&file_data.menu_header, sizeof(s3_dat_subheader), 1, file);
		fseek(file, file_data.main_header.sprites_ptr, 0);
		fread(&file_data.sprites_header, sizeof(s3_dat_subheader), 1, file);
		fseek(file, file_data.main_header.cisprites_ptr, 0);
		fread(&file_data.cisprites_header, sizeof(s3_dat_subheader), 1, file);
		fseek(file, file_data.main_header.shadows_ptr, 0);
		fread(&file_data.shadows_header, sizeof(s3_dat_subheader), 1, file);
		fseek(file, file_data.main_header.animations_ptr, 0);
		fread(&file_data.animation_header, sizeof(s3_dat_subheader), 1, file);
		fseek(file, file_data.main_header.palette_ptr, 0);
		fread(&file_data.palette_header, sizeof(s3_dat_subheader), 1, file);
		fseek(file, file_data.main_header.text_ptr, 0);
		fread(&file_data.text_header, sizeof(text_subheader), 1, file);


		if (determine_pixelformat(file_data.main_header)) {
			printf("Error: Unsupported pixel format\n");
			return 0;
		}

		unpack_gfx(argv[1], extract_folder, file_name, file_data);
	}
	else if (!strcmp(argv[1], "-r")) {
		repack(argv[1], extract_folder, file_name);
	}
	else {
		printf("Usage: S3DatPacker <option> <filename>\n");
		printf("Options:\n-u Unpack dat archive\n-r Repack dat archive\n");
		return 1;
	}

	return 0;
}

int determine_pixelformat(s3_dat_main_header main_header) {
	if (main_header.bits_red == 0x7c00 && main_header.bits_green == 0x3e0 && main_header.bits_blue == 0x1f) {
		pixelformat = RGB555;
		return 0;
	}
	if (main_header.bits_red == 0xf800 && main_header.bits_green == 0x7e0 && main_header.bits_blue == 0x1f) {
		pixelformat = RGB565;
		return 0;
	}
	return 1;
}

int unpack_gfx(char* file_path, char* extract_folder, char* file_name, s3_dat_file_format file_data) {

	int c = 0;
	uint32_t address;
	uint32_t offset;
	uint8_t num_elem;
	int16_t offset_x = 0;
	int16_t offset_y = 0;
	int file_pos = 0;

	filelist = fopen(file_name, "w");

	printf("Unpacking...\n");

	if (!filelist) {
		printf("Cannot create filelist \"%s\": %s", file_name, strerror(errno));
		return 1;
	}

	char digit_buffer[12];
	char* internal_name;

	fwrite("<DATGFX Format=\"", 1, 16, filelist);
	sprintf(digit_buffer, "%d", pixelformat);
	fwrite(&digit_buffer, 1, 1, filelist);
	fwrite("\">\n", 1, 3, filelist);

	if (file_data.sprites_header.offset_count > 0) {
		fwrite("\t<SPRITES EntityCount=\"", 1, 23, filelist);
		sprintf(digit_buffer, "%d", file_data.sprites_header.offset_count);
		fwrite(&digit_buffer, 1, strlen(digit_buffer), filelist);
		fwrite("\">\n", 1, 3, filelist);
	}

	for (unsigned i = 0; i<file_data.sprites_header.offset_count; ++i)
	{
		fseek(file, file_data.main_header.sprites_ptr + 8 + i * 4, 0);
		fread(&address, sizeof(address), 1, file);
		fseek(file, address + 7, 0);
		fread(&num_elem, sizeof(num_elem), 1, file);
		for (unsigned j = 0; j < num_elem; j++) {
			fseek(file, address + 8 + j * 4, 0);
			fread(&offset, sizeof(offset), 1, file);
			char file_name[256];
			strcpy(file_name, extract_folder);
			strcat(file_name, "/sprites/");
			_mkdir(file_name);
			strcat(file_name, "sprite");
			sprintf(digit_buffer, "%d", i);
			strcat(file_name, digit_buffer);
			strcat(file_name, "_");
			sprintf(digit_buffer, "%d", j);
			strcat(file_name, digit_buffer);
			strcat(file_name, ".bmp");

			if (!read_image(address, offset, file_name, file_path, SPRITE, &offset_x, &offset_y)) {
				printf("Couldn't write sprite (%i, %i)\nAborting.\n", i, j);
				return 0;
			}
			else {
				internal_name = strchr(file_name, '/');
				fwrite("\t\t<SPRITE EntityId=\"", 1, 20, filelist);
				sprintf(digit_buffer, "%d", i);
				fwrite(digit_buffer, 1, strlen(digit_buffer), filelist);
				fwrite("\" OffsetX=\"", 1, 11, filelist);
				sprintf(digit_buffer, "%d", offset_x);
				fwrite(digit_buffer, 1, strlen(digit_buffer), filelist);
				fwrite("\" OffsetY=\"", 1, 11, filelist);
				sprintf(digit_buffer, "%d", offset_y);
				fwrite(digit_buffer, 1, strlen(digit_buffer), filelist);
				fwrite("\" Path=\"", 1, 8, filelist);
				fwrite(internal_name, 1, strlen(internal_name), filelist);
				fwrite("\"/>\n", 1, 4, filelist);
				++c;
			}
		}

	}
	

	if (file_data.sprites_header.offset_count > 0) {
		printf("%d Sprites unpacked\n", c);
		fwrite("\t</SPRITES>\n", 1, 12, filelist);
	}

	if (file_data.textures_header.offset_count > 0) {
		fwrite("\t<TEXTURES EntityCount=\"", 1, 24, filelist);
		sprintf(digit_buffer, "%d", file_data.textures_header.offset_count);
		fwrite(digit_buffer, 1, strlen(digit_buffer), filelist);
		fwrite("\">\n", 1, 3, filelist);
	}

	c = 0;
	for (unsigned i = 0; i<file_data.textures_header.offset_count; ++i)
	{

		fseek(file, file_data.main_header.textures_ptr + 8 + i * 4, 0);
		fread(&address, sizeof(address), 1, file);
		char file_name[256];
		strcpy(file_name, extract_folder);
		strcat(file_name, "/textures/");
		_mkdir(file_name);
		strcat(file_name, "texture");
		sprintf(digit_buffer, "%d", i);
		strcat(file_name, digit_buffer);
		strcat(file_name, ".bmp");
		if (!read_image(address, 0, file_name, file_path, TEXTURE, &offset_x, &offset_y)) {
			printf("Couldn't write texture %i\nAborting.\n", i);
			return 0;
		}
		else {
			internal_name = strchr(file_name, '/');
			fwrite("\t\t<TEXTURE EntityId=\"", 1, 21, filelist);
			sprintf(digit_buffer, "%d", i);
			fwrite(digit_buffer, 1, strlen(digit_buffer), filelist);
			fwrite("\" OffsetX=\"", 1, 11, filelist);
			sprintf(digit_buffer, "%d", (int8_t)offset_x);
			fwrite(digit_buffer, 1, strlen(digit_buffer), filelist);
			fwrite("\" OffsetY=\"", 1, 11, filelist);
			sprintf(digit_buffer, "%d", (int8_t)offset_y);
			fwrite(digit_buffer, 1, strlen(digit_buffer), filelist);
			fwrite("\" Path=\"", 1, 8, filelist);
			fwrite(internal_name, 1, strlen(internal_name), filelist);
			fwrite("\"/>\n", 1, 4, filelist);
			++c;
		}
	}
	

	if (file_data.textures_header.offset_count > 0) {
		printf("%d Textures unpacked\n", c);
		fwrite("\t</TEXTURES>\n", 1, 13, filelist);
	}

	if (file_data.menu_header.offset_count > 0) {
		fwrite("\t<MENUS EntityCount=\"", 1, 21, filelist);
		sprintf(digit_buffer, "%d", file_data.menu_header.offset_count);
		fwrite(digit_buffer, 1, strlen(digit_buffer), filelist);
		fwrite("\">\n", 1, 3, filelist);
	}

	c = 0;
	for (unsigned i = 0; i<file_data.menu_header.offset_count; ++i)
	{

		fseek(file, file_data.main_header.menu_ptr + 8 + i * 4, 0);
		fread(&address, sizeof(address), 1, file);
		char file_name[256];
		strcpy(file_name, extract_folder);
		strcat(file_name, "/menu/");
		_mkdir(file_name);
		strcat(file_name, "menu");
		sprintf(digit_buffer, "%d", i);
		strcat(file_name, digit_buffer);
		strcat(file_name, ".bmp");
		//printf("adres=%d\n", file_data.main_header.terrain_ptr);
		//printf("adres=%d\n", address);
		if (!read_image(address, 0, file_name, file_path, MENU, &offset_x, &offset_y)) {
			printf("Couldn't write menu texture %i\nAborting.\n", i);
			return 0;
		}
		else {
			internal_name = strchr(file_name, '/');
			fwrite("\t\t<MENU EntityId=\"", 1, 18, filelist);
			sprintf(digit_buffer, "%d", i);
			fwrite(digit_buffer, 1, strlen(digit_buffer), filelist);
			fwrite("\" OffsetX=\"", 1, 11, filelist);
			sprintf(digit_buffer, "%d", offset_x);
			fwrite(digit_buffer, 1, strlen(digit_buffer), filelist);
			fwrite("\" OffsetY=\"", 1, 11, filelist);
			sprintf(digit_buffer, "%d", offset_y);
			fwrite(digit_buffer, 1, strlen(digit_buffer), filelist);
			fwrite("\" Path=\"", 1, 8, filelist);
			fwrite(internal_name, 1, strlen(internal_name), filelist);
			fwrite("\"/>\n", 1, 4, filelist);
			++c;
		}

	}

	if (file_data.menu_header.offset_count > 0) {
		fwrite("\t</MENUS>\n", 1, 10, filelist);
		printf("%d Menu Textures unpacked\n", c);
	}

	if (file_data.cisprites_header.offset_count > 0) {
		fwrite("\t<CISPRITES EntityCount=\"", 1, 25, filelist);
		sprintf(digit_buffer, "%d", file_data.cisprites_header.offset_count);
		fwrite(digit_buffer, 1, strlen(digit_buffer), filelist);
		fwrite("\">\n", 1, 3, filelist);
	}

	c = 0;
	for (unsigned i = 0; i<file_data.cisprites_header.offset_count; ++i)
	{
		fseek(file, file_data.main_header.cisprites_ptr + 8 + i * 4, 0);
		fread(&address, sizeof(address), 1, file);
		fseek(file, address + 7, 0);
		fread(&num_elem, sizeof(num_elem), 1, file);
		for (unsigned j = 0; j < num_elem; j++) {
			fseek(file, address + 8 + j * 4, 0);
			fread(&offset, sizeof(offset), 1, file);
			char file_name[256];
			strcpy(file_name, extract_folder);
			strcat(file_name, "/ci_sprites/");
			_mkdir(file_name);
			strcat(file_name, "cisprite");
			sprintf(digit_buffer, "%d", i);
			strcat(file_name, digit_buffer);
			strcat(file_name, "_");
			sprintf(digit_buffer, "%d", j);
			strcat(file_name, digit_buffer);
			strcat(file_name, ".bmp");
			if (!read_image(address, offset, file_name, file_path, CISPRITE, &offset_x, &offset_y)) {
				printf("Couldn't write Color-Indexed Sprite (%i, %i)\nAborting.\n", i, j);
				return 0;
			}
			else {
				internal_name = strchr(file_name, '/');
				fwrite("\t\t<CISPRITE EntityId=\"", 1, 22, filelist);
				sprintf(digit_buffer, "%d", i);
				fwrite(digit_buffer, 1, strlen(digit_buffer), filelist);
				fwrite("\" OffsetX=\"", 1, 11, filelist);
				sprintf(digit_buffer, "%d", offset_x);
				fwrite(digit_buffer, 1, strlen(digit_buffer), filelist);
				fwrite("\" OffsetY=\"", 1, 11, filelist);
				sprintf(digit_buffer, "%d", offset_y);
				fwrite(digit_buffer, 1, strlen(digit_buffer), filelist);
				fwrite("\" Path=\"", 1, 8, filelist);
				fwrite(internal_name, 1, strlen(internal_name), filelist);
				fwrite("\"/>\n", 1, 4, filelist);
				++c;
			}
		}

	}

	if (file_data.cisprites_header.offset_count > 0) {
		printf("%d Color-Indexed Sprites unpacked\n", c);
		fwrite("\t</CISPRITES>\n", 1, 14, filelist);
	}

	if (file_data.shadows_header.offset_count > 0) {
		fwrite("\t<SHADOWS EntityCount=\"", 1, 23, filelist);
		sprintf(digit_buffer, "%d", file_data.shadows_header.offset_count);
		fwrite(digit_buffer, 1, strlen(digit_buffer), filelist);
		fwrite("\">\n", 1, 3, filelist);
	}

	c = 0;
	for (unsigned i = 0; i<file_data.shadows_header.offset_count; ++i)
	{
		fseek(file, file_data.main_header.shadows_ptr + 8 + i * 4, 0);
		fread(&address, sizeof(address), 1, file);
		fseek(file, address + 7, 0);
		fread(&num_elem, sizeof(num_elem), 1, file);
		for (unsigned j = 0; j < num_elem; j++) {
			fseek(file, address + 8 + j * 4, 0);
			fread(&offset, sizeof(offset), 1, file);
			char file_name[256];
			strcpy(file_name, extract_folder);
			strcat(file_name, "/shadows/");
			_mkdir(file_name);
			strcat(file_name, "shadow");
			sprintf(digit_buffer, "%d", i);
			strcat(file_name, digit_buffer);
			strcat(file_name, "_");
			sprintf(digit_buffer, "%d", j);
			strcat(file_name, digit_buffer);
			strcat(file_name, ".bmp");
			if (!read_image(address, offset, file_name, file_path, SHADOW, &offset_x, &offset_y)) {
				printf("Couldn't write shadow (%i, %i)\nAborting.\n", i, j);
				return 0;
			}
			else {
				internal_name = strchr(file_name, '/');
				fwrite("\t\t<SHADOW EntityId=\"", 1, 20, filelist);
				sprintf(digit_buffer, "%d", i);
				fwrite(digit_buffer, 1, strlen(digit_buffer), filelist);
				fwrite("\" OffsetX=\"", 1, 11, filelist);
				sprintf(digit_buffer, "%d", offset_x);
				fwrite(digit_buffer, 1, strlen(digit_buffer), filelist);
				fwrite("\" OffsetY=\"", 1, 11, filelist);
				sprintf(digit_buffer, "%d", offset_y);
				fwrite(digit_buffer, 1, strlen(digit_buffer), filelist);
				fwrite("\" Path=\"", 1, 8, filelist);
				fwrite(internal_name, 1, strlen(internal_name), filelist);
				fwrite("\"/>\n", 1, 4, filelist);
				++c;
			}
		}

	}
	

	if (file_data.shadows_header.offset_count > 0) {
		fwrite("\t</SHADOWS>\n", 1, 12, filelist);
		printf("%d Shadows unpacked\n", c);
	}

	if (file_data.palette_header.offset_count > 0) {
		fwrite("\t<PALETTES EntityCount=\"", 1, 24, filelist);
		sprintf(digit_buffer, "%d", file_data.palette_header.offset_count);
		fwrite(digit_buffer, 1, strlen(digit_buffer), filelist);
		fwrite("\">\n", 1, 3, filelist);
	}

	c = 0;
	for (unsigned i = 0; i<file_data.palette_header.offset_count; ++i)
	{
		fseek(file, file_data.main_header.palette_ptr + 12 + i * 4, 0);
		fread(&address, sizeof(address), 1, file);
		char file_name[256];
		strcpy(file_name, extract_folder);
		strcat(file_name, "/palettes/");
		_mkdir(file_name);
		strcat(file_name, "palette");
		sprintf(digit_buffer, "%d", i);
		strcat(file_name, digit_buffer);
		strcat(file_name, ".bmp");
		if (!read_image(address, 0, file_name, file_path, PALETTE, &offset_x, &offset_y)) {
			printf("Couldn't write palette %i\nAborting.\n", i);
			return 0;
		}
		else {
			internal_name = strchr(file_name, '/');
			fwrite("\t\t<PALETTE EntityId=\"", 1, 21, filelist);
			sprintf(digit_buffer, "%d", i);
			fwrite(digit_buffer, 1, strlen(digit_buffer), filelist);
			fwrite("\" Path=\"", 1, 8, filelist);
			fwrite(internal_name, 1, strlen(internal_name), filelist);
			fwrite("\"/>\n", 1, 4, filelist);
			++c;
		}
	}

	if (file_data.palette_header.offset_count > 0) {
		fwrite("\t</PALETTES>\n", 1, 13, filelist);
		printf("%d Palettes unpacked\n", c);
	}

	if (file_data.animation_header.offset_count > 0) {
		fwrite("\t<ANIMATIONS EntityCount=\"", 1, 26, filelist);
		sprintf(digit_buffer, "%d", file_data.animation_header.offset_count);
		fwrite(digit_buffer, 1, strlen(digit_buffer), filelist);
		fwrite("\">\n", 1, 3, filelist);
	}

	c = 0;
	for (unsigned i = 0; i<file_data.animation_header.offset_count; ++i)
	{
		fseek(file, file_data.main_header.animations_ptr + 8 + i * 4, 0);
		fread(&address, sizeof(address), 1, file);
		char file_name[256];
		strcpy(file_name, extract_folder);
		strcat(file_name, "/animations/");
		_mkdir(file_name);
		strcat(file_name, "animation");
		sprintf(digit_buffer, "%d", i);
		strcat(file_name, digit_buffer);
		strcat(file_name, ".dat");
		int anim_length = 0;
		fseek(file, address, 0);
		fread(&anim_length, 4, 1, file);
		if (!anim_length) {
			printf("Animation %i is empty!\nAborting.\n", i);
			return 0;
		}
		else {
			FILE* anim_file;
			int* anim_buffer = (int*)malloc(sizeof(int) * (anim_length * 6 + 1));
			anim_buffer[0] = anim_length;
			fread(anim_buffer + 1, 4, 6 * anim_length, file);
			anim_file = fopen(file_name, "wb");
			if (!anim_file) {
				printf("Couldn't write animation %i\nAborting.\n", i);
				return 0;
			}
			else {
				fwrite(anim_buffer, 4, 6 * anim_length + 1, anim_file);
				free(anim_buffer);
				fclose(anim_file);
				internal_name = strchr(file_name, '/');
				fwrite("\t\t<ANIMATION EntityId=\"", 1, 23, filelist);
				sprintf(digit_buffer, "%d", i);
				fwrite(digit_buffer, 1, strlen(digit_buffer), filelist);
				fwrite("\" Path=\"", 1, 8, filelist);
				fwrite(internal_name, 1, strlen(internal_name), filelist);
				fwrite("\"/>\n", 1, 4, filelist);
				++c;
			}
		}
	}

	if (file_data.animation_header.offset_count > 0) {
		fwrite("\t</ANIMATIONS>\n", 1, 15, filelist);
		printf("%d Animations unpacked\n", c);
	}

	if (file_data.text_header.string_count > 0) {
		fwrite("\t<TEXTS EntityCount=\"", 1, 21, filelist);
		sprintf(digit_buffer, "%d", file_data.text_header.string_count);
		fwrite(digit_buffer, 1, strlen(digit_buffer), filelist);
		fwrite("\">\n", 1, 3, filelist);
	}

	c = 0;
	char chr;
	char* string;
	int length = 0;
	for (int i = 0; i < file_data.text_header.language_count; i++) {
		for (unsigned j = 0; j < file_data.text_header.string_count; ++j) {
			fseek(file, file_data.main_header.text_ptr + 12 + j * 4 + i * file_data.text_header.string_count * 4, 0);
			fread(&offset, 4, 1, file);
			fseek(file, offset, 0);

			while (fread(&chr, 1, 1, file)) {
				if (chr != '\0') {
					length++;
				}
				else {
					break;
				}
			}

			string = malloc(length);
			fseek(file, offset, 0);
			fread(string, 1, length, file);

			char file_name[256];
			strcpy(file_name, extract_folder);
			strcat(file_name, "/texts/");
			_mkdir(file_name);
			strcat(file_name, languages[i]);
			_mkdir(file_name);
			strcat(file_name, "text");
			sprintf(digit_buffer, "%d", j);
			strcat(file_name, digit_buffer);
			strcat(file_name, ".txt");

			FILE* text_file;
			text_file = fopen(file_name, "w");

			if (!text_file) {
				printf("Couldn't write text %i for language %i\nAborting.\n", j, i);
				return 0;
			}
			else {
				fwrite(string, 1, length, text_file);
				fclose(text_file);
			}

			free(string);
			length = 0;

			internal_name = strchr(file_name, '/');
			fwrite("\t\t<TEXT EntityId=\"", 1, 18, filelist);
			sprintf(digit_buffer, "%d", j);
			fwrite(digit_buffer, 1, strlen(digit_buffer), filelist);
			fwrite("\" Path=\"", 1, 8, filelist);
			fwrite(internal_name, 1, strlen(internal_name), filelist);
			fwrite("\"/>\n", 1, 4, filelist);

			c++;
		}
	}

	if (file_data.text_header.string_count > 0) {
		fwrite("\t</TEXTS>\n", 1, 10, filelist);
		printf("%d Texts unpacked\n", c);
	}

	fwrite("</DATGFX>", 1, 9, filelist);

	return 0;
}