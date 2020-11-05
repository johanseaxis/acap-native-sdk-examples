PROG1	= video_scene_subscriber_client
OBJS1	= $(PROG1).cpp generated/scene.pb.cc
PROGS	= $(PROG1)

LDFLAGS += -L./lib -Wl,-rpath,'$$ORIGIN/lib'

PKGS = protobuf video-scene-subscriber
CFLAGS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --cflags $(PKGS)) -I./generated
LDLIBS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs $(PKGS))

CFLAGS += -Wall
CPPFLAGS += -Os -pipe -std=c++17

all: $(PROGS) copylibs

$(OBJS1): proto

$(PROG1): $(OBJS1)
	$(CXX) $^ $(CFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) -o $@

# Generate the protobuf C++ code
proto:
	mkdir -p generated
	protoc -I $(SDKTARGETSYSROOT)/usr/share/protobuf scene.proto --cpp_out=./generated

# Copy protobuf libraries and libatomic dependency from the SDK into lib folder
LIBPROTO_FILES = $(wildcard $(SDKTARGETSYSROOT)/usr/lib/libprotobuf.so*)
LIBATOMIC_FILES = $(wildcard $(SDKTARGETSYSROOT)/usr/lib/libatomic.so*)
.PHONY: copylibs
copylibs:
	mkdir -p lib
	cp -Rf $(LIBPROTO_FILES) lib
	cp -Rf $(LIBATOMIC_FILES) lib

clean:
	rm -rf $(PROGS) *.o *.eap generated/ lib/
