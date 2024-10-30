all: util-multi-fasta-split util-mfasta-tool

util-multi-fasta-split:
	cd multi-fasta-split && $(MAKE) -j
	mkdir -p bin
	cp multi-fasta-split/multi-fasta-split bin/

util-mfasta-tool:
	cd mfasta-tool && $(MAKE) -j
	mkdir -p bin
	cp mfasta-tool/mfasta-tool bin/

clean:
	cd multi-fasta-split && $(MAKE) clean
	cd mfasta-tool && $(MAKE) clean
	-rm bin/*
	rmdir bin
	