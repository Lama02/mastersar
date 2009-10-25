.PHONY: all 
#.SECONDARY:

ifneq ($(KERNELRELEASE),)

  obj-m := hideModule.o
  # nom du module (pas le .ko mais l'intermédiaire)

  # Si plusieurs sources, on liste les fichiers objets :
  #$(name)-objs := file1.o file2.o

else
  KERNELDIR ?= /lib/modules/$(shell uname -r)/build
  PWD := $(shell pwd)

all :
	$(info obj-m : $(obj-m))
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean
	rm -f *~
endif
