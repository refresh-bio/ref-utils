all: mfasta-tool multi-fasta-split

# *** REFRESH makefile utils
include refresh.mk

$(call INIT_GLOBALS)
$(call CHECK_OS_ARCH)

# *** Project directories
3RD_PARTY_DIR := ./3rd_party
MD5_DIR := $(3RD_PARTY_DIR)/md5
OUT_BIN_DIR := ./bin

MFASTA_TOOL_SRC_DIR := ./mfasta-tool
MULTI_FASTA_SPLIT_SRC_DIR := ./multi-fasta-split

# *** Project configuration
$(call CHECK_NASM)
$(call ADD_REFRESH_LIB, $(3RD_PARTY_DIR))
$(call ADD_ZLIB_NG, $(3RD_PARTY_DIR)/zlib-ng)
$(call ADD_ISAL, $(3RD_PARTY_DIR)/isa-l)
$(call ADD_LIBDEFLATE, $(3RD_PARTY_DIR)/libdeflate)
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

# *** Source files
MULTI_FASTA_SPLIT_CPP_OBJS := $(MULTI_FASTA_SPLIT_SRC_DIR)/multi-fasta-split.cpp.o

MFASTA_TOOL_C_OBJS := $(MD5_DIR)/md5.c.o
MFASTA_TOOL_CPP_OBJS := $(MFASTA_TOOL_SRC_DIR)/mfasta-tool.cpp.o

# *** Source files dependencies
$(MULTI_FASTA_SPLIT_SRC_DIR)/multi-fasta-split.cpp.o: $(MULTI_FASTA_SPLIT_SRC_DIR)/multi-fasta-split.cpp

$(MD5_DIR)/md5.c.o: $(MD5_DIR)/md5.c
$(MFASTA_TOOL_SRC_DIR)/mfasta-tool.cpp.o: $(MFASTA_TOOL_SRC_DIR)/mfasta-tool.cpp $(MFASTA_TOOL_SRC_DIR)/params.h $(MFASTA_TOOL_SRC_DIR)/data_source.h $(MFASTA_TOOL_SRC_DIR)/data_storer.h $(MFASTA_TOOL_SRC_DIR)/data_partitioner.h $(MFASTA_TOOL_SRC_DIR)/md5_filter.h $(MFASTA_TOOL_SRC_DIR)/part_packer.h


# *** Source files build rules
$(MULTI_FASTA_SPLIT_SRC_DIR)/%.cpp.o: $(MULTI_FASTA_SPLIT_SRC_DIR)/%.cpp
	$(CXX) $(CPP_FLAGS) $(OPTIMIZATION_FLAGS) $(ARCH_FLAGS) $(INCLUDE_DIRS)  -c $< -o $@

$(MD5_DIR)/%.c.o: $(MD5_DIR)/%.c
	$(CC) $(C_FLAGS) $(OPTIMIZATION_FLAGS) $(ARCH_FLAGS) $(INCLUDE_DIRS) -c $< -o $@

$(MFASTA_TOOL_SRC_DIR)/%.cpp.o: $(MFASTA_TOOL_SRC_DIR)/%.cpp
	$(CXX) $(CPP_FLAGS) $(OPTIMIZATION_FLAGS) $(ARCH_FLAGS) $(INCLUDE_DIRS)  -c $< -o $@


# *** Targets
multi-fasta-split: $(OUT_BIN_DIR)/multi-fasta-split

$(OUT_BIN_DIR)/multi-fasta-split: $(GZ_TARGET) mimalloc_obj libdeflate \
	$(MULTI_FASTA_SPLIT_CPP_OBJS) 
	-mkdir -p $(OUT_BIN_DIR)	
	$(CXX) -o $@  \
	$(MIMALLOC_OBJ) \
	$(MULTI_FASTA_SPLIT_CPP_OBJS) \
	$(LIBRARY_FILES) \
	$(LINKER_FLAGS) \
	$(LINKER_DIRS)

mfasta-tool:  $(OUT_BIN_DIR)/mfasta-tool

$(OUT_BIN_DIR)/mfasta-tool: $(GZ_TARGET) mimalloc_obj libdeflate \
	$(MFASTA_TOOL_C_OBJS) \
	$(MFASTA_TOOL_CPP_OBJS) 
	-mkdir -p $(OUT_BIN_DIR)	
	$(CXX) -o $@  \
	$(MIMALLOC_OBJ) \
	$(MFASTA_TOOL_C_OBJS) \
	$(MFASTA_TOOL_CPP_OBJS) \
	$(LIBRARY_FILES) \
	$(LINKER_FLAGS) \
	$(LINKER_DIRS)


# *** Cleaning
.PHONY: clean init
clean: clean-zlib-ng clean-isa-l clean-libdeflate clean-mimalloc_obj
	-rm $(MULTI_FASTA_SPLIT_CPP_OBJS)
	-rm $(MFASTA_TOOL_C_OBJS)
	-rm $(MFASTA_TOOL_CPP_OBJS)
	-rm -r $(OUT_BIN_DIR)/

init:
	$(call INIT_SUBMODULES)

