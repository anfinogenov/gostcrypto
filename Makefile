TARGET=crypt
LIB=./lib/

PREFIX=/usr/local/bin/

SOURCES=main.cpp

GCC=$(CFLAGS) g++ -std=c++11 -Wall -pedantic -O2

HASH=hash_3411
KDF=pbkdf2
ELLIPTIC=elliptic
CIPHER=cipher_3412
MODE=ofbmode

#LIBFILES=$(LIB)lib$(HASH).so $(LIB)lib$(KDF).so \
$(LIB)lib$(ELLIPTIC).so $(LIB)lib$(CIPHER).so $(LIB)lib$(MODE).so
LIBMAKE=$(MAKE) static

#LIBS=-l$(HASH) -l$(KDF) -l$(ELLIPTIC) -l$(CIPHER) -l$(MODE)
LIBS=$(LIB)lib$(HASH).a $(LIB)lib$(KDF).a $(LIB)lib$(ELLIPTIC).a \
     $(LIB)lib$(CIPHER).a $(LIB)lib$(MODE).a
#LDFLAGS=-L$(LIB) $(LIBS) -Wl,-rpath,$(LIB)
LDFLAGS=$(LIBS)

TARGETECHO=@echo -e "\033[00;36mBuilding \033[00;32m$@\033[00m"

.PHONY: all clean install uninstall libs

all: $(TARGET)

clean:
	rm $(TARGET)

$(TARGET): $(SOURCES) libs
	$(TARGETECHO)
	$(GCC) $(SOURCES) $(LDFLAGS) -o $(TARGET) -lgmp -lgmpxx



libs: $(LIBS) #$(LIBFILES)

$(LIB)lib$(HASH).a: ./hash/*
	$(TARGETECHO)
	@cd ./hash && $(LIBMAKE)
	@echo

$(LIB)lib$(KDF).a: ./pbkdf2/*
	$(TARGETECHO)
	@cd ./pbkdf2 && $(LIBMAKE)
	@echo

$(LIB)lib$(ELLIPTIC).a: ./elliptic/*
	$(TARGETECHO)
	@cd ./elliptic && $(LIBMAKE)
	@echo

$(LIB)lib$(CIPHER).a: ./kuznyechik/
	$(TARGETECHO)
	@cd ./kuznyechik && $(LIBMAKE)
	@echo

$(LIB)lib$(MODE).a: ./mode/ofb/*
	$(TARGETECHO)
	@cd ./mode/ofb && $(LIBMAKE)
	@echo



install: all
	install --mode=0644 -t $(PREFIX) $(TARGET)

uninstall:
	-rm $(PREFIX)$(TARGET)

