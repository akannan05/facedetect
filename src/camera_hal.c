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

struct buffer{
    void *start;
    size_t length;
};

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

int set_camera_format(int fd, int width, int height){
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1){
        perror("Error encountered while setting format.");
        return -1;
    }

    printf("Format set: %dx%d\n", fmt.fmt.pix.width, fmt.fmt.pix.height);
    return 0;
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

    for(int i = 0; i < req.count; i++){
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

int capture_frame(int fd, const char *filename){
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
        perror("Error encountered dequeuing buffer.");
        return -1;
    }

    FILE *file = fopen(filename, "wb");
    if (file) {
        fwrite(buffers[buf.index].start, buf.bytesused, 1, file);
        fclose(file);
        printf("Saved frame to %s\n", filename);
    }

    if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
        perror("Error encountered re-queuing buffer.");
        return -1;
    }

    return 0;
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

int main() {
    const char *device = "/dev/video0";
    int fd = open_camera(device);
    if (fd == -1) return 1;

    if (check_camera_capabilities(fd) == -1) return 1;
    if (set_camera_format(fd, 240, 240) == -1) return 1;
    if (request_and_map_buffers(fd, 4) == -1) return 1;
    if (start_streaming(fd) == -1) return 1;

    capture_frame(fd, "frame.yuyv");

    stop_streaming(fd);
    return 0;
}
