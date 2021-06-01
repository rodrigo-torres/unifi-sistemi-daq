#include <linux/module.h>
// Un tempo era anche necessario fare #include <linux/kernel.h>

// Per motivi legale, e' sempre meglio specificare una licensa con la cui
//  distriburremo il nostro modulo
MODULE_LICENSE("GPL");

// Il kernel non ne sa nulla dei nommi delle funzione init e exit del
// nostro modulo. Occorre specificarle

//! \file kello.c
//! \brief A minimal kernel module that prints out a kernel message to the
//! corresponding system log using a call to the LINUX function printk
//!

// Questo e' un modulo molto semplice che fara' un print sullo schermo
// Ma ATTENZIONE non possiamo usare printf essendo questo un modulo per kernel
// Usiamo invece printk, che scrive in una serie di file di log

static int kellor_init(void) {
  // Sfruttiamo il metodo di concattenazione in C per specificare la priorita' 
  // del messaggio. Le priorita' sono tutte specificate da MACRO che iniziano 
  // tutti da KERN_, ci sono:
  // KERN_DEBUG  e' la meno prioritaria
  // KERN_INFO
  // KERN_NOTICE
  // KERN_WARNING
  // KERN_ERR
  // KERN_CRIT
  // KERN_ALERT
  // KERN_EMERG  e' la piu' importante

  // ATTENZIONE! Noi non useremmo mai KERN_EMERG, ci la teniamo per quando verra' l'apocalisse
  // Useremo sempre KERN_ALERT
  printk(KERN_ALERT "Hello world! Is there anyone out there? This is the kernel speaking.\n");
  return 0;
}

static void kellor_exit(void) {
  printk(KERN_ALERT "Seems there's no one out there... Aight then, I'm leaving.\n");
}

module_init(kellor_init);
module_exit(kellor_exit);
