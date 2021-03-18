#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/uaccess.h>

#define MY_DEVICE_FILE "morsecode"


static ssize_t my_read(struct file *file , char* buff, size_t count, loff_t* ppos){
    return 0;
}

static ssize_t my_write(struct file *file, const char* buff, size_t count, loff_t *ppos){
    return 0;
}


struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .read = my_read,
    .write = my_write,
};

static struct miscdevice morsecode ={
    .minor = MISC_DYNAMIC_MINOR,
    .name = MY_DEVICE_FILE, 
    .fops = &my_fops
};


static int __init morsecode_init(void)
{
    int ret; 
    printk(KERN_INFO "----> My morsecode driver init(): file /dev/%s.\n", MY_DEVICE_FILE);
    
    ret = misc_register(&morsecode);

    return ret;
}
static void __exit morsecode_exit(void)
{
    printk(KERN_INFO "<---- My morsecode driver exit().\n");
    misc_deregister(&morsecode);
}

// Link our init/exit functions into the kernel's code.
module_init(morsecode_init);
module_exit(morsecode_exit);

// Information about this module:
MODULE_AUTHOR("Mikhail Alexeev");
MODULE_DESCRIPTION("A simple test driver");
MODULE_LICENSE("GPL"); // Important to leave as GPL