obj-m :=

ifeq ($(BUILD_VARIANT),uart1)
rtk_uart1-objs := rtk_u1.o rtk_uart_common.o
obj-m += rtk_uart1.o
endif
ifeq ($(BUILD_VARIANT),uart2)
rtk_uart2-objs := rtk_u2.o rtk_uart_common.o
obj-m += rtk_uart2.o
endif
