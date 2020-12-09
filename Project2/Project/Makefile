CC = gcc
CFLAGS = -W -Wall
LIBS = -lpthread
TARGET = final
OBJECTS = LED.o LCD.o final.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm *.o final