all: util-multi-fasta-split

util-multi-fasta-split:
	cd multi-fasta-split && $(MAKE) -j
	mkdir -p bin
	cp multi-fasta-split/multi-fasta-split bin/

clean:
	cd multi-fasta-split && $(MAKE) clean
	-rm bin/*
	rmdir bin
	