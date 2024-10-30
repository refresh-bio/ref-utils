ISAL_DIR = ../3rd_party/isa-l
MD5_DIR = ../3rd_party/md5
ZLIB_DIR = ../3rd_party/zlib-ng
LIBDEFLATE_DIR = ../3rd_party/libdeflate

INC_DIRS =. ../3rd_party/zlib-ng/ ../3rd_party/isa-l/include ../3rd_party/libdeflate ../libs ..
INCLUDE_DIR=$(foreach d, $(INC_DIRS), -I$d)

MFS_LIBS_DIR = ../3rd_party

ifdef MSVC     # Avoid the MingW/Cygwin sections
    uname_S := Windows
    uname_M := "x86_64"
else                          # If uname not available => 'not'
    uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
    uname_M := $(shell sh -c 'uname -m 2>/dev/null || echo not')
endif

NASM_V := $(shell nasm --version 2>/dev/null)

ifeq ($(PLATFORM), arm8)
$(info *** ARMv8 with NEON extensions ***)
    ARCH_FLAGS := -march=armv8-a  -DARCH_ARM
else ifeq ($(PLATFORM), m1)
$(info *** Apple M1(or never) with NEON extensions ***)
    ARCH_FLAGS := -march=armv8.4-a  -DARCH_ARM
else ifeq ($(PLATFORM), sse2)
$(info *** x86-64 with SSE2 extensions ***)
    ARCH_FLAGS := -msse2 -m64 -DARCH_X64 
else ifeq ($(PLATFORM), avx)
$(info *** x86-64 with AVX extensions ***)
    ARCH_FLAGS := -mavx -m64  -DARCH_X64
else ifeq ($(PLATFORM), avx2)
$(info *** x86-64 with AVX2 extensions ***)
    ARCH_FLAGS := -mavx2 -m64  -DARCH_X64
else
$(info *** Unspecified platform - use native compilation)
    ifeq ($(uname_M),x86_64)
        ARCH_FLAGS := -march=native -DARCH_X64
    else
        ARCH_FLAGS := -march=native -DARCH_ARM
    endif	
endif

CFLAGS	= -fPIC -static -pthread -Wall -O3 -std=c++20 $(ARCH_FLAGS) $(INCLUDE_DIR) -fpermissive
CLINK	= -lm

ifeq ($(uname_S),Linux)
    CLINK+=-fabi-version=6
    CLINK+=-static -Wl,--whole-archive -lpthread -Wl,--no-whole-archive
endif

ifeq ($(uname_S),Darwin)
    CLINK += -lpthread -static-libgcc
endif

ifeq ($(uname_M),x86_64)
    ifdef NASM_V
        GZ_LIB = isa-l.a
        gz_target = isa-l
        CFLAGS+=-DREFRESH_USE_IGZIP
    else
        GZ_LIB = libz.a
        gz_target = ng_zlib
        CFLAGS+=-DREFRESH_USE_ZLIB
    endif
else
    GZ_LIB = libz.a
    gz_target = ng_zlib
    CFLAGS+=-DREFRESH_USE_ZLIB
endif

all: mfasta-tool

ng_zlib:
	cd $(ZLIB_DIR) && ./configure --zlib-compat && $(MAKE) libz.a
	cp $(ZLIB_DIR)/libz.* $(MFS_LIBS_DIR)

isa-l:
	cd $(ISAL_DIR) && $(MAKE) -f Makefile.unx
	cp $(ISAL_DIR)/bin/isa-l.a $(MFS_LIBS_DIR)
	cp $(ISAL_DIR)/bin/libisal.* $(MFS_LIBS_DIR)

libdeflate:
	cd $(LIBDEFLATE_DIR) && cmake -B build && cmake --build build
	cp $(LIBDEFLATE_DIR)/build/libdeflate.* $(MFS_LIBS_DIR)

%.o: %.cpp $(gz_target)
	$(CXX) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CXX) $(CFLAGS) -c $< -o $@

mfasta-tool: $(gz_target) \
	libdeflate \
	mfasta-tool.o \
	$(MD5_DIR)/md5.o
	$(CXX) -o $@	\
	mfasta-tool.o \
	$(MFS_LIBS_DIR)/libdeflate.a \
	$(MFS_LIBS_DIR)/$(GZ_LIB) \
	$(MD5_DIR)/md5.o \
	$(CLINK)

clean:
	-rm *.o
	-rm mfasta-tool
	-rm $(MD5_DIR)/*.o
	cd $(ZLIB_DIR) && $(MAKE) -f Makefile.in clean
	cd $(ISAL_DIR) && $(MAKE) -f Makefile.unx clean
	-cd $(LIBDEFLATE_DIR) && rm -r build
	-rm $(MFS_LIBS_DIR)/libz.*
	-rm $(MFS_LIBS_DIR)/isa-l.*
	-rm $(MFS_LIBS_DIR)/libisal.*
	-rm $(MFS_LIBS_DIR)/libdeflate.*
