#
# Makefile for the linux kernel.
#

# Object file lists.

obj-y				:= irq.o time.o gpio.o clock.o dev.o cpu.o
obj-y				+= cache.o mfp.o
# NUC700 CPU support files

obj-$(CONFIG_CPU_NUC710)	+= nuc710.o
obj-$(CONFIG_CPU_NUC740)	+= nuc740.o
obj-$(CONFIG_CPU_NUC745)	+= nuc745.o

# machine support

obj-$(CONFIG_MACH_NUC710EVB)	+= mach-nuc710evb.o
obj-$(CONFIG_MACH_NUC740EVB)	+= mach-nuc740evb.o
obj-$(CONFIG_MACH_NUC745EVB)	+= mach-nuc745evb.o
