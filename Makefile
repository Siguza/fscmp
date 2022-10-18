TARGET = fscmp
FW     = AppleFSCompression

.PHONY: all clean

all: $(TARGET)

$(TARGET): src/*.c $(FW).framework/Headers/*.h Makefile
	$(CC) -o $@ -Wall -O3 $(CFLAGS) src/*.c -F. -framework $(FW) -framework CoreFoundation $(LDFLAGS)

clean:
	rm -f $(TARGET)
