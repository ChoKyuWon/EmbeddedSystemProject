CC = gcc
CFLAGS = -W -Wall
TARGET = final
OBJECTS = LED.o LCD.o final.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm *.o final