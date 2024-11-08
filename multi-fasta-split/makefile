all: multi-fasta-split

# *** REFRESH makefile utils
include ../refresh.mk

$(call INIT_GLOBALS)
$(call CHECK_OS_ARCH)

# *** Project directories
3RD_PARTY_DIR := ../3rd_party
SRC_DIR := ./src

# *** Project configuration
$(call CHECK_NASM)
$(call ADD_REFRESH_LIB, $(3RD_PARTY_DIR))
$(call ADD_ZLIB_NG, $(3RD_PARTY_DIR)/zlib-ng)
$(call ADD_ISAL, $(3RD_PARTY_DIR)/isa-l)
$(call ADD_LIBDEFLATE, $(3RD_PARTY_DIR)/libdeflate)
#$(call ADD_LIBZSTD, $(3RD_PARTY_DIR)/libzstd)
$(call ADD_MIMALLOC, $(3RD_PARTY_DIR)/mimalloc)
$(call CHOOSE_GZIP_DECOMPRESSION)
$(call SET_STATIC, $(STATIC_LINK))
$(call SET_C_CPP_STANDARDS, c11, c++20)
$(call SET_FLAGS, $(TYPE))

$(call SET_COMPILER_VERSION_ALLOWED, GCC, Linux_x86_64, 10, 20)
$(call SET_COMPILER_VERSION_ALLOWED, GCC, Linux_aarch64, 11, 20)
$(call SET_COMPILER_VERSION_ALLOWED, GCC, Darwin_x86_64, 11, 13)
$(call SET_COMPILER_VERSION_ALLOWED, GCC, Darwin_arm64, 11, 13)

$(call CHECK_COMPILER_VERSION)

#$(call ADD_RADULS_INPLACE, $(3RD_PARTY)/raduls-inplace)
#$(call ADD_IGRAPH, $(3RD_PARTY)/igraph)
#$(call ADD_SBWT, $(3RD_PARTY)/sbwt)

# *** Source files
CPP_OBJS := $(SRC_DIR)/multi-fasta-split.cpp.o

# *** Source files dependencies
$(SRC_DIR)/multi-fasta-split.cpp.o: $(SRC_DIR)/multi-fasta-split.cpp

# *** Source files build rules
$(SRC_DIR)/%.cpp.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CPP_FLAGS) $(OPTIMIZATION_FLAGS) $(ARCH_FLAGS) $(INCLUDE_DIRS)  -c $< -o $@


# *** Targets
multi-fasta-split: $(GZ_TARGET) mimalloc_obj libdeflate \
	$(C_OBJS) \
	$(CPP_OBJS) 
	$(CXX) -o $@ \
	$(MIMALLOC_OBJ) \
	$(C_OBJS) \
	$(CPP_OBJS) \
	$(LIBRARY_FILES) \
	$(LINKER_FLAGS) \
	$(LINKER_DIRS)


# *** Cleaning
.PHONY: clean init
clean: clean-zlib-ng clean-isa-l clean-libdeflate clean-mimalloc_obj
	-rm $(C_OBJS)
	-rm $(CPP_OBJS)
	-rm multi-fasta-split

init:
	$(call INIT_SUBMODULES)

