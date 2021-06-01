LINUX=/lib/modules/`uname -r`/build

# Istruzione generica per compilare un modulo del kernel LINUX
# La variabile LINUX contiene la path alla directory con i headers
#  e tutti l'altre dipendeze necessarie per compilare un modulo per
#  la versione corrente del kernel LINUX
# Il parametro `M=$(PWD)` lo si passa a Make per informarle la
#  direttory dove stiamo compilando il nostro modulo LINUX
# La parola chiave `modules` indica che stiamo compilando un modulo LINUX:
all:
	$(MAKE) -C$(LINUX) M=$(PWD) modules

# Aggiorniamo una variabile di sistema LINUX con il nostro file oggetto
#  che abbiamo compilato per communicarle al kernel il nomme del nostro modulo.
# Attenzione! La variabile vienne aggiornata solo all'interno del contesto di
#  Make
obj-m += emptyr.o

.PHONY : clean

clean:
	rm *.o *.ko *.mod.c modules.order Module.symvers
# Nota bene! Questo file di Make e' standard e consigliato dagli sviluppatori
#  del kernel LINUX. In questa istanza, Make accede a tutti gli header del
#  kernel ma NON lo tocca. Si consiglia anche di avere un ambiene pulito
#  per ogni modulo, ovvero, di avere nella directory del nostro progetto di 
#  modulo kernel solo un file sorgente e questo corrent file Makefile.

