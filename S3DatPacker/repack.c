#include "stdafx.h"
#include "repack.h"
#include "bmp.h"

FILE* test;
int file_pos = 0x54;
static pixel_format pixelformat;



int repack(char* path, char* extract_folder, char* file_name) {
	FILE* filelist;
	dat_type type = SND;

	printf("Repacking...\n");

	test = fopen("test.dat", "wb");

	filelist = fopen(file_name, "r");

	if (!filelist) {
		printf("Couldn't load filelist\n");
	}
	else {

		int worked = determine_dat_type(&type, filelist); //lol

		if (gather_contents(filelist, extract_folder)) printf("Something went wrong\n");

		fclose(filelist);
	}
	fclose(test);

	return 0;
}

static int determine_dat_type(dat_type* type, FILE* filelist) {
	char buffer[256];
	if (fgets(buffer, 256, filelist) != NULL) {
		if (strstr(buffer, "<DATGFX") != NULL) {
			*type = GFX;
			if (!sscanf_s(buffer, "<DATGFX Format=\"%i\">", &pixelformat)) {
				pixelformat = RGB565; //use default
			}
		}
		else if (strstr(buffer, "<DATSND>") != NULL) {
			*type = SND;
		}
		else {
			return 1;
		}
	}
	else {
		return 1;
	}
	return 0;
}

static int gather_contents(FILE* filelist, const char* extract_folder) {
	char** entity_paths;
	int* entity_subnum;
	int16_t* offsets_x;
	int16_t* offsets_y;
	int8_t* offsets_x_8;
	int8_t* offsets_y_8;
	int current_idx = 0;
	int prev_idx = 0;
	int elem_count = 0;
	int path_idx = 0;
	int entity_count = 0;
	int language_count = 0;
	short offset_x = 0;
	short offset_y = 0;
	

	char path_buffer[256];
	char section_buffer[256];

	if (write_main_header()) return 1;

	while (fgets(section_buffer, 256, filelist) != NULL) {
		if (strstr(section_buffer, "<TEXTS ") != NULL) {
			sscanf_s(section_buffer, "%*s EntityCount=\"%i\"", &entity_count);
			entity_paths = malloc(sizeof(char*) * entity_count * 8);

			while (fgets(section_buffer, 256, filelist) != NULL) {
				if (strstr(section_buffer, "<TEXT ") != NULL) {
					sscanf_s(section_buffer, "%*s EntityId=\"%i\" Path=\"%[^\"]", &current_idx, &path_buffer, 256);
					entity_paths[path_idx] = malloc(256);
					strcpy(entity_paths[path_idx], path_buffer);
					path_idx++;
					elem_count++;
					if (elem_count == entity_count) {
						elem_count = 0;
						language_count++;
					}
					
				}
				else {
					if (elem_count) {
						printf("Text missing\n");
						for (int i = 0; i < path_idx; i++) {
							free(entity_paths[path_idx]);
						}
						free(entity_paths);
						return 1;
					}
					break;
				}
			}
			
			if (write_section_simple(TEXT, extract_folder, entity_count*8, entity_paths)) return 1;

			path_idx = 0;
			entity_count = 0;
			current_idx = 0;
		}
		else if (strstr(section_buffer, "<TEXTURES ") != NULL) {
			sscanf_s(section_buffer, "%*s EntityCount=\"%i\"", &entity_count);
			entity_paths = malloc(sizeof(char*) * entity_count);
			offsets_x_8 = malloc(sizeof(short)*entity_count);
			//offsets_y_8 = malloc(sizeof(short)*entity_count);

			while (fgets(section_buffer, 256, filelist) != NULL) {
				if (strstr(section_buffer, "<TEXTURE ") != NULL) {
					sscanf_s(section_buffer, "%*s EntityId=\"%i\" Type=\"%hi\" Path=\"%[^\"]", &current_idx, &offset_x, &path_buffer, 256);
					offsets_x_8[path_idx] = offset_x;
					//offsets_y_8[path_idx] = offset_y;
					entity_paths[path_idx] = malloc(256);
					strcpy(entity_paths[path_idx], path_buffer);
					path_idx++;
				}
				else {
					break;
				}
			}

			if (write_section_texture(extract_folder, entity_count, entity_paths, offsets_x_8)) return 1;

			path_idx = 0;
			entity_count = 0;
			current_idx = 0;
		}
		else if (strstr(section_buffer, "<MENUS ") != NULL) {
			sscanf_s(section_buffer, "%*s EntityCount=\"%i\"", &entity_count);
			entity_paths = malloc(sizeof(char*) * entity_count);
			offsets_x = malloc(sizeof(short)*entity_count);
			offsets_y = malloc(sizeof(short)*entity_count);

			while (fgets(section_buffer, 256, filelist) != NULL) {
				if (strstr(section_buffer, "<MENU ") != NULL) {
					sscanf_s(section_buffer, "%*s EntityId=\"%i\" OffsetX=\"%hi\" OffsetY=\"%hi\" Path=\"%[^\"]", &current_idx, &offset_x, &offset_y, &path_buffer, 256);
					offsets_x[path_idx] = offset_x;
					offsets_y[path_idx] = offset_y;
					entity_paths[path_idx] = malloc(256);
					strcpy(entity_paths[path_idx], path_buffer);
					path_idx++;
				}
				else {
					break;
				}
			}

			if (write_section_menu(extract_folder, entity_count, entity_paths, offsets_x, offsets_y)) return 1;

			path_idx = 0;
			entity_count = 0;
			current_idx = 0;
		}
		else if (strstr(section_buffer, "<SPRITES ") != NULL) {
			sscanf_s(section_buffer, "%*s EntityCount=\"%i\"", &entity_count);

			if (entity_count) {
				entity_subnum = malloc(sizeof(int) * entity_count);
				entity_paths = malloc(sizeof(char*) * entity_count * 300); // :s
				offsets_x = malloc(sizeof(short)*entity_count * 300);
				offsets_y = malloc(sizeof(short)*entity_count * 300);

				while (fgets(section_buffer, 256, filelist) != NULL) {
					if (strstr(section_buffer, "<SPRITE ") != NULL) {
						sscanf_s(section_buffer, "%*s EntityId=\"%i\" OffsetX=\"%hi\" OffsetY=\"%hi\" Path=\"%[^\"]", &current_idx, &offset_x, &offset_y, &path_buffer, 256);
						offsets_x[path_idx] = offset_x;
						offsets_y[path_idx] = offset_y;
						entity_paths[path_idx] = malloc(256);
						strcpy(entity_paths[path_idx], path_buffer);

						if (prev_idx != current_idx) {
							entity_subnum[prev_idx] = elem_count;
							elem_count = 1;
						}
						else {
							elem_count++;
						}
						path_idx++;
						prev_idx = current_idx;
					}
					else if (strstr(section_buffer, "</SPRITES>") != NULL) {
						entity_subnum[prev_idx] = elem_count;
						elem_count = 0;
						break;
					}
					else {
						break;
					}
				}

				if (write_section(SPRITE, extract_folder, entity_count, entity_paths, entity_subnum, offsets_x, offsets_y)) return 1;
			}

			entity_count = 0;
			current_idx = 0;
			prev_idx = 0;
			elem_count = 0;
			path_idx = 0;
		}
		else if (strstr(section_buffer, "<CISPRITES ") != NULL) {
			sscanf_s(section_buffer, "%*s EntityCount=\"%i\"", &entity_count);
			entity_subnum = malloc(sizeof(int) * entity_count);
			entity_paths = malloc(sizeof(char*) * entity_count * 300); // :s
			offsets_x = malloc(sizeof(short)*entity_count * 300);
			offsets_y = malloc(sizeof(short)*entity_count * 300);

			while (fgets(section_buffer, 256, filelist) != NULL) {
				if (strstr(section_buffer, "<CISPRITE ") != NULL) {
					sscanf_s(section_buffer, "%*s EntityId=\"%i\" OffsetX=\"%hi\" OffsetY=\"%hi\" Path=\"%[^\"]", &current_idx, &offset_x, &offset_y, &path_buffer, 256);
					offsets_x[path_idx] = offset_x;
					offsets_y[path_idx] = offset_y;
					entity_paths[path_idx] = malloc(256);
					strcpy(entity_paths[path_idx], path_buffer);

					if (prev_idx != current_idx) {
						entity_subnum[prev_idx] = elem_count;
						elem_count = 1;
					}
					else {
						elem_count++;
					}
					path_idx++;
					prev_idx = current_idx;
				}
				else if (strstr(section_buffer, "</CISPRITES>") != NULL) {
					entity_subnum[prev_idx] = elem_count;
					elem_count = 0;
					break;
				}
				else {
					break;
				}
			}
			

			if (write_section(CISPRITE, extract_folder, entity_count, entity_paths, entity_subnum, offsets_x, offsets_y)) return 1;

			entity_count = 0;
			current_idx = 0;
			prev_idx = 0;
			elem_count = 0;
			path_idx = 0;
		}
		else if (strstr(section_buffer, "<SHADOWS ") != NULL) {
			sscanf_s(section_buffer, "%*s EntityCount=\"%i\"", &entity_count);
			entity_subnum = malloc(sizeof(int) * entity_count);
			entity_paths = malloc(sizeof(char*) * entity_count * 300); // :s
			offsets_x = malloc(sizeof(short)*entity_count * 300);
			offsets_y = malloc(sizeof(short)*entity_count * 300);

			while (fgets(section_buffer, 256, filelist) != NULL) {
				if (strstr(section_buffer, "<SHADOW ") != NULL) {
					sscanf_s(section_buffer, "%*s EntityId=\"%i\" OffsetX=\"%hi\" OffsetY=\"%hi\" Path=\"%[^\"]", &current_idx, &offset_x, &offset_y, &path_buffer, 256);
					offsets_x[path_idx] = offset_x;
					offsets_y[path_idx] = offset_y;
					entity_paths[path_idx] = malloc(256);
					strcpy(entity_paths[path_idx], path_buffer);

					if (prev_idx != current_idx) {
						entity_subnum[prev_idx] = elem_count;
						elem_count = 1;
					}
					else {
						elem_count++;
					}
					path_idx++;
					prev_idx = current_idx;
				}
				else if (strstr(section_buffer, "</SHADOWS>") != NULL) {
					entity_subnum[prev_idx] = elem_count;
					elem_count = 0;
					break;
				}
				else {
					break;
				}
			}

			if (write_section(SHADOW, extract_folder, entity_count, entity_paths, entity_subnum, offsets_x, offsets_y)) return 1;
			entity_count = 0;
			current_idx = 0;
			prev_idx = 0;
			elem_count = 0;
			path_idx = 0;
		}
		else if (strstr(section_buffer, "<ANIMATIONS ") != NULL) {
			sscanf_s(section_buffer, "%*s EntityCount=\"%i\"", &entity_count);
			entity_paths = malloc(sizeof(char*) * entity_count);

			while (fgets(section_buffer, 256, filelist) != NULL) {
				if (strstr(section_buffer, "<ANIMATION ") != NULL) {
					sscanf_s(section_buffer, "%*s EntityId=\"%i\" Path=\"%[^\"]", &current_idx, &path_buffer, 256);
					entity_paths[path_idx] = malloc(256);
					strcpy(entity_paths[path_idx], path_buffer);
					path_idx++;
				}
				else {
					break;
				}
			}

			if (write_section_simple(ANIMATION, extract_folder, entity_count, entity_paths)) return 1;

			path_idx = 0;
			entity_count = 0;
			current_idx = 0;
		}
		else if (strstr(section_buffer, "<PALETTES ") != NULL) {
			sscanf_s(section_buffer, "%*s EntityCount=\"%i\"", &entity_count);
			entity_paths = malloc(sizeof(char*) * entity_count);

			while (fgets(section_buffer, 256, filelist) != NULL) {
				if (strstr(section_buffer, "<PALETTE ") != NULL) {
					sscanf_s(section_buffer, "%*s EntityId=\"%i\" Path=\"%[^\"]", &current_idx, &path_buffer, 256);
					entity_paths[path_idx] = malloc(256);
					strcpy(entity_paths[path_idx], path_buffer);
					path_idx++;
				}
				else {
					break;
				}
			}

			if (write_section_simple(PALETTE, extract_folder, entity_count, entity_paths)) return 1;

			path_idx = 0;
			entity_count = 0;
			current_idx = 0;
		}
	}
	//write file size
	fseek(test, 0x30, 0);
	fwrite(&file_pos, 4, 1, test);
	return 0;
}

int write_main_header() {
	s3_dat_main_header main_header;

	main_header.magic = 0x41304;
	main_header.unk0 = 0xc;
	main_header.unk1 = 0;
	main_header.header_size = 0x54;
	main_header.unk2 = 0x20;
	main_header.unk3 = 0x40;
	main_header.unk4 = 0;
	main_header.unk5 = 0x10;

	if (pixelformat == RGB555) {
		main_header.bits_red = 0x7c00;
		main_header.bits_green = 0x3e0;
		main_header.bits_blue = 0x1f;
	}
	else {
		main_header.bits_red = 0xf800;
		main_header.bits_green = 0x7e0;
		main_header.bits_blue = 0x1f;
	}

	main_header.unk6 = 0;
	main_header.file_size = 0;
	main_header.text_ptr = 0;
	main_header.textures_ptr = 0;
	main_header.menu_ptr = 0;
	main_header.sprites_ptr = 0;
	main_header.cisprites_ptr = 0;
	main_header.shadows_ptr = 0;
	main_header.animations_ptr = 0;
	main_header.palette_ptr = 0;

	if (!test) return 1;

	fwrite(&main_header, sizeof(main_header), 1, test);

	return 0;
}

static int write_section_simple(const file_type type, const char* extract_folder, const int object_count, char** entity_paths) {
	FILE* temp_file;
	char* temp_data;
	int temp_size = 0;
	char path_buffer[256];
	char null_term = '\0';

	int16_t color_buffer[2048];

	int section_ptr_offset = file_pos;
	switch (type) {
		case TEXT:
			fseek(test, 0x34, 0);
			break;
		case ANIMATION:
			fseek(test, 0x4c, 0);
			break;
		case PALETTE:
			fseek(test, 0x50, 0);
			break;
	}

	fwrite(&file_pos, 4, 1, test);

	fseek(test, file_pos, 0);

	int header_pos = write_sub_header(type, object_count);

	file_pos = ftell(test);
	for (int i = 0; i < object_count; i++) {
		fseek(test, header_pos, 0);
		fwrite(&file_pos, 4, 1, test);
		header_pos = ftell(test);
		fseek(test, file_pos, 0);

		strcpy(path_buffer, extract_folder);
		strcat(path_buffer, entity_paths[i]);

		if (type == PALETTE) {
			gather_palette_colors(color_buffer, path_buffer);
			fwrite(&color_buffer, 2, 2048, test);
		}
		else {
			if (type == TEXT) {
				temp_file = fopen(path_buffer, "r");
			}
			else {
				temp_file = fopen(path_buffer, "rb");
			}


			if (!temp_file) {
				printf("Couldn't open file %s %s\nAborting.\n", entity_paths[i], path_buffer);
				return 1;
			}

			fseek(temp_file, 0, SEEK_END);
			temp_size = ftell(temp_file);

			temp_data = malloc(temp_size);
			rewind(temp_file);
			fread(temp_data, 1, temp_size, temp_file);

			fwrite(temp_data, 1, temp_size, test);

			if (type == TEXT) fwrite(&null_term, 1, 1, test);

			free(temp_data);

			fclose(temp_file);
		}

		free(entity_paths[i]);

		file_pos = ftell(test);

	}

	free(entity_paths);

	return 0;
}

static int write_section_menu(const char* extract_folder, const int object_count, char** entity_paths, int16_t* offsets_x, int16_t* offsets_y) {
	FILE* temp_file = NULL;
	int temp_result_size;
	uint16_t temp_x = 0;
	uint16_t temp_y = 0;
	char* temp_data;
	unsigned char* temp_result;
	int temp_size = 0;
	uint8_t padding[] = { 0, 0 };
	char path_buffer[256];

	int section_ptr_offset = file_pos;
	
	fseek(test, 0x3c, 0);

	fwrite(&file_pos, 4, 1, test);

	fseek(test, file_pos, 0);

	int header_pos = write_sub_header(MENU, object_count);

	file_pos = ftell(test);
	for (int i = 0; i < object_count; i++) {
		fseek(test, header_pos, 0);
		fwrite(&file_pos, 4, 1, test);
		header_pos = ftell(test);
		fseek(test, file_pos, 0);

		strcpy(path_buffer, extract_folder);
		strcat(path_buffer, entity_paths[i]);

		
		temp_file = fopen(path_buffer, "rb");


		if (!temp_file) {
			printf("Couldn't open file %s %s\nAborting.\n", entity_paths[i], path_buffer);
			return 1;
		}

		fseek(temp_file, 0, SEEK_END);
		temp_size = ftell(temp_file);

		if (temp_size) {
			temp_result = malloc(temp_size * 2);
			temp_data = malloc(temp_size);
			rewind(temp_file);
			fread(temp_data, 1, temp_size, temp_file);
			temp_result_size = compress_bitmap(temp_result, temp_data, MENU);

			fseek(test, file_pos, 0);
			temp_x = *(uint32_t*)(temp_data + 18);
			temp_y = *(uint32_t*)(temp_data + 22);
			fwrite(&temp_x, 2, 1, test);
			fwrite(&temp_y, 2, 1, test);
			
			fwrite(&offsets_x[i], 2, 1, test);
			fwrite(&offsets_y[i], 2, 1, test);
			if (ftell(test) % 2 == 0) {
				fwrite(&padding, 1, 2, test);
			}
			else {
				fwrite(&padding, 1, 1, test);
			}
			fwrite(temp_result, temp_result_size, 1, test);
			file_pos = ftell(test);

			free(temp_data);
			free(temp_result);
		}
		else {
			fseek(test, file_pos, 0);
			fwrite(&padding, 1, 2, test);
			fwrite(&padding, 1, 2, test);
			
			fwrite(&offsets_x[i], 2, 1, test);
			fwrite(&offsets_y[i], 2, 1, test);
			if (ftell(test) % 2 == 0) {
				fwrite(&padding, 1, 2, test);
			}
			else {
				fwrite(&padding, 1, 1, test);
			}
			file_pos = ftell(test);
		}

		free(entity_paths[i]);

		file_pos = ftell(test);

	}

	fclose(temp_file);
	free(offsets_x);
	free(offsets_y);
	free(entity_paths);

	return 0;
}

static int write_section_texture(const char* extract_folder, const int object_count, char** entity_paths, int8_t* types) {
	FILE* temp_file = NULL;
	int temp_result_size;
	uint16_t temp_x = 0;
	uint16_t temp_y = 0;
	char* temp_data;
	unsigned char* temp_result;
	int temp_size = 0;
	uint8_t padding[] = { 0, 0 };
	uint8_t type_size = 1;
	char path_buffer[256];

	int section_ptr_offset = file_pos;
	fseek(test, 0x38, 0);

	fwrite(&file_pos, 4, 1, test);

	fseek(test, file_pos, 0);

	int header_pos = write_sub_header(TEXTURE, object_count);

	file_pos = ftell(test);
	for (int i = 0; i < object_count; i++) {
		fseek(test, header_pos, 0);
		fwrite(&file_pos, 4, 1, test);
		header_pos = ftell(test);
		fseek(test, file_pos, 0);

		strcpy(path_buffer, extract_folder);
		strcat(path_buffer, entity_paths[i]);


		temp_file = fopen(path_buffer, "rb");


		if (!temp_file) {
			printf("Couldn't open file %s %s\nAborting.\n", entity_paths[i], path_buffer);
			return 1;
		}

		fseek(temp_file, 0, SEEK_END);
		temp_size = ftell(temp_file);

		if (temp_size) {
			temp_result = malloc(temp_size * 2);
			temp_data = malloc(temp_size);
			rewind(temp_file);
			fread(temp_data, 1, temp_size, temp_file);
			temp_result_size = compress_bitmap(temp_result, temp_data, TEXTURE);

			fseek(test, file_pos, 0);
			temp_x = *(uint32_t*)(temp_data + 18);
			temp_y = *(uint32_t*)(temp_data + 22);
			fwrite(&temp_x, 2, 1, test);
			fwrite(&temp_y, 2, 1, test);
			fwrite(&type_size, 1, 1, test);
			fwrite(&types[i], 1, 1, test);
			if (!((ftell(test)-1) % 16)) {			//this is apparantly a thing
				fwrite(&padding, 1, 1, test);
			}
			fwrite(temp_result, temp_result_size, 1, test);
			file_pos = ftell(test);

			free(temp_data);
			free(temp_result);
		}
		else {
			fseek(test, file_pos, 0);
			fwrite(&padding, 1, 2, test);
			fwrite(&padding, 1, 2, test);
			fwrite(&type_size, 1, 1, test);
			fwrite(&types[i], 1, 1, test);
			if (ftell(test) % 2 == 0) {
				fwrite(&padding, 1, 2, test);
			}
			else {
				fwrite(&padding, 1, 1, test);
			}
			file_pos = ftell(test);
		}

		free(entity_paths[i]);

		file_pos = ftell(test);

	}
	
	fclose(temp_file);
	free(types);
	free(entity_paths);

	return 0;
}

static int write_section(const file_type type, const char* extract_folder, const int object_count, char** entity_paths, int* entity_subnum, int16_t* offsets_x, int16_t* offsets_y) {

	FILE* temp_file;
	int temp_size = 0;
	int temp_result_size;
	uint16_t temp_x = 0;
	uint16_t temp_y = 0;
	char* temp_data;
	unsigned char* temp_result;
	uint8_t padding[] = { 0, 0 };

	char path_buffer[256];

	int section_ptr_offset = file_pos;
	switch (type) {
	case SPRITE:
		fseek(test, 0x40, 0);
		fwrite(&file_pos, 4, 1, test);
		break;
	case CISPRITE:
		fseek(test, 0x44, 0);
		fwrite(&file_pos, 4, 1, test);
		break;
	case SHADOW:
		fseek(test, 0x48, 0);
		fwrite(&file_pos, 4, 1, test);
		break;
	}
	fseek(test, file_pos, 0);

	int header_pos = write_sub_header(type, object_count);
	file_pos = ftell(test);

	s3_dat_subsubheader subsub_header;
	subsub_header.magic = 0x1402;
	subsub_header.unk0 = 8;
	subsub_header.unk1 = 0;

	int* offset_list;
	int sub_pos = 0;
	int idx = 0;
	int sub_offset = 0;
	int temp_offset = 0;

	for (int i = 0; i < object_count; i++) {
		fseek(test, header_pos, 0);
		fwrite(&file_pos, 4, 1, test);
		header_pos = ftell(test);
		fseek(test, file_pos, 0);
		subsub_header.offset_count = entity_subnum[i];
		sub_offset = ftell(test);
		fwrite(&subsub_header, sizeof(subsub_header), 1, test);
		sub_pos = ftell(test);

		offset_list = calloc(entity_subnum[i], sizeof(int));
		fwrite(offset_list, 4, entity_subnum[i], test);
		file_pos = ftell(test);
		free(offset_list);

		for (int j = 0; j < entity_subnum[i]; j++) {
			fseek(test, sub_pos, 0);
			temp_offset = file_pos - sub_offset;
			fwrite(&temp_offset, 4, 1, test);
			sub_pos = ftell(test);

			strcpy(path_buffer, extract_folder);
			strcat(path_buffer, entity_paths[idx]);
			temp_file = fopen(path_buffer, "rb");

			if (!temp_file) {
				printf("Couldn't open file %s\nAborting.\n", entity_paths[idx]);
				return 1;
			}

			fseek(temp_file, 0, SEEK_END);
			temp_size = ftell(temp_file);

			if (temp_size) {
				temp_result = malloc(temp_size * 2);
				temp_data = malloc(temp_size);
				rewind(temp_file);
				fread(temp_data, 1, temp_size, temp_file);
				temp_result_size = compress_bitmap(temp_result, temp_data, type);

				fseek(test, file_pos, 0);
				int shit = 0xc;
				fwrite(&shit, 4, 1, test);
				temp_x = *(uint32_t*)(temp_data + 18);
				temp_y = *(uint32_t*)(temp_data + 22);
				fwrite(&temp_x, 2, 1, test);
				fwrite(&temp_y, 2, 1, test);
				fwrite(&offsets_x[idx], 2, 1, test);
				fwrite(&offsets_y[idx], 2, 1, test);
				if (ftell(test) % 2 == 0) {
					fwrite(&padding, 1, 2, test);
				}
				else {
					fwrite(&padding, 1, 1, test);
				}
				fwrite(temp_result, temp_result_size, 1, test);
				file_pos = ftell(test);

				free(temp_data);
				free(temp_result);
			}
			else {
				fseek(test, file_pos, 0);
				int shit = 0xc;
				fwrite(&shit, 4, 1, test);
				fwrite(&padding, 1, 2, test);
				fwrite(&padding, 1, 2, test);
				fwrite(&offsets_x[idx], 2, 1, test);
				fwrite(&offsets_y[idx], 2, 1, test);
				if (ftell(test) % 2 == 0) {
					fwrite(&padding, 1, 2, test);
				}
				else {
					fwrite(&padding, 1, 1, test);
				}
				file_pos = ftell(test);
			}

			fclose(temp_file);
			free(entity_paths[idx]);
			idx++;
		}
		file_pos = ftell(test);
	}
	free(offsets_x);
	free(offsets_y);
	free(entity_paths);
	free(entity_subnum);

	return 0;
	
}

static int write_sub_header(const file_type type, const int entity_count) {
	s3_dat_subheader sub_header;
	text_subheader text_header;
	palette_subheader palette_header;
	int header_pos;

	switch (type) {
		case TEXT:
			text_header.magic = 0x1904;
			text_header.header_size = 12 + entity_count * 4;
			text_header.string_count = entity_count;
			text_header.language_count = 8;
			fwrite(&text_header, sizeof(text_header), 1, test);
			break;
		case TEXTURE:
			sub_header.magic = 0x2412;
			sub_header.header_size = 8 + entity_count * 4;
			sub_header.offset_count = entity_count;
			fwrite(&sub_header, sizeof(sub_header), 1, test);
			break;
		case MENU:
			sub_header.magic = 0x11306;
			sub_header.header_size = 8 + entity_count * 4;
			sub_header.offset_count = entity_count;
			fwrite(&sub_header, sizeof(sub_header), 1, test);
			break;
		case SPRITE:
			sub_header.magic = 0x106;
			sub_header.header_size = 8 + entity_count * 4;
			sub_header.offset_count = entity_count;
			fwrite(&sub_header, sizeof(sub_header), 1, test);
			break;
		case SHADOW:
			sub_header.magic = 0x5982;
			sub_header.header_size = 8 + entity_count * 4;
			sub_header.offset_count = entity_count;
			fwrite(&sub_header, sizeof(sub_header), 1, test);
			break;
		case CISPRITE:
			sub_header.magic = 0x3112;
			sub_header.header_size = 8 + entity_count * 4;
			sub_header.offset_count = entity_count;
			fwrite(&sub_header, sizeof(sub_header), 1, test);
			break;
		case ANIMATION:
			sub_header.magic = 0x21702;
			sub_header.header_size = 8 + entity_count * 4;
			sub_header.offset_count = entity_count;
			fwrite(&sub_header, sizeof(sub_header), 1, test);
			break;
		case PALETTE:
			palette_header.magic = 0x2607;
			palette_header.header_size = 12 + entity_count * 4;
			palette_header.offset_count = entity_count;
			palette_header.palette_size = 256;
			fwrite(&palette_header, sizeof(palette_header), 1, test);
			break;
		default:
			break;
	}
	header_pos = ftell(test);

	int* offset_list;

	offset_list = calloc(entity_count, sizeof(int));
	fwrite(offset_list, 4, entity_count, test);
	
	
	free(offset_list);
	return header_pos;
}