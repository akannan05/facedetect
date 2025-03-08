#ifndef CAMERA_UTILS_H
#define CAMERA_UTILS_H

#include <stdint.h>
#include <stddef.h>

void yuyv_to_bgr(const uint8_t *yuyv, uint8_t *bgr, size_t w, size_t h);

void yuyv_to_bgr_pixel(uint8_t y1, uint8_t cb, uint8_t y2, uint8_t cr, uint8_t *bgr_1, uint8_t *bgr2);

#endif //CAMERA_UTILS_H
