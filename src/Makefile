CC = gcc
CCFLAGS = -Wall -Wextra
DEPS = camera_hal.h camera_utils.h
OBJS = main.o camera_hal.o camera_utils.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CCFLAGS)

main: $(OBJS)
	$(CC) $(CCFLAGS) -o test $(OBJS)
