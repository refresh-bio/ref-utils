all: mfasta-tool multi-fasta-split

# *** REFRESH makefile utils
include refresh.mk

$(call INIT_SUBMODULES)
$(call INIT_GLOBALS)
$(call CHECK_OS_ARCH, $(PLATFORM))

# *** Project directories
3RD_PARTY_DIR := ./3rd_party
SRC_MD5_DIR := $(3RD_PARTY_DIR)/md5
OBJ_MD5_DIR := ./obj/md5
OUT_BIN_DIR := ./bin
OBJ_DIR := ./obj

SRC_MFASTA_TOOL_DIR := ./mfasta-tool
OBJ_MFASTA_TOOL_DIR := ./obj/mfasta-tool
SRC_MULTI_FASTA_SPLIT_DIR := ./multi-fasta-split
OBJ_MULTI_FASTA_SPLIT_DIR := ./obj/multi-fasta-split

# *** Project configuration
$(call CHECK_NASM)
$(call ADD_ZLIB_NG, $(3RD_PARTY_DIR)/zlib-ng)
$(call ADD_ISAL, $(3RD_PARTY_DIR)/isa-l)
$(call ADD_LIBDEFLATE, $(3RD_PARTY_DIR)/libdeflate)
$(call ADD_MIMALLOC, $(3RD_PARTY_DIR)/mimalloc)
$(call CHOOSE_GZIP_DECOMPRESSION)
$(call ADD_REFRESH_LIB, $(3RD_PARTY_DIR))
$(call SET_STATIC, $(STATIC_LINK))
$(call SET_C_CPP_STANDARDS, c11, c++20)
$(call SET_GIT_COMMIT)

$(call SET_FLAGS, $(TYPE))

$(call SET_COMPILER_VERSION_ALLOWED, GCC, Linux_x86_64, 10, 20)
$(call SET_COMPILER_VERSION_ALLOWED, GCC, Linux_aarch64, 11, 20)
$(call SET_COMPILER_VERSION_ALLOWED, GCC, Darwin_x86_64, 11, 13)
$(call SET_COMPILER_VERSION_ALLOWED, GCC, Darwin_arm64, 11, 13)

ifneq ($(MAKECMDGOALS),clean)
$(call CHECK_COMPILER_VERSION)
endif

# *** Source files
SRC_MD5 := $(wildcard $(SRC_MD5_DIR)/*.c)
OBJ_MD5 := $(patsubst $(SRC_MD5_DIR)/%.c, $(OBJ_MD5_DIR)/%.c.o, $(SRC_MD5))

SRC_MFASTA_TOOL := $(wildcard $(SRC_MFASTA_TOOL_DIR)/*.cpp)
OBJ_MFASTA_TOOL := $(patsubst $(SRC_MFASTA_TOOL_DIR)/%.cpp, $(OBJ_MFASTA_TOOL_DIR)/%.cpp.o, $(SRC_MFASTA_TOOL))

SRC_MULTI_FASTA_SPLIT := $(wildcard $(SRC_MULTI_FASTA_SPLIT_DIR)/*.cpp)
OBJ_MULTI_FASTA_SPLIT := $(patsubst $(SRC_MULTI_FASTA_SPLIT_DIR)/%.cpp, $(OBJ_MULTI_FASTA_SPLIT_DIR)/%.cpp.o, $(SRC_MULTI_FASTA_SPLIT))

# *** Build rules
$(OBJ_MD5_DIR)/%.c.o: $(SRC_MD5_DIR)/%.c
	@mkdir -p $(OBJ_MD5_DIR)
	$(CXX) $(CPP_FLAGS) $(OPTIMIZATION_FLAGS) $(ARCH_FLAGS) $(INCLUDE_DIRS) -MMD -MF $@.d -c $< -o $@
$(OBJ_MFASTA_TOOL_DIR)/%.cpp.o: $(SRC_MFASTA_TOOL_DIR)/%.cpp | $(GZ_TARGET)
	@mkdir -p $(OBJ_MFASTA_TOOL_DIR)
	$(CXX) $(CPP_FLAGS) $(OPTIMIZATION_FLAGS) $(ARCH_FLAGS) $(INCLUDE_DIRS) -MMD -MF $@.d -c $< -o $@
$(OBJ_MULTI_FASTA_SPLIT_DIR)/%.cpp.o: $(SRC_MULTI_FASTA_SPLIT_DIR)/%.cpp | $(GZ_TARGET)
	@mkdir -p $(OBJ_MULTI_FASTA_SPLIT_DIR)
	$(CXX) $(CPP_FLAGS) $(OPTIMIZATION_FLAGS) $(ARCH_FLAGS) $(INCLUDE_DIRS) -MMD -MF $@.d -c $< -o $@

# Dependency files
-include $(OBJ_MD5:.o=.o.d)
-include $(OBJ_MFASTA_TOOL:.o=.o.d)
-include $(OBJ_MULTI_FASTA_SPLIT:.o=.o.d)

# *** Targets
multi-fasta-split: $(OUT_BIN_DIR)/multi-fasta-split

$(OUT_BIN_DIR)/multi-fasta-split: $(GZ_TARGET) mimalloc_obj libdeflate \
	$(OBJ_MULTI_FASTA_SPLIT) 
	-mkdir -p $(OUT_BIN_DIR)	
	$(CXX) -o $@  \
	$(MIMALLOC_OBJ) \
	$(OBJ_MULTI_FASTA_SPLIT) \
	$(LIBRARY_FILES) \
	$(LINKER_FLAGS) \
	$(LINKER_DIRS)

mfasta-tool:  $(OUT_BIN_DIR)/mfasta-tool

$(OUT_BIN_DIR)/mfasta-tool: $(GZ_TARGET) mimalloc_obj libdeflate \
	$(OBJ_MD5) \
	$(OBJ_MFASTA_TOOL) 
	-mkdir -p $(OUT_BIN_DIR)	
	$(CXX) -o $@  \
	$(MIMALLOC_OBJ) \
	$(OBJ_MD5) \
	$(OBJ_MFASTA_TOOL) \
	$(LIBRARY_FILES) \
	$(LINKER_FLAGS) \
	$(LINKER_DIRS)


# *** Cleaning
.PHONY: clean init
clean: clean-zlib-ng clean-isa-l clean-libdeflate clean-mimalloc_obj
	-rm -r $(OBJ_DIR)
	-rm -r $(OUT_BIN_DIR)

init:
	$(call INIT_SUBMODULES)

