LIBNAME=ofbmode
TMP=./tmp/
LIB=../../lib/
SRC=./
HDR=./
HDR2=../headerscommon/

PREFIXLIB=/usr/local/lib/
PREFIXINCLUDE=/usr/local/include/

SOURCES=$(LIBNAME).cpp
HEADERS=$(LIBNAME).hpp
OBJECT=$(LIBNAME).o
SLIBNAME=lib$(LIBNAME).a
DLIBNAME=lib$(LIBNAME).so

GCC=g++ -std=c++11 -Wall -Wextra -pedantic -Ofast -march=native

.PHONY: all clean install uninstall static dynamic

all: static #dynamic

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
	g++ -shared $(TMP)$(OBJECT) -o $(LIB)$(DLIBNAME)

$(TMP)$(OBJECT): $(SRC)$(SOURCES) $(HDR)$(HEADERS) #$(HDR2)*
	@$(MAKE) $(TMP) > /dev/null 2>&1
	$(GCC) -fpic -c $(SRC)$(SOURCES) -o $(TMP)$(OBJECT)


install: all
	install --mode=0644 -t $(PREFIXINCLUDE) $(HDR)$(HEADERS)
	install --mode=0644 -t $(PREFIXLIB) $(LIB)$(SLIBNAME)
	install --mode=0755 -t $(PREFIXLIB) $(LIB)$(DLIBNAME)

uninstall:
	-rm $(PREFIXINCLUDE)$(HEADERS) $(PREFIXLIB)$(SLIBNAME) $(PREFIXLIB)$(DLIBNAME)

$(LIB):
	-mkdir $(LIB)

$(TMP):
	-mkdir $(TMP)

