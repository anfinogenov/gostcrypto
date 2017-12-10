TARGET=crypt
LIB=./lib/

PREFIX=/usr/local/bin/

SOURCES=main.cpp

GCC=$(CFLAGS) g++ -std=c++11 -Wall -pedantic -O2

HASH=hash_3411
KDF=pbkdf2
ELLIPTIC=elliptic
CIPHER=cipher_3412
MODE=ctrmode

LIBFILES=$(LIB)lib$(HASH).so $(LIB)lib$(KDF).so \
$(LIB)lib$(ELLIPTIC).so $(LIB)lib$(CIPHER).so #$(LIB)lib$(MODE).so
LIBMAKE=$(MAKE) dynamic

LIBS=-l$(HASH) -l$(KDF) -l$(ELLIPTIC) -l$(CIPHER) #-l$(MODE)
LDFLAGS=-L$(LIB) $(LIBS) -Wl,-rpath,$(LIB)

TARGETECHO=@echo -e "\033[00;36mBuilding \033[00;32m$@\033[00m"

.PHONY: all clean install uninstall libs

all: $(TARGET)

clean:
	rm $(TARGET)

$(TARGET): $(SOURCES) libs
	$(TARGETECHO)
	$(GCC) $(SOURCES) $(LDFLAGS) -o $(TARGET)



libs: $(LIBFILES)

$(LIB)lib$(HASH).so: ./hash/*
	$(TARGETECHO)
	@cd ./hash && $(LIBMAKE)
	@echo

$(LIB)lib$(KDF).so: ./pbkdf2/*
	$(TARGETECHO)
	@cd ./pbkdf2 && $(LIBMAKE)
	@echo

$(LIB)lib$(ELLIPTIC).so: ./elliptic/*
	$(TARGETECHO)
	@cd ./elliptic && $(LIBMAKE)
	@echo

$(LIB)lib$(CIPHER).so: ./kuznyechik/
	$(TARGETECHO)
	@cd ./kuznyechik && $(LIBMAKE)
	@echo

$(LIB)lib$(MODE).so: ./mode/ctr/*
	$(TARGETECHO)
	@cd ./mode/ctr && $(LIBMAKE)
	@echo



install: all
	install --mode=0644 -t $(PREFIX) $(TARGET)

uninstall:
	-rm $(PREFIX)$(TARGET)

