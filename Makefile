CC     = gcc
CFLAGS = -std=c99
TARGET = lexico

all: $(TARGET)

$(TARGET): main.c lexico.c lexico.h
	$(CC) $(CFLAGS) -o $@ main.c lexico.c

clean:
	rm -f $(TARGET) *.o
