# $Id: Makefile.in,v 1.63 2015/08/05 23:15:41 tom Exp $
##############################################################################
# Copyright (c) 1998-2014,2015 Free Software Foundation, Inc.                #
#                                                                            #
# Permission is hereby granted, free of charge, to any person obtaining a    #
# copy of this software and associated documentation files (the "Software"), #
# to deal in the Software without restriction, including without limitation  #
# the rights to use, copy, modify, merge, publish, distribute, distribute    #
# with modifications, sublicense, and/or sell copies of the Software, and to #
# permit persons to whom the Software is furnished to do so, subject to the  #
# following conditions:                                                      #
#                                                                            #
# The above copyright notice and this permission notice shall be included in #
# all copies or substantial portions of the Software.                        #
#                                                                            #
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR #
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   #
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    #
# THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      #
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    #
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        #
# DEALINGS IN THE SOFTWARE.                                                  #
#                                                                            #
# Except as contained in this notice, the name(s) of the above copyright     #
# holders shall not be used in advertising or otherwise to promote the sale, #
# use or other dealings in this Software without prior written               #
# authorization.                                                             #
##############################################################################
#
# Author: Thomas E. Dickey 1996-on
#
# Makefile for ncurses miscellany directory
#
# This makes/installs the terminfo database
#
# The variable 'srcdir' refers to the source-distribution, and can be set with
# the configure script by "--srcdir=DIR".
#
# The rules are organized to produce the libraries for the configured models,
# and the programs with the configured default model.

# turn off _all_ suffix rules; we'll generate our own
.SUFFIXES :

SHELL		= /bin/sh

THIS		= Makefile

CF_MFLAGS 	= 


DESTDIR		= 
top_srcdir	= ..
srcdir		= .
prefix		= /usr
exec_prefix	= ${prefix}
bindir		= ${exec_prefix}/bin
libdir		= ${exec_prefix}/lib
datarootdir	= ${prefix}/share
datadir		= ${datarootdir}
includesubdir	= 

INCLUDEDIR	= $(DESTDIR)$(includedir)$(includesubdir)

tabsetdir	= $(datadir)/tabset
ticdir		= /usr/share/terminfo
ticfile		= $(ticdir).db
source		= ${top_srcdir}/misc/terminfo.src

INSTALL		= /usr/bin/install -c
INSTALL_PROGRAM	= ${INSTALL}
INSTALL_SCRIPT	= ${INSTALL}
INSTALL_DATA	= ${INSTALL} -m 644

PKG_CONFIG_LIBDIR = /usr/lib/i386-linux-gnu/pkgconfig

################################################################################
all :	terminfo.tmp

depend :

sources :: terminfo.tmp 

install : install.data install.libs

install.data :	terminfo.tmp \
		$(DESTDIR)$(libdir) \
		$(DESTDIR)$(datadir) \
		$(DESTDIR)$(tabsetdir)
	DESTDIR=${DESTDIR} \
	prefix=${prefix} \
	exec_prefix=${exec_prefix} \
	bindir=${bindir} \
	top_srcdir=${top_srcdir} \
	srcdir=${srcdir} \
	datadir=${datadir} \
	ticdir=${ticdir} \
	source=terminfo.tmp \
	cross_compiling=yes \
	$(SHELL) ./run_tic.sh
	@cd $(srcdir)/tabset && \
		$(SHELL) -c 'for i in * ; do \
			if test -f $$i ; then \
			echo installing $$i; \
			$(INSTALL_DATA) $$i $(DESTDIR)$(tabsetdir)/$$i; \
			fi; done'

NCURSES_CONFIG	= ncurses6-config

install.libs :: $(DESTDIR)$(bindir) ncurses-config
	$(INSTALL_SCRIPT) ncurses-config $(DESTDIR)$(bindir)/$(NCURSES_CONFIG)

#install.libs :: pc-files $(DESTDIR)$(PKG_CONFIG_LIBDIR)
#	$(SHELL) -c 'for name in *.pc; do $(INSTALL_DATA) $$name $(DESTDIR)$(PKG_CONFIG_LIBDIR)/$$name; done'

#sources :: pc-files
#pc-files :
#	$(SHELL) ./gen-pkgconfig
#	touch $@

terminfo.tmp : run_tic.sed $(source)
	echo '** adjusting tabset paths'
	sed -f run_tic.sed $(source) >terminfo.tmp

run_tic.sed :
	WHICH_XTERM=xterm-new \
	XTERM_KBS=BS \
	datadir=${datadir} \
	$(SHELL) $(srcdir)/gen_edit.sh >$@

$(DESTDIR)$(bindir) \
$(DESTDIR)$(libdir) \
$(DESTDIR)$(datadir) \
$(DESTDIR)$(tabsetdir) \
$(DESTDIR)$(PKG_CONFIG_LIBDIR) :
	mkdir -p $@

uninstall : uninstall.data uninstall.libs

uninstall.data :
	-test -d $(DESTDIR)$(tabsetdir) && rm -rf $(DESTDIR)$(tabsetdir)
	-test -d $(DESTDIR)$(ticdir) && rm -rf $(DESTDIR)$(ticdir)
	-test -f $(DESTDIR)$(ticfile) && rm -f $(DESTDIR)$(ticfile)

uninstall.libs :
	-rm -f $(DESTDIR)$(bindir)/$(NCURSES_CONFIG)
#	$(SHELL) -c 'for name in *.pc; do rm -f $(DESTDIR)$(PKG_CONFIG_LIBDIR)/$$name; done'

tags :

TAGS :

mostlyclean :
	-rm -f terminfo.tmp
	-rm -f run_tic.sed
#	-rm -f *.pc pc-files
	-rm -f core tags TAGS *~ *.bak *.ln *.atac trace

clean :: mostlyclean

cleandir :: clean

distclean : clean
#	-rm -f gen-pkgconfig
	-rm -f Makefile run_tic.sh ncurses-config

realclean : distclean

###############################################################################
# The remainder of this file is automatically generated during configuration
###############################################################################
