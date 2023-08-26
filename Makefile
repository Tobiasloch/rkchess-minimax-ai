# Install
BIN=rkchess
CC=gcc
CCPP=g++

SRCDIR = src
LIBDIR = lib
TESTDIR = test
BINDIR = bin
OBJECTDIR = ${BINDIR}/obj

# Flags
# CFLAGS += -std=gnu11 -Wall -Wextra -pedantic -O2
CPPFLAGS += -std=c++11
CFLAGS += -std=gnu11 -g #debug
IFLAGS = -I $(LIBDIR) -I $(SRCDIR)

SRCCPP = $(shell find $(SRCDIR) -type f -name "*.cpp")
DEPENDSCPP := $(patsubst %.cpp,%.d,$(SRCCPP))
SRCC = $(shell find $(SRCDIR) -not -path src/ai/tablebases/generateStartGameStatesmain.c -type f -name "*.c")
DEPENDSC := $(patsubst %.cpp,%.d,$(SRCC))
HEADERS = $(shell find $(SRCDIR) -type f -name "*.h")

LIBSVGSRC = $(shell find $(LIBDIR)/sf-svg/SFC -type f -name "*.cpp")
LIBSRC = $(LIBDIR)/jsonLoader.c $(LIBDIR)/jsmn.c
LIBSRCPP = $(LIBSVGSRC)

# the OBJC from SRCC are in the folder ${OBJECTDIR} and have the same name and path from there SRCC
OBJC = ${SRCC:%.c=${OBJECTDIR}/%.o} ${LIBSRC:%.c=${OBJECTDIR}/%.o}
OBJCPP = ${SRCCPP:%.cpp=${OBJECTDIR}/%.o} ${LIBSRCPP:%.cpp=${OBJECTDIR}/%.o}

CLIBS = -lm
CPPLIBS = -lsfml-graphics -lsfml-window -lsfml-system -lm

ifeq ($(OS),Windows_NT)
BIN := $(BIN).exe
endif

.PHONE: all clean

all: ${BINDIR}/$(BIN)

${BINDIR}/$(BIN): ${OBJC} ${OBJCPP}
	@mkdir -p ${BINDIR}
	rm -f ${BINDIR}/$(BIN)
	$(CCPP) ${OBJC} ${OBJCPP} $(CPPFLAGS) -o $@ ${IFLAGS} ${CPPLIBS}

${OBJC}: ${SRCC} ${HEADERS}
	mkdir -p $(@D)
	$(CC) $(CFLAGS) ${@:${OBJECTDIR}/%.o=%.c} -c -o $@ ${IFLAGS} ${CLIBS}

${OBJCPP}: ${SRCCPP} ${HEADERS}
	mkdir -p $(@D)
	$(CCPP) $(CPPFLAGS) ${@:${OBJECTDIR}/%.o=%.cpp} -c -o $@ ${IFLAGS} ${CPPLIBS}

clean:
	rm -rf ${BINDIR}