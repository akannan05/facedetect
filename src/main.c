#include "camera_hal.h"
#include "camera_utils.h"

int main() {
    
    // fundamental device checks
    const char *device = "/dev/video0";
    int fd = open_camera(device);
    if (fd == -1) return 1;
    if (check_camera_capabilities(fd) == -1) return 1;

    // formatting and setting up buffers to map
    CameraFormat fmt = set_camera_format(fd, 240, 240);

    if (request_and_map_buffers(fd, 4) == -1) return 1;
    
    // actual image capturing & processing
    if (start_streaming(fd) == -1) return 1;

    process_frame(fd, fmt, 1, "output_frame.bmp");    

    stop_streaming(fd);
    return 0;
} 
