LOCAL_PATH := $(shell cd $(BSP_ROOT_DIR)/modules/wcn/wlan; pwd)
$(warning shell echo "$(BSP_BOARD_WLAN_DEVICE)")

ifeq ($(strip $(BSP_BOARD_WLAN_DEVICE)),sc2332)
ccflags-y += -DDEBUG

else ifeq ($(strip $(BSP_BOARD_WLAN_DEVICE)),sc2355)
ccflags-y += -DSC2355_FTR -DSC2355_TX_FILTER -DSC2355_QOS_MAP
ccflags-y += -DIBSS_SUPPORT -DIBSS_RSN_SUPPORT
ccflags-y += -DNAN_SUPPORT
ccflags-y += -DRTT_SUPPORT
ccflags-y += -DSC2355_RX_NO_LOOP
ccflags-y += -DACS_SUPPORT -DSC2355_HW_CSUM
ccflags-y += -DWMMAC_WFA_CERTIFICATION
ccflags-y += -DCOMPAT_SAMPILE_CODE
ccflags-y += -DRND_MAC_SUPPORT
ccflags-y += -DSOFTAP_HOOK
ccflags-y += -DATCMD_ASSERT
ccflags-y += -DTCPACK_DELAY_SUPPORT
ifeq ($(strip $(BSP_BUILD_VARIANT)),userdebug)
ccflags-y += -DCONFIG_SPRD_WCN_DEBUG
endif
ccflags-y += -DSPLIT_STACK

endif

SPRDWL_SRC := $(shell find $(LOCAL_PATH)/$(BSP_BOARD_WLAN_DEVICE) -name "*.c")
SPRDWL_DIR :=  $(SPRDWL_SRC:$(LOCAL_PATH)/%=%)
sprdwl_ng-objs += $(SPRDWL_DIR:%.c=%.o)
obj-m += sprdwl_ng.o

.PHONY:modules

modules:
	make $(BSP_MAKE_EXTRA_ARGS) ARCH=$(BSP_BOARD_ARCH) CROSS_COMPILE=$(BSP_KERNEL_CROSS_COMPILE) -C $(BSP_KERNEL_PATH) O=$(BSP_KERNEL_OUT) M=`pwd` $@

modules_install:
	make $(BSP_MAKE_EXTRA_ARGS) ARCH=$(BSP_BOARD_ARCH) CROSS_COMPILE=$(BSP_KERNEL_CROSS_COMPILE) -C $(BSP_KERNEL_PATH) O=$(BSP_KERNEL_OUT) M=`pwd` INSTALL_MOD_PATH=$(BSP_MODULES_OUT) $@

clean:
	make -C $(BSP_KERNEL_PATH) O=$(BSP_KERNEL_OUT) M=`pwd` $@
