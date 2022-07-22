include ../../../tools/build_scripts/internal_make_funcs.mk

# if were running on Windows build for Windows
ifdef WINDOWS
WINDOWS_BUILD=1
endif

# To compile as a shared library:
ifdef SHARED
# all code is position-indep with mingw, avoid warning about useless flag
ifndef WINDOWS_BUILD
LOCAL_CFLAGS += -fPIC -fpic -I./
endif
endif

DLEXT=so
# OSX shared library extension:
# DLEXT=dylib

# Windows shared library extension:
ifdef WINDOWS_BUILD
DLEXT=dll
endif

OBJS=	wslay_event.o	wslay_frame.o	wslay_net.o		\
		wslay_queue.o	wslay_stack.o

.SILENT:

.PHONY: all static shared clean

ifndef SHARED
all: static
else
all: shared static
endif

static: libwslay.a

shared: libwslay.$(DLEXT)

libwslay.a: $(OBJS)
	$(call AR_Log)
	$(AR) -rc $@ $(OBJS)
	$(AR) -s $@

libwslay.$(DLEXT): $(OBJS)
	echo "  LD    $@"
	$(CC) -shared -Wl,-soname,$@ $(LOCAL_LDFLAGS) $(LDFLAGS) -o $@ $(OBJS)

.c.o:
	# echo "[CC]  $@  $<"
	$(call CC_Log)
	@ \
	$(CC) $(LOCAL_CFLAGS) $(CFLAGS) -c $<

clean:
ifndef WINDOWS
	rm -f *.o libwslay*
else
	del /Q /F *.o libwslay*
endif
