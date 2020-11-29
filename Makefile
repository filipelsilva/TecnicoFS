# General Makefile

.PHONY: all clean zip unzip client server

all:
	(cd client; make all)
	(cd server; make all)

clean: rmzip
	(cd client; make clean)
	(cd server; make clean)

zip: # Uses git, not very portable in zip
	git archive -o entrega.zip HEAD
	
rmzip:
	rm -f entrega.zip
