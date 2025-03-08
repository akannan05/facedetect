#ifndef CAMERA_HAL_H
#define CAMERA_HAL_H

#include <linux/videodev2.h>

int open_camera(const char *device);
int check_camera_capabilities(int fd);
int set_camera_format(int fd, int width, int height);
int request_and_map_buffers(int fd, int buff_count);
int start_streaming(int fd);
int capture_frame(int fd, const char *filename);
void stop_streaming(int fd);

#endif //CAMERA_HAL_H