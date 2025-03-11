#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <linux/videodev2.h>
#include "camera_hal.h"
#include "camera_utils.h"

int open_camera(const char *device){
    int fd = open(device, O_RDWR);
    if (fd == -1){
        perror("Error finding device.");
        return -1;
    }
    return fd;
}

int check_camera_capabilities(int fd){
    struct v4l2_capability cap;
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1){
        perror("Error encountered while querying camera capabilities.");
        return -1;
    }

    printf("Driver: %s\n", cap.driver);
    printf("Card: %s\n", cap.card);
    printf("Bus info: %s\n", cap.bus_info);

    // V4L2_CAP_VIDEO_CAPTURE == video capture device?
    if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)){
        printf("Device does not support video capture.\n");
        return -1;
    }

    return 0;
}

CameraFormat set_camera_format(int fd, int width, int height){
    struct v4l2_format fmt;
    CameraFormat camera_format;

    camera_format.width = width;
    camera_format.height = height;

    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1){
        perror("Error encountered while setting format.");
        camera_format.width = 0;
        camera_format.height = 0;
    } else {
        printf("Format set: %dx%d\n", width, height);
    }

    return camera_format;
} 

struct buffer *buffers;
int num_buffers;

int request_and_map_buffers(int fd, int buff_count){
    struct v4l2_requestbuffers req;
    req.count = buff_count;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1){
        perror("Error encountered requesting buffers.");
        return -1;
    }

    buffers = calloc(req.count, sizeof(struct buffer));
    num_buffers = req.count;

    for(size_t i = 0; i < req.count; i++){
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1){
            perror("Error encountered whilst querying buffer");
            return -1;
        }

        buffers[i].length = buf.length;
        buffers[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        if (buffers[i].start == MAP_FAILED){
            perror("Error encountered whilst mapping buffer.");
            return -1;
        }
    }

    return 0;
}

int start_streaming(int fd){
    for(int i = 0; i < num_buffers; i++){
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (ioctl(fd, VIDIOC_QBUF, &buf) == -1){
            perror("Error encountered whilst queueing buffer.");
            return -1;
        }
    }

    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMON, &type) == -1){
        perror("Encountered error when starting stream.");
        return -1;
    }

    return 0;
}

void process_frame(int fd, CameraFormat fmt, bool save, const char *filename){
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    // dequeue frame
    if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
        perror("Error encountered whilst dequeueing buffer.");
        return;
    }

    uint8_t *bgr_data = malloc(fmt.width * fmt.height * 3); // bgr is 3 bytes per 1 pixel
    if(!bgr_data){
        perror("Memory allocation error on BGR buffer.");
        return;
    }

    yuyv_to_bgr((uint8_t *)buffers[buf.index].start, bgr_data, fmt.width, fmt.height);

    if(save && filename) {
        FILE *file = fopen(filename, "wb");
        if (!file){
            perror("Error opening file for saving frame");
            free(bgr_data);
            return;
        }

        size_t written = fwrite(bgr_data, 1, buf.bytesused * 3 / 2, file);
        if (written != buf.bytesused * 3 / 2){
            perror("Error writing frame data to file.");
        }

        fclose(file);
    }

    if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
        perror("Error encountered whilst requeueing buffer.");
    }

    free(bgr_data);
}

void stop_streaming(int fd){
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd, VIDIOC_STREAMOFF, &type);

    for(int i = 0; i < num_buffers; i++){
        munmap(buffers[i].start, buffers[i].length);
    }

    free(buffers);
    close(fd);
} 
