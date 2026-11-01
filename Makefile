VERSION = 0.2
TARGET = libdirectio.so.$(VERSION)

CFLAGS ?= -O2 -g -Wall -Wextra
LDLIBS = -ldl

all: $(TARGET)

$(TARGET): libdirectio.c
	$(CC) $(CFLAGS) -fPIC -shared -Wl,-soname,libdirectio.so.0 $(LDFLAGS) $< -o $@ $(LDLIBS)

test_directio: test_directio.c

check: $(TARGET) test_directio
	LD_PRELOAD=$(CURDIR)/$(TARGET) ./test_directio
	LD_PRELOAD=$(CURDIR)/$(TARGET) dd if=/dev/zero of=test_directio.dd bs=4096 count=256 status=none
	LD_PRELOAD=$(CURDIR)/$(TARGET) dd if=test_directio.dd of=/dev/null bs=4096 status=none
	rm -f test_directio.dd

clean:
	rm -f $(TARGET) test_directio test_directio.dat test_directio.fifo test_directio.dd

.PHONY: all check clean
