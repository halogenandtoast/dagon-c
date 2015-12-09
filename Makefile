all: dagonc

.PHONY: dagonc
dagonc:
	$(MAKE) -C $@

clean:
	cd dagonc && $(MAKE) clean
