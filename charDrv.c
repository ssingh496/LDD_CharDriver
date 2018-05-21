
/*Char driver to do the basic file operations
 * @author		Sandeep Pratap Singh
 * @date		21 May 2018
 **/
 
 
#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <linux/uaccess.h>          // Required for the copy to user function
#define  DEVICE_NAME "charDrvOps"    ///< The device will appear at /dev/ebbchar using this value
#define  CLASS_NAME  "charDrv"        ///< The device class -- this is a character device driver


MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Sandeep Singh");    ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("A simple Linux char driver doing basic user read/write operations");  ///< The description -- see modinfo
MODULE_VERSION("0.1");            ///< A version number to inform users

/*Global variables*/

static int    majorNumber;                  ///< Stores the device number -- determined automatically
static char   message[256] = {0};           ///< Memory for the string that is passed from userspace
static short  size_of_message;              ///< Used to remember the size of the string stored
static int    numberOpens = 0;              ///< Counts the number of times the device is opened
static struct class*  charDrvClass  = NULL; ///< The device-driver class struct pointer
static struct device* charDevice = NULL; ///< The device-driver device struct pointer


// The prototype functions for the character driver -- must come before the struct definition
static int     charDrv_open(struct inode *, struct file *);
static int     charDrv_release(struct inode *, struct file *);
static ssize_t charDrv_read(struct file *, char *, size_t, loff_t *);
static ssize_t charDrv_write(struct file *, const char *, size_t, loff_t *);


/** @brief Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations fops =
{
   .open  =  charDrv_open,
   .read  =  charDrv_read,
   .write =  charDrv_write,
   .release = charDrv_release,
};

/** @brief The LKM initialization function*/
static int __init charDrv_init(void)
{
   printk(KERN_INFO "charDrvOps_init: Initializing the  LKM\n");

   // try to allocate the dynamic major number
   
   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber<0)
   {
      printk(KERN_ALERT "charDrvOps failed to register a major number\n");
      return majorNumber;
   }
   printk(KERN_INFO "charDrvOps: registered correctly with major number %d\n", majorNumber);
   
   // Register the device class
   charDrvClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(charDrvClass))
   {  // Check for error and clean up if there is
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(charDrvClass);          // Correct way to return an error on a pointer
   }
   printk(KERN_INFO "charDrv: device class registered correctly\n");
   
      // Register the device driver
   charDevice = device_create(charDrvClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(charDevice)){               // Clean up if there is an error
      class_destroy(charDrvClass);           // Repeated code but the alternative is goto statements
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(charDevice);
   }
   printk(KERN_INFO "charDrvClass: device class created correctly\n"); // Made it! device was initialized
   return 0;
 
}

/** @brief The LKM cleanup function
 */
static void __exit charDrv_exit(void)
{
   device_destroy(charDrvClass, MKDEV(majorNumber, 0));     // remove the device
   class_unregister(charDrvClass);                          // unregister the device class
   class_destroy(charDrvClass);                             // remove the device class
   unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
   printk(KERN_INFO "charDrv: Goodbye from the LKM!\n");
}   

/** @brief The device open function that is called each time the device is opened*/
static int charDrv_open(struct inode *inodep, struct file *filep)
{
   numberOpens++;
   printk(KERN_INFO "charDrv: Device has been opened %d time(s)\n", numberOpens);
   return 0;
 
}
/** @brief This function is called whenever device is being read from user space*/
static ssize_t charDrv_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
   int error_count = 0;
   // copy_to_user has the format ( * to, *from, size) and returns 0 on success
   error_count = copy_to_user(buffer, message, size_of_message);

   if (error_count==0){            // if true then have success
      printk(KERN_INFO "charDrv: Sent %d characters to the user\n", size_of_message);
      return (size_of_message=0);  // clear the position to the start and return 0
   }
   else {
      printk(KERN_INFO "charDrv: Failed to send %d characters to the user\n", error_count);
      return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
   }
}
/** @brief This function is called whenever the device is being written to from user space*/
static ssize_t charDrv_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
   sprintf(message, "%s(%zu letters)", buffer, len);   // appending received string with its length
   size_of_message = strlen(message);                 // store the length of the stored message
   printk(KERN_INFO "charDrv: Received %zu characters from the user\n", len);
   return len;
}

/** @brief The device release function that is called whenever the device is closed/released by user func
*/
static int charDrv_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "charDrv: Device successfully closed\n");
   return 0;
}

/** @brief A module must use the module_init() module_exit() macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function (as
 *  listed above)
 */
module_init(charDrv_init);
module_exit(charDrv_exit);
