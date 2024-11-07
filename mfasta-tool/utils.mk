### Macros for 3rd-party libraries registration
# Add zlib-ng
define ADD_ZLIB_NG
	$(info *** Adding zlib-ng ***)
	$(eval INCLUDE_DIRS += -I$(1))
	$(eval ZLIB_DIR := $(1))
	$(eval ZLIB_A_DIR := $(1))
	$(eval ZLIB_A := $(1)/libz.a)
endef

# Add isa-l
define ADD_ISAL
	$(info *** Adding isal ***)
	$(eval INCLUDE_DIRS += -I$(1)/include)
	$(eval ISAL_DIR := $(1))
	$(eval ISAL_A_DIR := $(1)/bin)
	$(eval ISAL_A := $(1)/bin/isa-l.a)
endef

# Add libdeflate
define ADD_LIBDEFLATE
	$(info *** Adding libdeflate ***)
	$(eval INCLUDE_DIRS += -I$(1))
	$(eval LIBDEFLATE_DIR := $(1))
	$(eval LIBDEFLATE_A_DIR := $(1))
	$(eval LIBDEFLATE_A := $(1)/build/libdeflate.a)
	$(eval LIBRARY_FILES += $(LIBDEFLATE_A))
	$(eval LINKER_DIRS += -L $(LIBDEFLATE_A_DIR))
endef

# Add zstd	(!!! CHECK)
define ADD_LIBZSTD
	$(info *** Adding libzstd ***)
	$(eval INCLUDE_DIRS += -I$(1))
	$(eval LIBZSTD_DIR := $(1))
	$(eval LIBZSTD_A_DIR := $(1))
	$(eval LIBZSTD_A := $(1)/lib/libzstd.a)
	$(eval LIBRARY_FILES += $(LIBZSTD_A))
	$(eval LINKER_DIRS += -L $(LIBZSTD_A_DIR))
endef

# Add mimalloc
define ADD_MIMALLOC
	$(info *** Adding mimalloc ***)
	$(eval MIMALLOC_INCLUDE_DIR := $(1/include))
	$(eval INCLUDE_DIRS += -I$(1)/include)
	$(eval MIMALLOC_DIR := $(1))
	$(eval MIMALLOC_OBJ := $(1)/mimalloc.o)
endef

# Add RADULS-inplace (!!! CHECK)
define ADD_RADULS_INPLACE
	$(info *** Adding raduls-inplace ***)
	$(eval INCLUDE_DIRS += -I$(1)/Raduls)
	$(eval RADULS_INPLACE_DIR := $(1)/Raduls)
	$(eval RADULS_INPLACE_A_DIR := $(1)/Raduls)
	$(eval RADULS_INPLACE_A := $(1)/Raduls/libraduls.a)
	$(eval LIBRARY_FILES += $(RADULS_INPLACE_A))
	$(eval LINKER_DIRS += -L $(RADULS_INPLACE_A_DIR))
endef

# Add igraph (!!! CHECK)
define ADD_IGRAPH
	$(info *** Adding igraph ***)
	$(eval INCLUDE_DIRS += -I$(1)/include)
	$(eval IGRAPH_DIR := $(1))
	$(eval IGRAPH_A_DIR := $(1)/build/src)
	$(eval IGRAPH_A := $(IGRAPH_A_DIR)/libigraph.a)
	$(eval LIBRARY_FILES += $(IGRAPH_A))
	$(eval LINKER_DIRS += -L $(IGRAPH_A_DIR))
endef

# Add SBWT (!!! CHECK)
define ADD_SBWT
	$(info *** Adding SBWT ***)
	$(eval INCLUDE_DIRS += -I$(1)/include)
	$(eval SBWT_DIR := $(1))
	$(eval SBWT_A_DIR := $(1)/build)
	$(eval SBWT_A := $(SBWT_A_DIR)/libsbwt_static.a)
	$(eval SBWT_SDSL_A := $(SBWT_A_DIR)/external/sdsl-lite/build/lib/libsdsl.a)
	$(eval SBWT_KMC_CORE_A := $(SBWT_A_DIR)/external/KMC/build/libkmc_core.a)
	$(eval SBWT_KMC_TOOLS_A := $(SBWT_A_DIR)/external/KMC/build/libkmc_tools.a)
	$(eval LIBRARY_FILES += $(SBWT_A) $(SBWT_SDSL_A) $(SBWT_KMC_CORE_A) $(SBWT_KMC_TOOLS_A))
	$(eval LINKER_DIRS += -L $(SBWT_A_DIR))
endef

# Add REFRESH libs
define ADD_REFRESH_LIB
	$(info *** Adding REFRESH libs ***)
	$(eval INCLUDE_DIRS += -I$(1))
endef

# Add StatsLib
define ADD_STATS_LIB
	$(info *** Adding StatsLib ***)
	$(eval INCLUDE_DIRS += -I$(1)/include)
endef

# Add Annoy 
define ADD_ANNOY_LIB
	$(info *** Adding Annoy lib ***)
	$(eval INCLUDE_DIRS += -I$(1)/include)
endef

# Add hnswlib
define ADD_HNSWLIB
	$(info *** Adding hnswlib ***)
	$(eval INCLUDE_DIRS += -I$(1))
endef

# Add umappp lib
define ADD_UMAPPP_LIB
	$(info *** Adding UMAPPP lib ***)
	$(eval INCLUDE_DIRS += -I$(1)/include)
endef

# Add CppIrlba lib
define ADD_CPPIRLBA_LIB
	$(info *** Adding CppIrlba lib ***)
	$(eval INCLUDE_DIRS += -I$(1)/include)
endef

# Add CppKmeans lib
define ADD_CPPKMEANS_LIB
	$(info *** Adding CppIrlba lib ***)
	$(eval INCLUDE_DIRS += -I$(1)/include)
endef

# Add aarand lib
define ADD_AARAND_LIB
	$(info *** Adding aarand lib ***)
	$(eval INCLUDE_DIRS += -I$(1)/include)
endef

# Add knncolle lib
define ADD_KNNCOLLE_LIB
	$(info *** Adding knncolle lib ***)
	$(eval INCLUDE_DIRS += -I$(1)/include)
endef

# Add Eigen lib
define ADD_EIGEN_LIB
	$(info *** Adding Eigen lib ***)
	$(eval INCLUDE_DIRS += -I$(1))
endef

### Macros configuring compiler/linker flags
# Add os-specific flags for static linking
define SET_STATIC
	$(if $(filter true,$(1)), \
		$(if $(filter Darwin,$(OS_TYPE)), \
			$(eval STATIC_LFLAGS := -static-libgcc -static-libstdc++ -pthread), \
			$(if $(filter x86_64,$(ARCH_TYPE)), \
				$(eval STATIC_LFLAGS = -static -Wl,--whole-archive -lpthread -Wl,--no-whole-archive), \
				$(eval STATIC_LFLAGS = -static-libgcc -static-libstdc++ -lpthread) \
			)
		)
	)
endef

# Add C, C++ standards
define SET_C_CPP_STANDARDS
	$(eval C_STD := $(1))
	$(eval CPP_STD := $(2))
endef

# Add type-specifix flags
define SET_FLAGS
	$(if $(filter Linux_x86_64,$(OS_ARCH_TYPE)), \
		$(eval PLATFORM_SPECIFIC_C_FLAGS := ) \
		$(eval PLATFORM_SPECIFIC_CPP_FLAGS := ) \
		$(eval PLATFORM_SPECIFIC_LINKER_FLAGS := -fabi-version=6), \
		$(if $(filter Linux_aarch64,$(OS_ARCH_TYPE)), \
			$(eval PLATFORM_SPECIFIC_C_FLAGS := ) \
			$(eval PLATFORM_SPECIFIC_CPP_FLAGS := -ffp-contract=off) \
			$(eval PLATFORM_SPECIFIC_LINKER_FLAGS := -fabi-version=6), \
			$(if $(filter Darwin_arm64,$(OS_ARCH_TYPE)) \
				$(eval PLATFORM_SPECIFIC_C_FLAGS := ) \
				$(eval PLATFORM_SPECIFIC_CPP_FLAGS := ) \
				$(eval PLATFORM_SPECIFIC_LINKER_FLAGS := ), \
				$(if $(filter Darwin_x86_64,$(OS_ARCH_TYPE)) \
					$(eval PLATFORM_SPECIFIC_C_FLAGS := ) \
					$(eval PLATFORM_SPECIFIC_CPP_FLAGS := ) \
					$(eval PLATFORM_SPECIFIC_LINKER_FLAGS := ) \
				) \
			) \
		) \
	)

	$(eval C_FLAGS += -std=$(C_STD) -Wall -fPIC -pthread -fpermissive $(PLATFORM_SPECIFIC_C_FLAGS))
	$(eval CPP_FLAGS += -std=$(CPP_STD) -Wall -fPIC -pthread -fpermissive $(PLATFORM_SPECIFIC_CPP_FLAGS))
	$(eval LINKER_FLAGS += -lm -lpthread $(PLATFORM_SPECIFIC_LINKER_FLAGS))

	$(if $(filter release,$(1)), \
		$(eval OPTIMIZATION_FLAGS += -O3) \
		$(eval C_FLAGS += ) \
		$(eval CPP_FLAGS +=  ), \
		$(if $(filter debug,$(1)), \
			$(eval OPTIMIZATION_FLAGS += -O0 -g) \
			$(eval C_FLAGS += ) \
			$(eval CPP_FLAGS +=  ), \
		)
	)

	$(eval CPP_FLAGS_SSE2 := $(CPPFLAGS) -msse2)
	$(eval CPP_FLAGS_SSE4 := $(CPPFLAGS) -msse4)
	$(eval CPP_FLAGS_AVX := $(CPPFLAGS) -mavx)
	$(eval CPP_FLAGS_AVX2 := $(CPPFLAGS) -mavx2)
	$(eval CPP_FLAGS_AVX512 := $(CPPFLAGS) -mavx512)
	$(eval CPP_FLAGS_NEON := $(CPPFLAGS) -mneon)
endef


### Macros checking system and software
# Check for NASM
define CHECK_NASM
	$(eval NASM_VERSION := $(shell nasm --version 2>/dev/null))
endef

# Choose lib for gzip decompression
define CHOOSE_GZIP_DECOMPRESSION
	$(if $(filter x86_64,$(ARCH_TYPE)), \
    	$(if $(and $(NASM_VERSION),$(ISAL_DIR)), \
			$(eval GZ_TARGET := isa-l), \
			$(eval GZ_TARGET := zlib-ng) \
		), \
		$(eval GZ_TARGET := zlib-ng) \
  	)

	$(if $(filter isa-l,$(GZ_TARGET)), \
		$(info ISAL will be used for gzip decompression) \
		$(eval GZ_LIB := isa-l.a) \
		$(eval LIBRARY_FILES += $(ISAL_A)) \
		$(eval LINKER_DIRS += -L $(ISAL_A_DIR))
		$(eval C_FLAGS += -DREFRESH_USE_IGZIP) \
		$(eval CPP_FLAGS += -DREFRESH_USE_IGZIP), \
		$(info zlib-ng will be used for gzip decompression) \
		$(eval GZ_LIB := libz.a) \
		$(eval LIBRARY_FILES += $(ZLIB_A)) \
		$(eval LINKER_DIRS += -L $(ZLIB_A_DIR))
		$(eval C_FLAGS += -DREFRESH_USE_ZLIB) \
		$(eval CPP_FLAGS += -DREFRESH_USE_ZLIB) \
	)
endef

# Check for OS and architecture
define CHECK_OS_ARCH
	$(if $(MSVC), \
		$(eval OS_TYPE := windows) \
		$(eval ARCH_TYPE := x86_64), \
		$(eval OS_TYPE := $(shell uname -s 2>/dev/null || echo not)) \
		$(eval ARCH_TYPE := $(shell uname -m 2>/dev/null || echo not)) \
		)

	$(eval OS_ARCH_TYPE := $(OS_TYPE)_$(ARCH_TYPE))

	$(if $(filter arm8,$(PLATFORM)), \
		$(eval ARCH_FLAGS := -march=armv8-a -DARCH_ARM) \
		$(info *** ARMv8 with NEON extensions ***), \
		$(if $(filter m1,$(PLATFORM)), \
			$(eval ARCH_FLAGS := -march=armv8.4-a -DARCH_ARM) \
			$(info *** Apple M1 (or newer) with NEON extensions ***), \
			$(if $(filter sse2,$(PLATFORM)), \
				$(eval ARCH_FLAGS := -msse2 -m64 -DARCH_X64) \
				$(info *** x86-64 with SSE2 extensions ***), \
				$(if $(filter avx,$(PLATFORM)), \
					$(eval ARCH_FLAGS := -mavx -m64 -DARCH_X64) \
					$(info *** x86-64 with AVX extensions ***), \
					$(if $(filter avx2,$(PLATFORM)), \
						$(eval ARCH_FLAGS := -mavx2 -m64 -DARCH_X64) \
						$(info *** x86-64 with AVX2 extensions ***), \
						$(if $(filter x86_64,$(ARCH_TYPE)), \
							$(eval ARCH_FLAGS := -march=native -DARCH_X64) \
							$(info *** Unspecified platform - using native compilation for x86_64 ***), \
							$(eval ARCH_FLAGS := -march=native -DARCH_ARM) \
							$(info *** Unspecified platform - using native compilation for ARM ***)))))))

endef

# Load submodules if necessary
define INIT_SUBMODULES
	$(info *** Initialization of submodules ***)
	$(eval dummy := $(shell git submodule update --init --recursive))
endef


### Library targets
zlib-ng:
	cd $(ZLIB_DIR); cmake -DCMAKE_CXX_COMPILER=$(CXX) -DCMAKE_C_COMPILER=$(CC) -B build-g++/zlib-ng -S . -DZLIB_COMPAT=ON; cmake --build build-g++/zlib-ng --config Release

isa-l:
	cd $(ISAL_DIR) && $(MAKE) -f Makefile.unx

libdeflate:
	cd $(LIBDEFLATE_DIR) && cmake -B build && cmake --build build

libzstd:
	cd $(LIBZSTD_DIR) && $(MAKE)

radule-inplace:
	cd $(RADULS_DIR) && $(MAKE)

igraph:
	cd $(IGRAPH_DIR); cmake -S libs/igraph -B libs/igraph/build; cmake -S libs/igraph -B libs/igraph/build; cmake --build libs/igraph/build

mimalloc_obj:
	cd $(MIMALLOC_DIR) && $(CXX) -DMI_MALLOC_OVERRIDE -O3 -DNDEBUG -fPIC -Wall -Wextra -Wno-unknown-pragmas -fvisibility=hidden -ftls-model=initial-exec -fno-builtin-malloc -c -I include src/static.c -o mimalloc.o

sbwt:
	cd $(SBWT_DIR)/SBWT/build; cmake -DCMAKE_CXX_COMPILER=$(CXX) -DCMAKE_C_COMPILER=$(CC) .. -DMAX_KMER_LENGTH=32; $(MAKE) -j


### Clean library targets
clean-zlib-ng:
	-cd $(ZLIB_DIR) && $(MAKE) -f Makefile.in clean

clean-isa-l:
	-cd $(ISAL_DIR) && $(MAKE) -f Makefile.unx clean

clean-libdeflate:
	-cd $(LIBDEFLATE_DIR) && rm -r build

clean-libzstd:
	-cd $(LIBZSTD_DIR) && $(MAKE) clean

clean-radule-inplace:
	-cd $(RADULS_DIR) && $(MAKE) clean

clean-igraph:
	-rm -r $(IGRAPH_DIR)/build

clean-mimalloc_obj:
	-rm $(MIMALLOC_OBJ)

clean-sbwt:
	-rm $(SBWT_A)
	-rm $(SBWT_SDSL_A)
	-rm $(SBWT_KMC_CORE_A)
	-rm $(SBWT_KMC_TOOLS_A)


### Testing
testing:
	@echo "OS_TYPE: $(OS_TYPE)"
	@echo "ARCH_TYPE: $(ARCH_TYPE)"
	@echo "OS_ARCH_TYPE: $(OS_ARCH_TYPE)"
	@echo "ARCH_FLAGS: $(ARCH_FLAGS)"
	@echo "INCLUDE_DIRS: $(INCLUDE_DIRS)"
	@echo "Check NASM: $(NASM_VERSION)"
	@echo "ISAL_DIR: $(ISAL_DIR)"
	@echo "GZ_LIB: $(GZ_LIB)"
	@echo "GZ_TARGET: $(GZ_TARGET)"
	@echo "MIMALLOC_OBJ: $(MIMALLOC_OBJ)"
	@echo "STATIC_LFLAGS: $(STATIC_LFLAGS)"
	@echo "C_STD: $(C_STD)"
	@echo "CPP_STD: $(CPP_STD)"
	@echo "C_FLAGS: $(C_FLAGS)"
	@echo "CPP_FLAGS: $(CPP_FLAGS)"
	@echo "LINKER_FLAGS: $(LINKER_FLAGS)"
	@echo "OPTIMIZATION_FLAGS: $(OPTIMIZATION_FLAGS)"
	@echo "CPP_FLAGS_SSE4: $(CPP_FLAGS_SSE4)"
