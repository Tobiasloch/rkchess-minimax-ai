ifneq ($(words $(MAKECMDGOALS)),1)
.DEFAULT_GOAL = all
%:
	@$(MAKE) $@ --no-print-directory -rRf $(firstword $(MAKEFILE_LIST))
else
ifndef ECHO
T := $(shell $(MAKE) $(MAKECMDGOALS) --no-print-directory \
      -nrRf $(firstword $(MAKEFILE_LIST)) \
      ECHO="COUNTTHIS" | grep -c "COUNTTHIS")

N := x
C = $(words $N)$(eval N := x $N)
ECHO = echo "`expr " [\`expr $C '*' 100 / $T\`" : '.*\(....\)$$'`%]"
endif

# Install
BIN=rkchess
BINTEST=test
CC=gcc
CCPP=g++

SRCDIR = src
LIBDIR = lib
TESTDIR = test
BINDIR = bin
OBJECTDIR = ${BINDIR}/obj

# Flags
# CFLAGS += -std=gnu11 -O2
CFLAGS += -std=gnu11 -g #debug
CPPFLAGS += -std=c++11
IFLAGS = -I ${LIBDIR} -I ${SRCDIR}

SRCCPP = ${shell find ${SRCDIR} -type f -name "*.cpp"}
SRCC = ${shell find ${SRCDIR} -not -path src/ai/tablebases/generateStartGameStatesmain.c -type f -name "*.c"}
TESTSRC = ${shell find ${TESTDIR} -type f -name "*.c"}

LIBSVGSRC = ${shell find ${LIBDIR}/sf-svg/SFC -type f -name "*.cpp"}
LIBSRC = ${LIBDIR}/jsonLoader.c ${LIBDIR}/jsmn.c
LIBSRCTEST = ${LIBSRC} ${LIBDIR}/CuTest.c
LIBSRCPP = ${LIBSVGSRC}

# the OBJC from SRCC are in the folder ${OBJECTDIR} and have the same name and path from there SRCC
OBJC = ${SRCC:%.c=${OBJECTDIR}/%.o} ${LIBSRC:%.c=${OBJECTDIR}/%.o}
OBJTEST = ${TESTSRC:%.c=${OBJECTDIR}/%.o} ${LIBSRCTEST:%.c=${OBJECTDIR}/%.o}
OBJCPP = ${SRCCPP:%.cpp=${OBJECTDIR}/%.o} ${LIBSRCPP:%.cpp=${OBJECTDIR}/%.o}
DEP = ${OBJC:%.o=%.d} ${OBJCPP:%.o=%.d}

CLIBS = -lm
CPPLIBS = -lsfml-graphics -lsfml-window -lsfml-system -lm

ifeq (${OS},Windows_NT)
BIN := ${BIN}.exe
BINTEST := ${BINTEST}.exe
endif

.PHONY: all clean test testRun rkchess

all: rkchess test
	@$(ECHO) All done
rkchess: ${BINDIR}/${BIN}
test: ${BINDIR}/${BINTEST}

${BINDIR}/${BIN}: ${OBJC} ${OBJCPP}
	@$(ECHO) Linking $@
	@rm -f ${BINDIR}/${BIN}
	@mkdir -p ${BINDIR}
	@${CCPP} $^ ${CPPFLAGS} -o $@ ${IFLAGS} ${CPPLIBS}

${BINDIR}/${BINTEST}: ${OBJC} ${OBJTEST}
	@$(ECHO) Linking $@
	@rm -f ${BINDIR}/${BINTEST}
	@mkdir -p ${BINDIR}
	@${CC} $^ ${CPPFLAGS} -D TEST -o $@ ${IFLAGS} -I test ${CPPLIBS}

testRun: test
	@$(ECHO) Running tests...
	@./${BINDIR}/${BINTEST}

-include ${DEP}

${OBJECTDIR}/%.o: %.c
	@$(ECHO) Compiling $@
	@mkdir -p ${@D}
	@${CC} ${CFLAGS} $< -MMD -c -o $@ ${IFLAGS} ${CLIBS}

${OBJECTDIR}/%.o: %.cpp
	@$(ECHO) Compiling $@
	@mkdir -p ${@D}
	@${CCPP} ${CPPFLAGS} $< -MMD -c -o $@ ${IFLAGS} ${CPPLIBS}

clean:
	rm -rf ${BINDIR}

endif