ccflags-y += -DDEBUG

ifeq ($(BSP_BOARD_UNISOC_WCN_SOCKET),sipc)
	WCN_BT_BSP_PATH := $(shell cd $(BSP_ROOT_DIR)/modules/wcn/bluetooth/driver/tty-sipc; pwd)
else ifeq ($(BSP_BOARD_UNISOC_WCN_SOCKET),sdio)
	WCN_BT_BSP_PATH := $(shell cd $(BSP_ROOT_DIR)/modules/wcn/bluetooth/driver/tty-sdio; pwd)
esle
	$(error "no BSP_BOARD_UNISOC_WCN_SOCKET, pls fix it !!!")
endif

WCN_BT_LOCAL_SRC := $(wildcard $(WCN_BT_BSP_PATH)/*.c)
WCN_BT_LOCAL_ALI_SRC := $(wildcard $(WCN_BT_BSP_PATH)/alignment/*.c)

WCN_BT_BSP_SRC := $(wildcard $(WCN_BT_LOCAL_SRC))
WCN_BT_BSP_DIR := $(notdir $(WCN_BT_BSP_SRC))
WCN_BT_BSP_ALI_SRC := $(wildcard $(WCN_BT_LOCAL_ALI_SRC))
WCN_BT_BSP_ALI_DIR := $(notdir $(WCN_BT_BSP_ALI_SRC))
ifeq ($(BSP_BOARD_UNISOC_WCN_SOCKET),sipc)
	WCN_BT_BSP_OBJ := $(patsubst %.c, tty-sipc/%.o, $(WCN_BT_BSP_DIR))
	WCN_BT_BSP_ALI_OBJ := $(patsubst %.c, tty-sipc/alignment/%.o, $(WCN_BT_BSP_ALI_DIR))
else ifeq ($(BSP_BOARD_UNISOC_WCN_SOCKET),sdio)
	WCN_BT_BSP_OBJ := $(patsubst %.c, tty-sdio/%.o, $(WCN_BT_BSP_DIR))
	WCN_BT_BSP_ALI_OBJ := $(patsubst %.c, tty-sdio/alignment/%.o, $(WCN_BT_BSP_ALI_DIR))
else
	$(error "no BSP_BOARD_UNISOC_WCN_SOCKET, pls fix it !!!")
endif

sprdbt_tty-objs += $(WCN_BT_BSP_OBJ) \
					$(WCN_BT_BSP_ALI_OBJ)

obj-m := sprdbt_tty.o

.PHONY:modules

modules:
	make $(BSP_MAKE_EXTRA_ARGS) ARCH=$(BSP_BOARD_ARCH) CROSS_COMPILE=$(BSP_KERNEL_CROSS_COMPILE) -C $(BSP_KERNEL_PATH) O=$(BSP_KERNEL_OUT) M=`pwd` $@
	cp -fv ./sprdbt_tty.ko $(BSP_MODULES_DIST)/sprdbt_tty.ko
	make modules_clean_bt

modules_install:
	make $(BSP_MAKE_EXTRA_ARGS) ARCH=$(BSP_BOARD_ARCH) CROSS_COMPILE=$(BSP_KERNEL_CROSS_COMPILE) -C $(BSP_KERNEL_PATH) O=$(BSP_KERNEL_OUT) M=`pwd` INSTALL_MOD_PATH=$(BSP_MODULES_OUT) $@

modules_clean_bt:
	rm $(WCN_BT_BSP_PATH)/*.o
	rm $(WCN_BT_BSP_PATH)/.*.cmd
	rm $(WCN_BT_BSP_PATH)/alignment/*.o
	rm $(WCN_BT_BSP_PATH)/alignment/.*.cmd
	rm sprdbt_tty.mod.*
	rm *.o
	rm ./.*.cmd
	rm Module.symvers
	rm modules.order
	rm -rf .tmp_versions

clean:
	make -C $(BSP_KERNEL_PATH) O=$(BSP_KERNEL_OUT) M=`pwd` $@
