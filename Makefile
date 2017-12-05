TARGET=crypt
LIB=./lib/

PREFIX=/usr/local/bin/

SOURCES=main.cpp

GCC=$(CFLAGS) g++ -std=c++11 -Wall -pedantic -O2

HASH=hash_3411
KDF=pbkdf2
HYBRID=hybrid
CIPHER=kuznyechik
MODE=ctrmode

LIBFILES=$(LIB)lib$(HASH).so $(LIB)lib$(KDF).so \
$(LIB)lib$(HYBRID).so $(LIB)lib$(MODE).so #$(LIB)lib$(CIPHER).so

LIBS=-l$(HASH) -l$(KDF) -l$(HYBRID) -l$(MODE) #-l$(CIPHER)
LDFLAGS=-L$(LIB) $(LIBS) -Wl,-rpath,$(LIB)

.PHONY: all clean install uninstall libs

all: $(TARGET)

clean:
	rm $(TARGET)

$(TARGET): $(SOURCES) libs
	$(GCC) $(SOURCES) $(LDFLAGS) -o $(TARGET)



libs: $(LIBFILES)

$(LIB)lib$(HASH).so:
	cd ./hash && $(MAKE)

$(LIB)lib$(KDF).so:
	cd ./pbkdf2 && $(MAKE)

$(LIB)lib$(HYBRID).so:
	cd ./hybrid && $(MAKE)

$(LIB)lib$(CIPHER).so:
	cd ./kuznyechik && $(MAKE)

$(LIB)lib$(MODE).so:
	cd ./mode/ctr && $(MAKE)



install: all
	install --mode=0644 -t $(PREFIX) $(TARGET)

uninstall:
	-rm $(PREFIX)$(TARGET)

