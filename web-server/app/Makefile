CFLAGS += -Wall
PKGS = glib-2.0 monkey
CFLAGS += $(shell pkg-config --cflags $(PKGS))
LDLIBS += $(shell pkg-config --libs $(PKGS)) -ldl
LDFLAGS += -Wl,--no-as-needed,-rpath,'$$ORIGIN/lib'

.PHONY: all clean first

all: first hello list quiz

hello: hello.o

list: list.o

quiz: quiz.o

first:
	@echo -e "\n\tBuilding the examples." \
	"\n\n"\
	"\tThe build expects the library to be installed,\n"\
	"\tto work as closely as possible like an external app.\n\n"


clean:
	rm -f *.o hello quiz list
