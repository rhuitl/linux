#
# Makefile for the linux kernel.
#

# Object file lists.

obj-y				:= irq.o time.o gpio.o clock.o dev.o cpu.o cache.o
# NUC700 CPU support files

obj-$(CONFIG_CPU_NUC710)	+= nuc710.o

# machine support

obj-$(CONFIG_MACH_NUC710EVB)	+= mach-nuc710evb.o
