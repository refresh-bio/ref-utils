all: mfasta-tool multi-fasta-split

# *** REFRESH makefile utils
include refresh.mk

$(call INIT_SUBMODULES)
$(call INIT_GLOBALS)
$(call CHECK_OS_ARCH, $(PLATFORM))

# *** Project directories
$(call SET_SRC_OBJ_BIN,src,obj,bin)
3RD_PARTY_DIR := ./3rd_party

# *** Project configuration
$(call CHECK_NASM)
$(call PROPOSE_ZLIB_NG, $(3RD_PARTY_DIR)/zlib-ng)
$(call PROPOSE_ISAL, $(3RD_PARTY_DIR)/isa-l)
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

# *** Source files and rules
$(eval $(call PREPARE_DEFAULT_COMPILE_RULE,MFASTA_TOOL,mfasta-tool))
$(eval $(call PREPARE_DEFAULT_COMPILE_RULE,MULTI_FASTA_SPLIT,multi-fasta-split))

# *** Targets
multi-fasta-split: $(OUT_BIN_DIR)/multi-fasta-split

$(OUT_BIN_DIR)/multi-fasta-split: $(GZ_TARGET) mimalloc_obj libdeflate \
	$(OBJ_MULTI_FASTA_SPLIT) 
	-mkdir -p $(OUT_BIN_DIR)	
	$(CXX) -o $@  \
	$(MIMALLOC_OBJ) \
	$(OBJ_MULTI_FASTA_SPLIT) \
	$(LIBRARY_FILES) $(LINKER_FLAGS) $(LINKER_DIRS)

mfasta-tool:  $(OUT_BIN_DIR)/mfasta-tool

$(OUT_BIN_DIR)/mfasta-tool: $(GZ_TARGET) mimalloc_obj libdeflate \
	$(OBJ_MFASTA_TOOL) 
	-mkdir -p $(OUT_BIN_DIR)	
	$(CXX) -o $@  \
	$(MIMALLOC_OBJ) \
	$(OBJ_MFASTA_TOOL) \
	$(LIBRARY_FILES) $(LINKER_FLAGS) $(LINKER_DIRS)


# *** Cleaning
.PHONY: clean init
clean: clean-zlib-ng clean-isa-l clean-libdeflate clean-mimalloc_obj
	-rm -r $(OBJ_DIR)
	-rm -r $(OUT_BIN_DIR)

init:
	$(call INIT_SUBMODULES)

