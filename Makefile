default: all

all: wii gc

clean: gc-clean wii-clean

wii:
	$(MAKE) -f Makefile.wii
	
wii-clean:
	$(MAKE) -f Makefile.wii clean

gc:
	$(MAKE) -f Makefile.gc

gc-clean:
	$(MAKE) -f Makefile.gc clean

runwii:
	$(MAKE) -f Makefile.wii run

rungc:
	$(MAKE) -f Makefile.gc run
	
run: runwii
