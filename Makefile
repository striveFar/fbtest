# Makefile for natively compiling the fbtest program 
#

CC = arm-linux-gnueabi-gcc
CFLAGS = -O2 -Wall

OBJS = main.o

TARGET = fbtest

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)

