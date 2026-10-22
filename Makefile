PREFIX =

ZOPFLI_DIR = zopfli
ZLIB_DIR = zlib-turbo

#DEFINES=-DLOCAL1
CC = $(PREFIX)gcc
OPT_CFLAGS =
WARN = -W -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable
CFLAGS = -c -g -O2 $(WARN) $(OPT_CFLAGS) $(DEFINES)
INCLUDES = -I. -I$(ZLIB_DIR) -I$(ZOPFLI_DIR)/src/zopfli
#CFLAGS += $(INCLUDES)

LD = $(PREFIX)gcc
OPT_LDFLAGS =
LDFLAGS = $(OPT_LDFLAGS)

ZIPZOP_OBJS = header.o recompress.o zz_util.o zipzop.o
ZOPFLI_DIR = zopfli
ZOPFLISRC_DIR = zopfli/src/zopfli
#OBJS = $(ZOPFLI_OBJS) $(ZIPZOP_OBJS)
OBJS = $(ZIPZOP_OBJS)
LIBS = -lm -L$(ZLIB_DIR) -lz -L$(ZOPFLI_DIR) -lzopfli

E =
TARGET = zipzop-zt$(E)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
#	strip $(TARGET)

header.o: src/header.c
	$(CC) $< $(CFLAGS) $(INCLUDES)
recompress.o: src/recompress.c $(ZOPFLISRC_DIR)/deflate.h
	$(CC) $< $(CFLAGS) $(INCLUDES)
zz_util.o: src/zz_util.c
	$(CC) $< $(CFLAGS) $(INCLUDES)
zipzop.o: src/zipzop.c
	$(CC) $< $(CFLAGS) $(INCLUDES)


zopfli:
	git clone https://github.com/google/zopfli/

libdeflate:
	git clone https://github.com/ebiggers/libdeflate

clean:
	rm -rf zopfli *.o $(TARGET)

