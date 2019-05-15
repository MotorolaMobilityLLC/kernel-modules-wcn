# validate lookup result
ccflags-y += -DDEBUG

obj-m := sprd_fm.o
sprd_fm-objs := fmdrv_main.o fmdrv_ops.o fmdrv_rds_parser.o

.PHONY:modules

modules:
	make ARCH=$(BSP_BOARD_ARCH) CROSS_COMPILE=$(BSP_MODULES_CROSS_COMPILE) -C $(BSP_KERNEL_PATH) O=$(BSP_KERNEL_OUT) M=`pwd` $@

modules_install:
	make ARCH=$(BSP_BOARD_ARCH) CROSS_COMPILE=$(BSP_MODULES_CROSS_COMPILE) -C $(BSP_KERNEL_PATH) O=$(BSP_KERNEL_OUT) M=`pwd` INSTALL_MOD_PATH=$(BSP_MODULES_OUT) $@

clean:
	make -C $(BSP_KERNEL_PATH) O=$(BSP_KERNEL_OUT) M=`pwd` $@

