#include "camera_utils.h"
#include <stdio.h>
#include <png.h>
#include <setjmp.h>

// footroom and headroom offsets 
#define Y_OFFSET 16
#define C_OFFSET 128

#define PIX 255
#define YUYV_2BGR_1 219
#define YUYV_2BGR_2 224

#define COEFF_1 1.402
#define COEFF_2 1.772

jmp_buf png_jmpbuf;


void yuyv_to_bgr(const uint8_t *yuyv, uint8_t *bgr, size_t w, size_t h){
    size_t n = w * h;

    for (size_t i = 0; i < n/2; i++){
        uint8_t y1 = yuyv[i*4 + 0];
        uint8_t cb = yuyv[i*4 + 1];
        uint8_t y2 = yuyv[i*4 + 2];
        uint8_t cr = yuyv[i*4 + 3];

        uint8_t *bgr1 = bgr + i*6;
        uint8_t *bgr2 = bgr + (i*6 + 3);

        //call helper per pixel
        yuyv_to_bgr_pixel(y1, cb, y2, cr, bgr1, bgr2);
    }
}

void yuyv_to_bgr_pixel(uint8_t y1, uint8_t cb, uint8_t y2, uint8_t cr, uint8_t *bgr1, uint8_t *bgr2){
    if(!bgr1 || !bgr2){
        fprintf(stderr, "ERROR: invalid BGR buffer pointers\n");
        return;
    }

    int r, g, b;

    r = (int) (PIX/YUYV_2BGR_1) * (y1 - Y_OFFSET) + (PIX/YUYV_2BGR_2) * COEFF_1 * (cr - C_OFFSET);
    g = (int) (PIX/YUYV_2BGR_1) * (y1 - Y_OFFSET) - (PIX/YUYV_2BGR_2) * COEFF_2 * (0.114/0.587) * (cb - C_OFFSET) - (PIX/YUYV_2BGR_2) * COEFF_1 * (0.299/0.587) * (cr - C_OFFSET);
    b = (int) (PIX/YUYV_2BGR_1) * (y1 - Y_OFFSET) + (PIX/YUYV_2BGR_2) * COEFF_2 * (cb - C_OFFSET);

    bgr1[0] = (b < 0) ? 0 : (b > 255) ? 255 : b;
    bgr1[1] = (g < 0) ? 0 : (g > 255) ? 255 : g;
    bgr1[2] = (r < 0) ? 0 : (r > 255) ? 255 : r;

    r = (int) (PIX/YUYV_2BGR_1) * (y2 - Y_OFFSET) + (PIX/YUYV_2BGR_2) * COEFF_1 * (cr - C_OFFSET);
    g = (int) (PIX/YUYV_2BGR_1) * (y2 - Y_OFFSET) - (PIX/YUYV_2BGR_2) * COEFF_2 * (0.114/0.587) * (cb - C_OFFSET) - (PIX/YUYV_2BGR_2) * COEFF_1 * (0.299/0.587) * (cr - C_OFFSET);
    b = (int) (PIX/YUYV_2BGR_1) * (y2 - Y_OFFSET) + (PIX/YUYV_2BGR_2) * COEFF_2 * (cb - C_OFFSET);

    bgr2[0] = (b < 0) ? 0 : (b > 255) ? 255 : b;
    bgr2[1] = (g < 0) ? 0 : (g > 255) ? 255 : g;
    bgr2[2] = (r < 0) ? 0 : (r > 255) ? 255 : r;
}

void save_bgr_to_png(uint8_t *bgr_data, size_t w, size_t h, const char *filename){
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Unable to open file for writing");
        return;
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png){
        perror("Unable to create PNG write struct");
        fclose(fp);
        return;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        perror("Unable to create png info struct");
        png_destroy_write_struct(&png, (png_infopp)NULL);
        fclose(fp);
        return;
    }

    if (setjmp(png_jmpbuf)) {
        perror("Error during png creation");
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        return;
    }

    png_init_io(png, fp);

    png_set_IHDR(
        png, info, (png_uint_32)w, (png_uint_32)h, 8, PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT        
    );

    png_write_info(png, info);

    for (size_t y = 0; y < h; y++){
        uint8_t *row = bgr_data + (y * w * 3);
        png_write_row(png, row);
    }

    png_write_end(png, info);

    png_destroy_write_struct(&png, &info);
    fclose(fp);
}
