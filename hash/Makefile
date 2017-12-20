LIBNAME=hash_3411
TMP=./tmp/
LIB=../lib/
SRC=./source/
HDR=./headers/

PREFIXLIB=/usr/local/lib/
PREFIXINCLUDE=/usr/local/include/

SOURCES=$(LIBNAME).c
SOURCESCPP=$(LIBNAME).cpp
HEADERS=$(LIBNAME).h
OBJECT=$(LIBNAME).o
OBJECTCPP=$(LIBNAME)cpp.o
SLIBNAME=lib$(LIBNAME).a
DLIBNAME=lib$(LIBNAME).so

GCCFLAGS=-Wall -pedantic -O2 -march=native
GCC=$(CFLAGS) gcc -std=c99 $(GCCFLAGS)
GPP=$(CFLAGS) g++ -std=c++11 $(GCCFLAGS)

.PHONY: all clean install uninstall static dynamic

all: static dynamic

clean:
	rm $(LIB)$(SLIBNAME) $(LIB)$(DLIBNAME) $(TMP)$(OBJECT)

static: $(LIB)$(SLIBNAME)

dynamic: $(LIB)$(DLIBNAME)

#static
$(LIB)$(SLIBNAME): $(TMP)$(OBJECT) $(TMP)$(OBJECTCPP)
	@$(MAKE) $(LIB) > /dev/null 2>&1
	ar cr $(LIB)$(SLIBNAME) $(TMP)$(OBJECT) $(TMP)$(OBJECTCPP)

#dynamic
$(LIB)$(DLIBNAME): $(TMP)$(OBJECT) $(TMP)$(OBJECTCPP)
	@$(MAKE) $(LIB) > /dev/null 2>&1
	$(GCC) -shared $(TMP)$(OBJECT) $(TMP)$(OBJECTCPP) -o $(LIB)$(DLIBNAME)

#object
$(TMP)$(OBJECT): $(SRC)$(SOURCES) $(HDR)*
	@$(MAKE) $(TMP) > /dev/null 2>&1
	$(GCC) -fpic -c $(SRC)$(SOURCES) -o $(TMP)$(OBJECT)

#cpp object
$(TMP)$(OBJECTCPP): $(SRC)$(SOURCESCPP) $(HDR)*
	@$(MAKE) $(TMP) > /dev/null 2>&1
	$(GPP) -fpic -c $(SRC)$(SOURCESCPP) -o $(TMP)$(OBJECTCPP)

#debug
debug: $(LIB)$(DLIBNAME) $(SRC)debug.c
	$(GCC) $(SRC)debug.c -l$(LIBNAME) -L$(LIB) -Wl,-rpath,$(LIB) -o debug

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

