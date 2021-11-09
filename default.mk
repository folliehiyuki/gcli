# Copyright 2021 Nico Sonack <nsonack@outlook.com>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

# WHY ARE THESE LINES SWAPPED?
# To work around a bug in GNU make.
-include ${DEPS}
-include config.mk

.SILENT:
.DEFAULT_TARGET: all

PROGS	?=	${PROG}
SRCS	?=	${${PROGS:=_SRCS}} ${${LIBS:=_SRCS}}
OBJS	=	${SRCS:.c=.o}
DEPS	=	${SRCS:.c=.d}

all: Makefile config.mk
	${MAKE} -f Makefile depend

config.mk: autodetect.sh
	rm -f config.mk
	@echo " ==> Performing autodetection of system variables"
	./autodetect.sh "${MAKE}"

.PHONY: build clean install depend ${MAN:=-install}
build: default.mk ${PROGS} ${LIBS}
	@echo " ==> Compilation finished"

depend: ${DEPS}
	@echo " ==> Starting build"
	${MAKE} -f Makefile build

.SUFFIXES: .c .d
.c.d:
	@echo " ==> Generating dependencies for $<"
	${CC} ${MKDEPS_FLAGS} $< > $@

.c.o:
	@echo " ==> Compiling $<"
	${CC} -c ${COMPILE_FLAGS} -o $@ $<

${PROGS}: ${OBJS} ${LIBS}
	@echo " ==> Linking $@"
	${LD} -o ${@} ${${@}_SRCS:.c=.o} ${LINK_FLAGS}

${LIBS}: ${OBJS}
	@echo " ==> Archiving $@"
	${AR} -rc ${@} ${${@}_SRCS:.c=.o}

clean:
	@echo " ==> Cleaning"
	rm -f ${PROGS} ${LIBS} ${OBJS} ${DEPS} config.mk

PREFIX	?=	/usr/local
DESTDIR	?=	/
BINDIR	?=	${DESTDIR}/${PREFIX}/bin
LIBDIR	?=	${DESTDIR}/${PREFIX}/lib
MANDIR	?=	${DESTDIR}/${PREFIX}/man

${PROGS:=-install}:
	@echo " ==> Installing program ${@:-install=}"
	install -d ${BINDIR}
	install ${@:-install=} ${BINDIR}

${LIBS:=-install}:
	@echo " ==> Installing library {@:-install=}"
	install -d ${LIBDIR}
	install ${@:-install=} ${LIBDIR}

${MAN:=-install}:
	@echo " ==> Installing man page ${@:-install=}"
	install -d ${MANDIR}/man`echo "${@:-install=}" | sed 's/.*\.\([1-9]\)$$/\1/g'`
	gzip -c ${@:-install=} > ${MANDIR}/man`echo "${@:-install=}" | sed 's/.*\.\([1-9]\)$$/\1/g'`/`basename ${@:-install=}`.gz

install: all ${PROGS:=-install} ${LIBS:=-install} ${MAN:=-install}
	@echo " ==> Installation finished"

snmk-libdeps:
	@echo ${LIBADD}

snmk-update:
	@echo " ==> Updating to the most recent version of SN Makefiles"
	curl -L -O --url 'https://gitlab.com/herrhotzenplotz/makefile-template/-/raw/trunk/default.mk'
	curl -L -O --url 'https://gitlab.com/herrhotzenplotz/makefile-template/-/raw/trunk/autodetect.sh'
	@echo " ==> Updated. Rebuilding everything..."
	${MAKE} clean
	${MAKE} all
