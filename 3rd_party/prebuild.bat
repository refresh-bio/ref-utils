rem %1 - $(SolutionDir)
rem %2 - $(Configuration)

cd %1\3rd_party

@echo "zlib-ng"
cd zlib-ng 
cmake -B build-vs/zlib-ng -S . -DZLIB_COMPAT=ON
cmake --build build-vs/zlib-ng --config %2
cd ..

rem @echo "isa-l"
rem cd isa-l
rem nmake -f Makefile.nmake
rem cd ..

rem @echo "libdeflate"
rem cd libdeflate
rem cmake -B build
rem cmake --build build --config %2
rem cd ..
