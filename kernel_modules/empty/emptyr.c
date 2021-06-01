#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/module.h>


MODULE_LICENSE("GPL");

//! \file emptyr.c
//! \brief

//! \def NAME
//! \brif The name of this kernel module
#define NAME "emptyr"
//! \def DEV_NUM
//! \brief The number of minor devices requested to the kernel
#define DEV_NUM 1
//! \def BASE_MINOR
//! \brief The starting index for the device minor
#define BASE_MINOR 0
//! \def MOD_ENABLE_DEBUG
//! \brief Enables debug messages to the kernel log through printk()
#define MOD_ENABLE_DEBUG 1
//! \def HERE
//! \brief Substitutes the module name and the calling function as ARGUMENTS
//! to a call to printk. Do NOT use on its own
#define HERE NAME,(char *)__FUNCTION__
//! \def DEBUG_ALERT
//! \brief Prints a debug message with priority KERN_ALERT on the kernel log
#define DEBUG_ALERT(msg,...) \
if (debug) \
  printk(KERN_ALERT "%s: %s -" msg ,HERE,__VA_ARGS__)

#define WRITE_BUF 32

static dev_t device = 0;

static int cdev_flag = 0;	///< Indicates whether \see cdev has been initialized
static struct cdev cdev;	
static struct class * dev_class = NULL; 
static struct device * dev_device = NULL;

static int debug = 0;   ///< Boolean variable, enables debug messages

// This macro creates a file in /sys/module/<name>/parameters/<par> that
// controls the \see debug variable of type int with permisions 0644
module_param(debug, int, S_IRUGO | S_IWUSR);

//! \brief Exit function of the kernel module
//!
static void emptyr_exit(void) {
  DEBUG_ALERT("Exiting the module.");
   // 1. Rimuove FILE in /dev
   if (dev_device) {
     device_destroy (dev_class, device);
   }  
  // 2. Distruzione della classe di device
  if (dev_class) {
    class_destroy(dev_class);
  }
    // 3. Rimuove cdev dal sistema
  if (cdev_flag) {
    cdev_del(&cdev);
  }
  // 4. Deallocazione delle risorse del nostro char device
  if (device) {
    unregister_chrdev_region(device, DEV_NUM);  
  }
}


//! \brief
//! \param node
//! \param filep
static int open (struct inode * node, struct file * filep) {
//  MAJOR(node->i_rdev)
//  MINOR(node->i_rdev)
  DEBUG_ALERT("Opened the file.");
  
  return 0;
}

//! \brief
//! \param node
//! \param filep
static int close (struct inode * node, struct file * filep) {
  DEBUG_ALERT("Closed the file.");
  
  return 0;
}

//! \brief
//! \param filep
//! \param buff
//! \param count
//! \param ppos
static ssize_t read
(struct file * filp, char * buff, size_t count, loff_t *ppos) {
  int status, nc, send;
  char line [WRITE_BUF];
  
  DEBUG_ALERT("reading %u bytes", count);
  mdelay(500); // microsecond delay in kernel space
  
  nc = snprintf(line, WRITE_BUFF,"DATA\n");
  if (nc > WRITE_BUF) {
    nc = WRITE_BUF
  }  
  send = nc < count ? nc : count;
  status = copy_to_user(buff,line,send);
  
  return status ? -EFAULT : send;
}

//! \brief
//! \param filep
//! \param user_buff
//! \param count
//! \param ppos
static ssize_t write 
(struct file * filp, char const * user_buff, size_t count, loff_t *ppos) {
  // Vogliamo leggere dati, non ci importa scrivere dati
  DEBUG_ALERT("request to write %u bytes", count);
  return count;
}

//! \brief Specifies the callback functions for the device file operations
static struct file_operations fops = {
    .open   = open,
    .release = close,
    .read = read,
    .write = write
  };

//! \brief Entry function of the kernel module
//!
static int emptyr_init(void) {
  int status;
  
  // 1. Allocare le risorse per un char device
  status = alloc_chrdev_region (&device, BASE_MINOR, DEV_NUM, NAME);
  if (status < 0) {
    device = 0;
    goto failure;
  }
  DEBUG_ALERT("alloc_chrdev_region success.");
  
  // 2. Creazione della struttura cdev
  cdev_init (&cdev, &fops);
  cdev.owner = THIS_MODULE;
  DEBUG_ALERT("cdev_init success.");
  
  // 3. Aggiunta al sistema di cdev
  status  = cdev_add (&cdev, device, DEV_NUM);
  if (status) {
    goto failure;
  }
  cdev_flag = 1;
  DEBUG_ALERT("cdev_add success.");
  
  // 4. Creazione della classe di device (unica)
  dev_class = class_create(THIS_MODULE, NAME);
  if (IS_ERR(dev_class)) {
    status = (int) dev_class;
    dev_class = NULL;
    goto failure;
  }
  DEBUG_ALERT("class_create success.");
  
  // 5. Creazione di FILE in /dev
  dev_device = device_create(dev_class, NULL, device, NULL, NAME);
  if (IS_ERR(dev_device)) {
    status = (int) dev_device;
    dev_device = NULL;
    goto failure;
  }
  DEBUG_ALERT("device_create success.");
  
  DEBUG_ALERT("All's well that ends well.");
  return 0;
  
  failure:
    DEBUG_ALERT("These violent delights have violent ends");
    emptyr_exit();
    return status;
}


module_init(emptyr_init);
module_exit(emptyr_exit);
