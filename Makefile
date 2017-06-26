SHELL := /bin/bash
PATH := bin:dagon:$(PATH)

all: dagon

.PHONY: dagon
dagon:
	$(MAKE) -C $@

test:
	./bin/dspec


clean:
	cd dagon && $(MAKE) clean
