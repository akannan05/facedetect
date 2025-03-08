#include <stdio.h>
#include <stdint.h>

int main(){
    FILE *f = fopen("frame.yuyv", "rb");
    if(!f){
        perror("encountered error opening frame");
        return -1;
    }

    uint8_t buffer[4];
    while(fread(buffer, 1, 4, f) == 4) {
        printf("Y1: %d, Cb: %d, Y2: %d, cr: %d\n", buffer[0], buffer[1], buffer[2], buffer[3]);
    }

    fclose(f);
    return 0;
}
