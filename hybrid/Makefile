LIBNAME=hybrid
TMP=./tmp/
LIB=./lib/
SRC=./source/
HDR=./headers/

PREFIXLIB=/usr/local/lib/
PREFIXINCLUDE=/usr/local/include/

SOURCES=$(LIBNAME).cpp
HEADERS=$(LIBNAME).hpp
OBJECT=$(LIBNAME).o
SLIBNAME=lib$(LIBNAME).a
DLIBNAME=lib$(LIBNAME).so

GCC=$(CFLAGS) g++ -std=c++11 -Wall -pedantic -O2 -lgmp -lgmpxx

SHAREDLIBS=-lhash_3411 -lpbkdf2 -l$(LIBNAME)
HASHLIBPATH=../hash/lib/
KDFLIBPATH=../pbkdf2/lib/
LDFLAGS=-L$(HASHLIBPATH) -L$(KDFLIBPATH) -L$(LIB) $(SHAREDLIBS) \
-Wl,-rpath,$(HASHLIBPATH) -Wl,-rpath,$(KDFLIBPATH) -Wl,-rpath,$(LIB)

.PHONY: all clean install uninstall static dynamic

all: static dynamic

clean:
	rm $(LIB)$(SLIBNAME) $(LIB)$(DLIBNAME) $(TMP)$(OBJECT)

static: $(LIB)$(SLIBNAME)

dynamic: $(LIB)$(DLIBNAME)

#static
$(LIB)$(SLIBNAME): $(TMP)$(OBJECT)
	ar cr $(LIB)$(SLIBNAME) $(TMP)$(OBJECT)

#dynamic
$(LIB)$(DLIBNAME): $(TMP)$(OBJECT)
	$(GCC) -shared $(TMP)$(OBJECT) -o $(LIB)$(DLIBNAME)

$(TMP)$(OBJECT): $(SRC)$(SOURCES) $(HDR)*
	$(GCC) -fpic -c $(SRC)$(SOURCES) -o $(TMP)$(OBJECT)

debug: $(LIB)$(DLIBNAME) $(SRC)debug.cpp
	$(GCC) -c $(SRC)debug.cpp -o $(TMP)debug.o
	$(GCC) $(TMP)debug.o $(LDFLAGS) -o debug

install: all
	install --mode=0644 -t $(PREFIXINCLUDE) $(HDR)$(HEADERS)
	install --mode=0644 -t $(PREFIXLIB) $(LIB)$(SLIBNAME)
	install --mode=0755 -t $(PREFIXLIB) $(LIB)$(DLIBNAME)

uninstall:
	-rm $(PREFIXINCLUDE)$(HEADERS) $(PREFIXLIB)$(SLIBNAME) $(PREFIXLIB)$(DLIBNAME)

