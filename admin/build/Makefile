BOARD_LIST      = WIFIRE SDZL MEBII HMZ144 Whitecat

SKEL_SCRATCH    = skeleton/linux32/pic32prog \
                  skeleton/linux64/pic32prog \
                  skeleton/macosx/pic32prog \
                  skeleton/pic32prog.exe \
                  skeleton/sdcard.img \
                  skeleton/vmunix.dis \
                  skeleton/vmunix.hex

PIC32PROG       = https://raw.githubusercontent.com/sergev/pic32prog/master

DATE            := $(shell date +'%Y-%m-%d')

#
# Use this GCC toochain.
#
MIPS_GCC_ROOT   ?= /usr/local/mips-2014.05

ENV             = BSDSRC=$(abspath $(PWD)/../..)\
                  MAKESYSPATH=$(abspath $(PWD)/../../share/mk-pic32)\
                  MIPS_GCC_ROOT=$(MIPS_GCC_ROOT)

all:            build sdcard-$(DATE).zip
		for b in $(BOARD_LIST); do \
                    t=`echo $$b | tr A-Z a-z`; \
		    $(MAKE) litebsd-$$t-$(DATE).zip BOARD=$$b.pic32 TARGET=$$t; \
                done

clean:
		rm -rf *.zip $(wildcard litebsd-*-????-??-??)

clean-all:
		rm -f $(SKEL_SCRATCH) *.log

build:          build-$(DATE).log

sdcard-$(DATE).zip: ../../sdcard.img
		zip -jq $@ ../../sdcard.img

litebsd-$(TARGET)-$(DATE).zip: update-skeleton
		@rm -rf litebsd-$(TARGET)-$(DATE) $@
		cp -rp skeleton litebsd-$(TARGET)-$(DATE)
		ln ../../sdcard.img litebsd-$(TARGET)-$(DATE)/sdcard.img
		ln ../../sys/compile/$(BOARD)/vmunix.hex litebsd-$(TARGET)-$(DATE)/vmunix.hex
		ln ../../sys/compile/$(BOARD)/vmunix.dis litebsd-$(TARGET)-$(DATE)/vmunix.dis
		zip -rq $@ litebsd-$(TARGET)-$(DATE)

build-$(DATE).log:
		echo === LiteBSD Revision $(shell git rev-list HEAD --count) === > build-$(DATE).log
		bmake -C../.. build $(ENV) >> build-$(DATE).log 2>&1
		bmake -C../.. fs $(ENV) >> build-$(DATE).log 2>&1
		bmake -C../../sys/compile all $(ENV) >> build-$(DATE).log 2>&1

update-skeleton: skeleton/linux32/pic32prog skeleton/linux64/pic32prog \
                 skeleton/macosx/pic32prog skeleton/pic32prog.exe \

skeleton/linux32/pic32prog:
		(cd skeleton/linux32 && \
		curl -o pic32prog $(PIC32PROG)/linux32/pic32prog)

skeleton/linux64/pic32prog:
		(cd skeleton/linux64 && \
		curl -o pic32prog $(PIC32PROG)/linux64/pic32prog)

skeleton/macosx/pic32prog:
		(cd skeleton/macosx && \
		curl -o pic32prog $(PIC32PROG)/macosx/pic32prog)

skeleton/pic32prog.exe:
		(cd skeleton && \
		curl -o pic32prog.exe $(PIC32PROG)/pic32prog.exe)
