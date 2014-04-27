
default:main

.DEFAULT:
	cd src && $(MAKE) $@

clean:
	cd src && $(MAKE) $@
