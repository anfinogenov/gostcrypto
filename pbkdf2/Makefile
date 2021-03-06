LIBNAME=pbkdf2
TMP=./tmp/
LIB=../lib/
SRC=./source/
HDR=./headers/

PREFIXLIB=/usr/local/lib/
PREFIXINCLUDE=/usr/local/include/

SOURCES=$(LIBNAME).c
HEADERS=$(LIBNAME).h
OBJECT=$(LIBNAME).o
SLIBNAME=lib$(LIBNAME).a
DLIBNAME=lib$(LIBNAME).so

GCC=$(CFLAGS) gcc -std=c99 -Wall -Wextra -pedantic -O2

.PHONY: all clean install uninstall static dynamic

all: static dynamic

clean:
	rm $(LIB)$(SLIBNAME) $(LIB)$(DLIBNAME) $(TMP)$(OBJECT)

static: $(LIB)$(SLIBNAME)

dynamic: $(LIB)$(DLIBNAME)

#static
$(LIB)$(SLIBNAME): $(TMP)$(OBJECT)
	@$(MAKE) $(LIB) > /dev/null 2>&1
	ar cr $(LIB)$(SLIBNAME) $(TMP)$(OBJECT)

#dynamic
$(LIB)$(DLIBNAME): $(TMP)$(OBJECT)
	@$(MAKE) $(LIB) > /dev/null 2>&1
	$(GCC) -shared $(TMP)$(OBJECT) -o $(LIB)$(DLIBNAME)

$(TMP)$(OBJECT): $(SRC)$(SOURCES) $(HDR)*
	@$(MAKE) $(TMP) > /dev/null 2>&1
	$(GCC) -fpic -c $(SRC)$(SOURCES) -o $(TMP)$(OBJECT)

debug: $(LIB)$(DLIBNAME) $(SRC)debug.c
	$(GCC) $(SRC)debug.c -lhash_3411 -l$(LIBNAME) -L../hash/lib -L$(LIB) -Wl,-rpath,../hash/lib -Wl,-rpath,$(LIB) -o debug

install: all
	install --mode=0644 -t $(PREFIXINCLUDE) $(HDR)$(HEADERS)
	install --mode=0644 -t $(PREFIXLIB) $(LIB)$(SLIBNAME)
	install --mode=0755 -t $(PREFIXLIB) $(LIB)$(DLIBNAME)

uninstall:
	-rm $(PREFIXINCLUDE)$(HEADERS) $(PREFIXLIB)$(SLIBNAME) $(PREFIXLIB)$(DLIBNAME)

$(TMP):
	-mkdir $(TMP)

$(LIB):
	-mkdir $(LIB)

