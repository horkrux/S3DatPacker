#include "bmp.h"

#pragma pack(push)
#pragma pack(1)

typedef struct
{
	uint16_t bfType;
	uint32_t bfSize;
	uint16_t bfReserved1;
	uint16_t bfReserved2;
	uint32_t bfOffBits;
} BITMAPFILEHEADER;


typedef struct
{
	uint32_t biSize;
	uint32_t biWidth;
	uint32_t biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	uint32_t biXPelsPerMeter;
	uint32_t biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;
} BITMAPINFOHEADER;

typedef struct {
	uint32_t biRedMask;
	uint32_t biGreenMask;
	uint32_t biBlueMask;
} BITMAPINFOHEADERMASK;
#pragma pack(pop)

BITMAPFILEHEADER bmf;
BITMAPINFOHEADER bmi;
BITMAPINFOHEADERMASK bme;

bool write_bitmap(char *file_name, uint16_t width, uint16_t height, uint16_t* color_buffer, char* name, const pixel_format format, const int rgb24) {
	
	FILE* bitmap = fopen(file_name, "wb");
	int bpp = 2; //bytes per pixel
	if (rgb24) bpp = 3;

	if (!bitmap)
	{
		printf("Error opening %s: %s\n", file_name, strerror(errno));
		return false;
	}

	int num_pads = 0;
	if ((width * bpp) % 4) {
		num_pads = 4 - (width * bpp) % 4;
	}
	bmf.bfType = 0x4d42;
	
	
	bmi.biSizeImage = width*height * bpp + height*num_pads;
	if (rgb24) {
		bmf.bfSize = bmi.biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	}
	else {
		bmf.bfSize = bmi.biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(BITMAPINFOHEADERMASK);
	}
	
	bmf.bfReserved1 = 0;
	bmf.bfReserved1 = 0;
	if (!rgb24) {
		bmf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(BITMAPINFOHEADERMASK);
	}
	else {
		bmf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	}

	bmi.biSize = sizeof(BITMAPINFOHEADER);
	bmi.biWidth = width;
	bmi.biHeight = height;
	bmi.biPlanes = 1;
	if (rgb24) {
		bmi.biBitCount = 24;
	}
	else {
		bmi.biBitCount = 16;
	}
	
	bmi.biCompression = 3; 
	bmi.biXPelsPerMeter = 0;  
	bmi.biYPelsPerMeter = 0;  
	bmi.biClrUsed = 0;  
	bmi.biClrImportant = 0;


	//write headers
	fwrite(&bmf, sizeof(BITMAPFILEHEADER), 1, bitmap);
	fwrite(&bmi, sizeof(BITMAPINFOHEADER), 1, bitmap);
	
	
	if (!rgb24) {
		switch (format) {
		case RGB555:
			bme.biRedMask = 0x7c00;
			bme.biGreenMask = 0x3e0;
			bme.biBlueMask = 0x1f;

			fwrite(&bme, sizeof(BITMAPINFOHEADERMASK), 1, bitmap);
			break;
		case RGB565:
			bme.biRedMask = 0xf800;
			bme.biGreenMask = 0x7e0;
			bme.biBlueMask = 0x1f;

			fwrite(&bme, sizeof(BITMAPINFOHEADERMASK), 1, bitmap);
			break;
		default:
			break;
		}
	}
	uint8_t padding = 0;
	for (int i = height*width - width; i >= 0; i -= width) {
		if (rgb24) {
			uint8_t color_bytes[3];
			for (int j = 0; j < width; j++) {
				if (color_buffer[i + j] == 0x7c1f || color_buffer[i + j] == 0xf81f) {
					color_bytes[0] = 255;
					color_bytes[1] = 0;
					color_bytes[2] = 255;
				}
				else {
					color_bytes[0] = color_buffer[i + j];
					color_bytes[1] = color_buffer[i + j];
					color_bytes[2] = color_buffer[i + j];
				}
				fwrite(&color_bytes, 1, 3, bitmap);
			}

		}
		else {
			for (int j = 0; j < width; j++) {
			}
			
			fwrite(color_buffer + i, 2, width, bitmap);
		}
		
		for (int j = 0; j < num_pads; j++) {

			fwrite(&padding, sizeof(padding), 1, bitmap);
		}
	}

	if (bitmap)
		fclose(bitmap);

	return true;
}

int compress_bitmap(unsigned char* result, char* data, const file_type type) {
	int size = strlen(data);
	int start = *(uint32_t*)(data + 10);
	int x = *(uint32_t*)(data + 18);
	int y = *(uint32_t*)(data + 22);
	short color_depth = *(uint16_t*)(data + 28);

	int num_pads = 0;
	if ((x * (color_depth/8)) % 4) {
		num_pads = 4 - (x * (color_depth/8)) % 4;
	}

	int skip = 0;
	int length = 0;
	int current_idx = 0;
	int prev_meta = 0;
	int current_meta = 0;
	int meta_count = 0;
	uint16_t prev_pixel_value = 0;
	uint16_t pixel_value = 0;

	for (int i = y - 1; i > -1; i--) {
		result[current_meta] = 0;
		result[current_meta + 1] = 0;
		current_idx += 2;
		
		if (type == CISPRITE) {
			prev_pixel_value = 0xff;
		}
		else {
			prev_pixel_value = 0;
		}
		meta_count = 1;
		for (int j = 0; j < x; j++) {
			if (type == CISPRITE) {
				pixel_value = *(uint16_t*)(data + start + j * 3 + x * i * 3 + (i * num_pads));
			}
			else {
				pixel_value = *(uint16_t*)(data + start + j * 2 + x * i * 2 + (i * num_pads));
			}
			if (!pixel_value && type != CISPRITE || pixel_value == 0xff && type == CISPRITE) {
				if (result[current_meta + 1] < 127 && (prev_pixel_value == 0 && type != CISPRITE || prev_pixel_value == 0xff && type == CISPRITE)) {
					result[current_meta + 1]++;
				}
				else {
					prev_meta = current_meta;
					current_meta = current_idx;
					result[current_meta] = 0;
					result[current_meta + 1] = 1;
					meta_count++;
					current_idx += 2;
				}
			}
			else {
				if (result[current_meta] < 255) {
					result[current_meta]++;
				}
				else {
					prev_meta = current_meta;
					current_meta = current_idx;
					result[current_meta] = 1;
					result[current_meta + 1] = 0;
					meta_count++;
					current_idx += 2;
				}
				switch (type) {
				case CISPRITE:
					result[current_idx] = (uint8_t)pixel_value;
					current_idx++;
					break;
				case SHADOW:
					break;
				default:
					*(uint16_t*)(result + current_idx) = pixel_value;
					current_idx += 2;
					break;
				}
			}
			prev_pixel_value = pixel_value;
		}

		//if sequence length is 0, remove last meta
		if (result[current_meta] == 0) {
			result[current_meta + 1] = 0;
			if (meta_count > 1) {
				current_meta = prev_meta;
				if (i == 0) {
					current_idx -= 2;
				}
				else {
					current_idx -= 2;
				}
			}
		}
		
		result[current_meta + 1] |= 0x80;
		current_meta = current_idx;
	}
	return current_idx;
}

int gather_palette_colors(int16_t* color_buffer, const char* path_buffer) {
	FILE* palette_file;
	int offset = 0;
	int row = 0;

	palette_file = fopen(path_buffer, "rb");
	
	if (!palette_file) {
		printf("Couldn't open file at %s\nAborting.\n", path_buffer);
		return 1;
	}

	fseek(palette_file, 10, 0);
	fread(&offset, 4, 1, palette_file);

	fseek(palette_file, offset, 0);

	for (int i = 7; i > -1; i--) {
		fseek(palette_file, 512 * i + offset, 0);
		fread(color_buffer+256*row, 2, 256, palette_file);
		row++;
	}

	fclose(palette_file);

	return 0;
}


