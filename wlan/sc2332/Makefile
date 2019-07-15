ccflags-y += -DDEBUG

obj-m += sprdwl_ng.o
sprdwl_ng-objs += main.o
sprdwl_ng-objs += cfg80211.o
sprdwl_ng-objs += txrx.o
sprdwl_ng-objs += cmdevt.o
sprdwl_ng-objs += npi.o
sprdwl_ng-objs += wapi.o
sprdwl_ng-objs += msg.o
sprdwl_ng-objs += work.o
sprdwl_ng-objs += vendor.o
sprdwl_ng-objs += tcp_ack.o
sprdwl_ng-objs += qos.o
sprdwl_ng-objs += sipc.o
sprdwl_ng-objs += embedded.o

.PHONY:modules

modules:
	make $(BSP_MAKE_EXTRA_ARGS) ARCH=$(BSP_BOARD_ARCH) CROSS_COMPILE=$(BSP_KERNEL_CROSS_COMPILE) -C $(BSP_KERNEL_PATH) O=$(BSP_KERNEL_OUT) M=`pwd` $@

modules_install:
	make $(BSP_MAKE_EXTRA_ARGS) ARCH=$(BSP_BOARD_ARCH) CROSS_COMPILE=$(BSP_KERNEL_CROSS_COMPILE) -C $(BSP_KERNEL_PATH) O=$(BSP_KERNEL_OUT) M=`pwd` INSTALL_MOD_PATH=$(BSP_MODULES_OUT) $@

clean:
	make -C $(BSP_KERNEL_PATH) O=$(BSP_KERNEL_OUT) M=`pwd` $@
