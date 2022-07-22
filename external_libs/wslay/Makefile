
.SILENT:

.PHONY: all lib clean

all: lib
	echo "123 $(CFLAGS)"
	$(MAKE) -s -C lib

lib:
	$(MAKE) -s -C lib
	
clean:
	$(MAKE) -C lib clean
