#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/uaccess.h>
//schead

MODULE_LICENSE("GPL");

//! \file silenar.c
//! \brief

//! \def NAME
//! \brif The name of this kernel module
#define NAME "silenar"
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
//!
//! The ## before __VA_ARGS_ is a GNU C pre-processore extensions and is needed
//! in case the macro is called with no variadic arguments.
#define DEBUG_ALERT(msg,...) \
if (debug) \
  printk(KERN_ALERT "%s: %s -" msg ,HERE, ##__VA_ARGS__)

#define EVENTS_SIZE sizeof (struct event)

#define SIZE 10000
#define WRITE_BUF 32

#define GPIO_RUN 23
#define GPIO_ENB 24
#define GPIO_RDY 26
#define GPIO_ACK 27


//! \brief struct event defines the data for each SILENA ADC event
//!
struct event {
  int32_t tv_sec;   ///< Event timestamp, seconds field
  int32_t tv_usec;  ///< Event timestamp, microseconds field
  uint32_t value;   ///< Event ADC value
};

//! \brief struct driver_data defines the private data of the char device
//! of the SILENA DAQ
//!
struct driver_data {
  int rdyirq;   ///<
  int fHang;    ///<
  int fDone;    ///< Letture e' stata fata
  atomic_t write_idx;         ///< Write index of circular data buffer
  atomic_t read_idx;          ///< Read index of cicular data buffer
  struct event events[SIZE];  ///< Circular data buffer
};

static dev_t device = 0;

static int cdev_flag = 0;	///< Indicates whether \see cdev has been initialized
static struct cdev cdev;	
static struct class * dev_class = NULL; 
static struct device * dev_device = NULL;



static struct gpio gpios [] = {
  // GPIO table
  // GPIO, DIRECTION, NAME
  { 4,  GPIOF_IN, "D00" }, // ADC parallel data bus pins 0 ->
  { 5,  GPIOF_IN, "D01" },
  { 6,  GPIOF_IN, "D02" },
  { 7,  GPIOF_IN, "D03" },
  { 8,  GPIOF_IN, "D04" },
  { 9,  GPIOF_IN, "D05" },
  { 10, GPIOF_IN, "D06" },
  { 11, GPIOF_IN, "D07" },
  { 12, GPIOF_IN, "D08" },
  { 13, GPIOF_IN, "D09" },
  { 16, GPIOF_IN, "D10" },
  { 18, GPIOF_IN, "D11" },
  { 19, GPIOF_IN, "D12" }, // <- to 12
  { 23, GPIOF_OUT_INIT_LOW, "RUN" }, // Raspberry PI DAQ active
  { 24, GPIOF_OUT_INIT_LOW, "ENB" },
  { 25, GPIOF_IN, "LVE" },           // Dead time of silena
  { 26, GPIOF_IN, "RDY" },           // Silena data ready
  { 27, GPIOF_OUT_INIT_LOW, "ACK" }  // Raspberry PI data received
};

static int debug = 0;   ///< Boolean variable, enables debug messages

// This macro creates a file in /sys/module/<name>/parameters/<par> that
// controls the \see debug variable of type int with permisions 0644
module_param(debug, int, S_IRUGO | S_IWUSR);

DECLARE_WAIT_QUEUE_HEAD(queue);


//! \brief Exit function of the kernel module
//!
static void silenar_exit(void) {
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
//!
//! \param ddata is a pointer to the char device private data
int read_event(struct driver_data * ddata) {
  struct timespec64 tim;   // Timestamp of the event
  int write_idx, val, j;
  
  ktime_get_real_ts64(&tim);	// New method for gettimeofday

  //do_gettimeofday(&tim); // Wrapper for gettimeofday in kernel space

  write_idx = atomic_read(&(ddata->write_idx));
  
  if ( ((write_idx + 1) % SIZE) == atomic_read(&(ddata->read_idx)) ) {
    //DEBUG_ALERT("Strange things are happening");
    ddata->fHang = 1;
    return -1;
  }
  
  val = 0;
  for (j = 12; j >= 0; --j) {
    val = (val << 1) | gpio_get_value(gpios[j].gpio);
  }
  val ^= 0x1FFF;
  
  gpio_set_value(GPIO_ACK, 1);
  ddata->events[write_idx].tv_sec   = (int32_t)(tim.tv_sec);
  ddata->events[write_idx].tv_usec  = (int32_t)(tim.tv_nsec / 1000);
  ddata->events[write_idx].value    = val;
  write_idx = (write_idx + 1) % SIZE;
  atomic_set(&(ddata->write_idx), write_idx);
  
  gpio_set_value(GPIO_ACK, 0);
  return 0;
}

//! \brief Callback function of the Silena RDY interrupt
//!
//! \param irq is the code of the IRQ line
//! \param arg is a pointer to the char device private data
irqreturn_t irq_service (int irq, void * arg) {
  struct driver_data * ddata = arg;
  
  if (irq != ddata->rdyirq) {
    DEBUG_ALERT("Interrupt received is not RDY IRQ");
    return IRQ_HANDLED;
  }
  if (gpio_get_value(GPIO_RDY) == 1) {
    return IRQ_HANDLED;
  }
  if (ddata->fHang) {
    ddata->fDone = 1;
    wake_up(&queue);
    return IRQ_HANDLED;
  }
  if (read_event(ddata)) {
    // In case the read failed, the queue is not awoken
    return IRQ_HANDLED;
  }
  
  ddata->fDone = 1;
  wake_up(&queue);
  return IRQ_HANDLED;
}


//! \brief
//! \param node
//! \param filep
static int open (struct inode * node, struct file * filep) {
//  MAJOR(node->i_rdev)
//  MINOR(node->i_rdev)
  static struct driver_data ddata;
  int status;
  
  DEBUG_ALERT("Opened the file.");
  
  filep->private_data = &ddata;
  atomic_set(&(ddata.write_idx), 0);
  atomic_set(&(ddata.read_idx), 0);
  
  // Setup GPIO hardware
  status = gpio_request_array(gpios, ARRAY_SIZE(gpios));
  if (status) {
    // Gestione dell'errore
    DEBUG_ALERT("Unable to request GPIOs");
    return status;
  }
  
  gpio_direction_input(GPIO_RDY);
  ddata.rdyirq = gpio_to_irq(GPIO_RDY); // Il identificatore del interrupt
  ddata.fHang = 0;
  ddata.fDone = 1;
  
  if ( request_threaded_irq (ddata.rdyirq, irq_service, NULL, 
                                IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
                                "silenar_irq", &ddata) ) {
    DEBUG_ALERT("Unable to request RDY interrupt");
    gpio_free_array(gpios, ARRAY_SIZE(gpios));
    return -1;
  }
  
  gpio_set_value(GPIO_RUN,1);
  gpio_set_value(GPIO_ENB,1);
  
  return 0;
}

//! \brief
//! \param node
//! \param filep
static int close (struct inode * node, struct file * filp) {
  struct driver_data * ddata = filp->private_data;
  DEBUG_ALERT("Closed the file.");
  
  gpio_set_value(GPIO_RUN,0);
  gpio_set_value(GPIO_ENB,0);
  
  
  // Unregister the RDY interrupt
  free_irq(ddata->rdyirq, ddata);
  
  // Release GPIOs
  gpio_free_array(gpios, ARRAY_SIZE(gpios));
  
  return 0;
}

//! \brief
//! \param filep
//! \param buff
//! \param count
//! \param ppos
static ssize_t read
(struct file * filp, char * buff, size_t count, loff_t *ppos) {
  struct driver_data * ddata = filp->private_data;
  struct event * events = ddata->events;
  int read_idx, write_idx, transfer, transfer_bytes, retval;
  
  int request = count / EVENTS_SIZE;
  if (request == 0) {
    DEBUG_ALERT("Read request rejected, consult the documentation.");
    return -1;
  }
  
  read_idx = atomic_read(&(ddata->read_idx));
  if ( read_idx == atomic_read(&(ddata->write_idx)) ) { // Buffer is empty
    ddata->fDone = 0;
    retval = wait_event_interruptible(queue, ddata->fDone);
    if (retval) { // Spurious wakeup of event queue
      DEBUG_ALERT("Spurious wakeup of event queue");
      return retval;
    }
  }
  write_idx = atomic_read(&(ddata->write_idx));
  transfer = (write_idx + SIZE - read_idx) % SIZE;
  
  // Should we not decrement  transfer by one here?
  if ( transfer > request) {
    transfer = request;
    // transfer = request - 1;
  }
  
  transfer_bytes = transfer * EVENTS_SIZE;
  
  if (read_idx + transfer <= SIZE) {
    // Bytes transfer doesn't wrap around the circular buffer, only one call
    // to copy_to_user is needed
    retval = copy_to_user(buff, events + read_idx, transfer_bytes);
    //read_idx += (transfer - (retval / EVENTS_SIZE));
    if (retval) {
      goto copy_error;
    }
  }
  else { // read_idx > write_idx
    // Bytes transfer wraps around the circular buffer, first transfer the bytes
    // up to the upper bound of the memory allocated to the buffer
    int second_transfer = transfer_bytes - (SIZE - read_idx) * EVENTS_SIZE;
    retval = copy_to_user(buff, events + read_idx, transfer_bytes - second_transfer);
//    read_idx += ((transfer_bytes - second_transfer - retval) / EVENTS_SIZE);
//    if (read_idx == SIZE) {
//      read_idx = 0;
//    }
    if (retval) {
      goto copy_error;
    }
    // Then copy the remaining bytes starting from the head of the buffer
    retval = copy_to_user(buff, events, second_transfer);
//    read_idx += ((second_transfer - retval) / EVENTS_SIZE);
    if (retval) {
      goto copy_error;
    }  
  }
  // The transfer was complete, update the local read_idx variable
  read_idx = (read_idx + transfer) % SIZE;
  // And the global variable
  atomic_set(&(ddata->read_idx), read_idx);
   
  if ( (ddata->fHang) && (gpio_get_value(GPIO_RDY) == 0) ) {
    read_event(ddata);
    ddata->fHang = 0;
  }
    
  return transfer_bytes;
  
  copy_error:
    DEBUG_ALERT("%d bytes couldn't be transferred to user", retval);
    return -1;
}

//! \brief
//! \param filep
//! \param user_buff
//! \param count
//! \param ppos
static ssize_t write 
(struct file * filp, char const * user_buff, size_t count, loff_t *ppos) {
  // Vogliamo leggere dati, non ci importa scrivere dati
  if (user_buff == NULL) {
    return count;
  }

  switch (user_buff[0]) {
  case 'S': // Start the acquisition
    gpio_set_value(GPIO_ENB, 1);
    gpio_set_value(GPIO_RUN, 1);
    DEBUG_ALERT("Starting the acquisition.");
    break;
  case 'E': // End the acquisition
    gpio_set_value(GPIO_RUN, 0);
    gpio_set_value(GPIO_ENB, 0);
    DEBUG_ALERT("Stopping the acquisition.");
    break;
  default:
    DEBUG_ALERT("Unrecognized write command.");
    break;
  }

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
static int silenar_init(void) {
  int status;
  
  
  // 1. Allocare le risorse per un char device
  status = alloc_chrdev_region (&device, BASE_MINOR, DEV_NUM, NAME);
  if (status < 0) {
    device = 0;
    goto failure;
  }
  
  // 2. Creazione della struttura cdev
  cdev_init (&cdev, &fops);
  cdev.owner = THIS_MODULE;
  
  // 3. Aggiunta al sistema di cdev
  status  = cdev_add (&cdev, device, DEV_NUM);
  if (status) {
    goto failure;
  }
  cdev_flag = 1;
  
  // 4. Creazione della classe di device (unica)
  dev_class = class_create(THIS_MODULE, NAME);
  if (IS_ERR(dev_class)) {
    status = (int) dev_class;
    dev_class = NULL;
    goto failure;
  }
  // 5. Creazione di FILE in /dev
  dev_device = device_create(dev_class, NULL, device, NULL, NAME);
  if (IS_ERR(dev_device)) {
    status = (int) dev_device;
    dev_device = NULL;
    goto failure;
  }
  
  DEBUG_ALERT("All's well that ends well.");
  return 0;
  
  failure:
    DEBUG_ALERT("These violent delights have violent ends");
    silenar_exit();
    return status;
}


module_init(silenar_init);
module_exit(silenar_exit);
