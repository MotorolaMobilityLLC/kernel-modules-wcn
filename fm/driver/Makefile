ccflags-y += -DDEBUG

ifeq ($(BSP_BOARD_UNISOC_WCN_SOCKET),sipc)
WCN_FM_BSP_PATH ?= $(shell cd $(BSP_ROOT_DIR)/modules/wcn/fm/driver/fm_sipc; pwd)
endif

WCN_FM_LOCAL_SRC := $(wildcard $(WCN_FM_BSP_PATH)/*.c)

WCN_FM_BSP_SRC := $(wildcard $(WCN_FM_LOCAL_SRC))
WCN_FM_BSP_DIR := $(notdir $(WCN_FM_BSP_SRC))
ifeq ($(BSP_BOARD_UNISOC_WCN_SOCKET),sipc)
WCN_FM_BSP_OBJ := $(patsubst %.c, fm_sipc/%.o, $(WCN_FM_BSP_DIR))
endif

sprd_fm-objs += $(WCN_FM_BSP_OBJ)


obj-m := sprd_fm.o

.PHONY:modules

modules:
	make $(BSP_MAKE_EXTRA_ARGS) ARCH=$(BSP_BOARD_ARCH) CROSS_COMPILE=$(BSP_KERNEL_CROSS_COMPILE) -C $(BSP_KERNEL_PATH) O=$(BSP_KERNEL_OUT) M=`pwd` $@
	cp -fv ./sprd_fm.ko $(BSP_MODULES_DIST)/sprd_fm.ko
	make modules_clean_fm

modules_install:
	make $(BSP_MAKE_EXTRA_ARGS) ARCH=$(BSP_BOARD_ARCH) CROSS_COMPILE=$(BSP_KERNEL_CROSS_COMPILE) -C $(BSP_KERNEL_PATH) O=$(BSP_KERNEL_OUT) M=`pwd` INSTALL_MOD_PATH=$(BSP_MODULES_OUT) $@

modules_clean_fm:
	rm $(WCN_FM_BSP_PATH)/*.o
	rm $(WCN_FM_BSP_PATH)/.*.cmd
	rm sprd_fm.mod.*
	rm *.o
	rm ./.*.cmd
	rm Module.symvers
	rm modules.order
	rm -rf .tmp_versions

clean:
	make -C $(BSP_KERNEL_PATH) O=$(BSP_KERNEL_OUT) M=`pwd` $@
