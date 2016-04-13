#
# Build kernels for all listed boards:
#
BOARD_LIST      = WIFIRE.pic32\
                  SDZL.pic32\
                  MEBII.pic32\
                  HMZ144.pic32\
                  EMZ64.pic32\
                  Whitecat.pic32\
                  SNADPIC.pic32

all:            ../../usr.sbin/config/config
		for b in $(BOARD_LIST); do $(MAKE) kernel BOARD=$$b; done

clean:
		for b in $(BOARD_LIST); do $(MAKE) -C$$b clean; done

../../usr.sbin/config/config:
		$(MAKE) -C../../usr.sbin/config

kernel:         $(BOARD)/.depend
		$(MAKE) -C$(BOARD)

$(BOARD)/.depend: ../mips/conf/$(BOARD)
		(cd ../mips/conf; ../../../usr.sbin/config/config -g $(BOARD))
		$(MAKE) -C$(BOARD) depend clean
