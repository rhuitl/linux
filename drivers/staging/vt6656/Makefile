# TODO: all of these should be removed
ccflags-y := -DLINUX -D__KERNEL__ -DEXPORT_SYMTAB -D__NO_VERSION__
ccflags-y += -DHOSTAP

vt6656_stage-y +=	main_usb.o \
			card.o  \
			mac.o   \
			baseband.o \
			wctl.o  \
			80211mgr.o \
			wcmd.o\
			wmgr.o \
			bssdb.o \
			wpa2.o \
			rxtx.o \
			dpc.o \
			power.o \
			datarate.o \
			mib.o \
			rc4.o \
			tether.o \
			tcrc.o \
			ioctl.o \
			hostap.o \
			wpa.o \
			key.o \
			tkip.o \
			michael.o \
			rf.o \
			iwctl.o \
			wpactl.o \
			aes_ccmp.o \
			usbpipe.o \
			channel.o \
			control.o \
			firmware.o \
			int.o

obj-$(CONFIG_VT6656) +=	vt6656_stage.o

#EXTRA_CFLAGS += -O0
#CFLAGS_REMOVE_80211mgr.o = -Os
#CFLAGS_REMOVE_aes_ccmp.o = -Os
#CFLAGS_REMOVE_baseband.o = -Os
#CFLAGS_REMOVE_bssdb.o = -Os
#CFLAGS_REMOVE_built-in.o = -Os
#CFLAGS_REMOVE_card.o = -Os
#CFLAGS_REMOVE_channel.o = -Os
#CFLAGS_REMOVE_control.o = -Os
#CFLAGS_REMOVE_datarate.o = -Os
#CFLAGS_REMOVE_dpc.o = -Os
#CFLAGS_REMOVE_firmware.o = -Os
#CFLAGS_REMOVE_hostap.o = -Os
#CFLAGS_REMOVE_int.o = -Os
#CFLAGS_REMOVE_ioctl.o = -Os
#CFLAGS_REMOVE_iwctl.o = -Os
#CFLAGS_REMOVE_key.o = -Os
#CFLAGS_REMOVE_mac.o = -Os
#CFLAGS_REMOVE_main_usb.o = -Os
#CFLAGS_REMOVE_mib.o = -Os
#CFLAGS_REMOVE_michael.o = -Os
#CFLAGS_REMOVE_power.o = -Os
#CFLAGS_REMOVE_rc4.o = -Os
#CFLAGS_REMOVE_rf.o = -Os
#CFLAGS_REMOVE_rxtx.o = -Os
#CFLAGS_REMOVE_tcrc.o = -Os
#CFLAGS_REMOVE_tether.o = -Os
#CFLAGS_REMOVE_tkip.o = -Os
#CFLAGS_REMOVE_usbpipe.o = -Os
#CFLAGS_REMOVE_vt6656_stage.o = -Os
#CFLAGS_REMOVE_wcmd.o = -Os
#CFLAGS_REMOVE_wctl.o = -Os
#CFLAGS_REMOVE_wmgr.o = -Os
#CFLAGS_REMOVE_wpa2.o = -Os
#CFLAGS_REMOVE_wpactl.o = -Os
#CFLAGS_REMOVE_wpa.o = -Os
