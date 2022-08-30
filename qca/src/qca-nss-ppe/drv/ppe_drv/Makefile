# Makefile for PPE driver

ccflags-y := -I$(obj)/../exports -I$(obj)/..
ccflags-y += -DPPE_DRV_BUILD_ID="$(BUILD_ID)"

obj-m += qca-nss-ppe.o
qca-nss-ppe-objs := ppe_drv.o
qca-nss-ppe-objs += ppe_drv_br.o
qca-nss-ppe-objs += ppe_drv_dp.o
qca-nss-ppe-objs += ppe_drv_exception.o
qca-nss-ppe-objs += ppe_drv_flow.o
qca-nss-ppe-objs += ppe_drv_host.o
qca-nss-ppe-objs += ppe_drv_iface.o
qca-nss-ppe-objs += ppe_drv_lag.o
qca-nss-ppe-objs += ppe_drv_l3_if.o
qca-nss-ppe-objs += ppe_drv_nexthop.o
qca-nss-ppe-objs += ppe_drv_port.o
qca-nss-ppe-objs += ppe_drv_pppoe.o
qca-nss-ppe-objs += ppe_drv_pppoe_session.o
qca-nss-ppe-objs += ppe_drv_pub_ip.o
qca-nss-ppe-objs += ppe_drv_sc.o
qca-nss-ppe-objs += ppe_drv_vlan.o
qca-nss-ppe-objs += ppe_drv_vp.o
qca-nss-ppe-objs += ppe_drv_vsi.o
qca-nss-ppe-objs += ppe_drv_v4.o
qca-nss-ppe-objs += ppe_drv_v6.o

ccflags-y += -Wall -Werror
ccflags-y += -DPPE_DRV_DEBUG_LEVEL=3
