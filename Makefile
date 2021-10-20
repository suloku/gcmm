default: all

all: light

light: wii gc

dark: wii-dark gc-dark

clean: gc-clean wii-clean

wii:
	$(MAKE) -f Makefile.wii
	
wii-dark:
	$(MAKE) -f Makefile.wii THEME=-DDARK_MODE
	
wii-clean:
	$(MAKE) -f Makefile.wii clean

gc:
	$(MAKE) -f Makefile.gc

gc-dark:
	$(MAKE) -f Makefile.gc THEME=-DDARK_MODE

gc-clean:
	$(MAKE) -f Makefile.gc clean

runwii:
	$(MAKE) -f Makefile.wii run

rungc:
	$(MAKE) -f Makefile.gc run
	
run: runwii
