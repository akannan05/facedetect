#include "camera_utils.h"
#include <stdio.h>

// footroom and headroom offsets 
#define Y_OFFSET 16
#define C_OFFSET 128

#define PIX 255
#define YUYV_2BGR_1 219
#define YUYV_2BGR_2 224

#define COEFF_1 1.402
#define COEFF_2 1.772

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
    }
}

void yuyv_to_bgr_pixel(uint8_t y1, uint8_t cb, uint8_t y2, uint8_t cr, uint8_t *bgr_1, uint8_t *bgr2){
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


