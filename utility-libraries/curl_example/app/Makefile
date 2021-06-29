PROG1	= curl_example
OBJS1	= $(PROG1).c
PROGS	= $(PROG1)

LDLIBS += -Llib/build/lib/
CFLAGS += -Ilib/build/include/curl/
CFLAGS += -Wall

SHLIB_DIR = ./lib
LDFLAGS = -L$(SHLIB_DIR) -Wl,--no-as-needed,-rpath,'$$ORIGIN/lib'
SHLIBS += -lm -lcurl

LIBCORE_FILES = $(wildcard lib/build/lib/libcurl.so*)

all:	$(PROGS)

$(PROG1): $(OBJS1)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SHLIBS) $(LDLIBS)  $^ $(LIBS) $(LDLIBS) -o $@

libscopy:
	mkdir -p $(SHLIB_DIR)
	cp $(LIBCORE_FILES) $(SHLIB_DIR)

clean:
	rm -f $(PROGS) *.o *.eap
