PROG1	= subscribe_to_object_detection
OBJS1	= $(PROG1).c generated/video-object-detection.pb-c.c
PROGS	= $(PROG1)

PKGS = gio-2.0 video-object-detection-subscriber gio-unix-2.0

LDLIBS += -lprotobuf-c
LDFLAGS += -L./lib -Wl,-rpath,'$$ORIGIN/lib'

CFLAGS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --cflags $(PKGS)) -I./generated -I./include
LDLIBS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs $(PKGS))

CFLAGS += -Wall -Wformat=2 -Wpointer-arith -Wbad-function-cast -Wstrict-prototypes -Wmissing-prototypes -Winline -Wdisabled-optimization -Wfloat-equal

all:	$(PROGS)

$(OBJS1): proto

$(PROG1): $(OBJS1)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBS) $(LDLIBS) -o $@

proto:
	mkdir -p generated
	protoc-c -I $(SDKTARGETSYSROOT)/usr/share/protobuf video-object-detection.proto --c_out=./generated

clean:
	rm -rf $(PROGS) *.o *.eap generated/
