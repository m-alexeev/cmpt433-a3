#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/kfifo.h>


#define MY_DEVICE_FILE "morsecode"
#define FIFO_SIZE 512 

static DECLARE_KFIFO(morsecode_fifo, char, FIFO_SIZE);


static ssize_t morse_read(struct file *file , char* buff, size_t count, loff_t* ppos){
    int bytesRead; 
    bytesRead = 0;
    if (kfifo_to_user(&morsecode_fifo,buff, count, &bytesRead)){
        return -EFAULT;
    }
    // TODO: Check what to do with read data

    return bytesRead;

}

static ssize_t morse_write(struct file *file, const char* buff, size_t count, loff_t *ppos){
    int i; 
    int copied; 

    //Write data to fifo
    for (i = 0; i < count; i ++){
        if (kfifo_from_user(&morsecode_fifo, &buff[i], sizeof(buff[i]), &copied)){
            return -EFAULT;
        }
        msleep(100);
    }
    return count; 
}


struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .read = morse_read,
    .write = morse_write,
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
    
    INIT_KFIFO(morsecode_fifo);

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