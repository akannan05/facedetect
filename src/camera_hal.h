<<<<<<< HEAD
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
=======
#ifndef CAMERA_HAL_H
#define CAMERA_HAL_H
>>>>>>> 43fb2454fde5362930489c8c046e42b479a2db8c

#include <linux/videodev2.h>

struct buffer{
    void *start;
    size_t length;
};

typedef struct {
    int width;
    int height;
} CameraFormat;

int open_camera(const char *device);
int check_camera_capabilities(int fd);
CameraFormat set_camera_format(int fd, int width, int height);
int request_and_map_buffers(int fd, int buff_count);
int start_streaming(int fd);
void process_frame(int fd, CameraFormat fmt, bool save, const char *filename);
void stop_streaming(int fd);

#endif //CAMERA_HAL_H