VERSION=0.1

TARGET=libdirectio.so.$(VERSION)
OBJS=libdirectio.o
CC=gcc
CFLAGS= -fPIC -Wall -O2 -g
SHAREDFLAGS= -nostartfiles -shared -W1,-soname,libdirectio.so.0

all: $(TARGET)

%.o: %.c
	$(CC) -I. $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(SHAREDFLAGS) $(OBJS) -o $(TARGET) -lc -ldl

clean:
	rm -f $(OBJS) $(TARGET)
