#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/kfifo.h>
#include <linux/leds.h>


#define MY_DEVICE_FILE "morsecode"
#define FIFO_SIZE 512 
/*
static unsigned short morsecode_codes[] = {
		0xB800,	// A 1011 1
		0xEA80,	// B 1110 1010 1
		0xEBA0,	// C 1110 1011 101
		0xEA00,	// D 1110 101
		0x8000,	// E 1
		0xAE80,	// F 1010 1110 1
		0xEE80,	// G 1110 1110 1
		0xAA00,	// H 1010 101
		0xA000,	// I 101
		0xBBB8,	// J 1011 1011 1011 1
		0xEB80,	// K 1110 1011 1
		0xBA80,	// L 1011 1010 1
		0xEE00,	// M 1110 111
		0xE800,	// N 1110 1
		0xEEE0,	// O 1110 1110 111
		0xBBA0,	// P 1011 1011 101
		0xEEB8,	// Q 1110 1110 1011 1
		0xBA00,	// R 1011 101
		0xA800,	// S 1010 1
		0xE000,	// T 111
		0xAE00,	// U 1010 111
		0xAB80,	// V 1010 1011 1
		0xBB80,	// W 1011 1011 1
		0xEAE0,	// X 1110 1010 111
		0xEBB8,	// Y 1110 1011 1011 1
		0xEEA0	// Z 1110 1110 101
}; */

DEFINE_LED_TRIGGER(morsecode_led);
static DECLARE_KFIFO(morsecode_fifo, char, FIFO_SIZE);

#define LED_DOT_TIME 200


static void led_blink(void){
    //turn on
    led_trigger_event(morsecode_led, LED_FULL);
    msleep(LED_DOT_TIME);
    //turn off 
    led_trigger_event(morsecode_led, LED_OFF);
    msleep(LED_DOT_TIME*4);
}


static ssize_t morse_read(struct file *file , char* buff, size_t count, loff_t* ppos){
    int bytesRead; 
    bytesRead = 0;
    if (kfifo_to_user(&morsecode_fifo,buff, count, &bytesRead)){
        return -EFAULT;
    }

    return bytesRead;
}

static ssize_t morse_write(struct file *file, const char* buff, size_t count, loff_t *ppos){
    int i; 
    int copied;
    char value; 
    //Clear the buffer 

    //Write data to fifo
    for (i = 0; i < count; i ++){
        if (copy_from_user(&value, &buff[i], sizeof(buff[i]) )){
            return -EFAULT;
        }
        //If letter, blink 
        if ((value >= 'a' && value <= 'z') || (value >= 'A' && value <= "Z")){
            led_blink();
        }

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

static void morsecode_led_register(void){
    //setup trigger
    led_trigger_register_simple("morsecode", &morsecode_led);
}

static void morsecode_led_unregister(void){
    //Cleanup
    led_trigger_unregister_simple(morsecode_led);
}

static int __init morsecode_init(void)
{
    int ret; 
    printk(KERN_INFO "----> My morsecode driver init(): file /dev/%s.\n", MY_DEVICE_FILE);
    
    INIT_KFIFO(morsecode_fifo);
    morsecode_led_register();
    ret = misc_register(&morsecode);

    return ret;
}
static void __exit morsecode_exit(void)
{
    printk(KERN_INFO "<---- My morsecode driver exit().\n");
    morsecode_led_unregister();
    misc_deregister(&morsecode);
}

// Link our init/exit functions into the kernel's code.
module_init(morsecode_init);
module_exit(morsecode_exit);

// Information about this module:
MODULE_AUTHOR("Mikhail Alexeev");
MODULE_DESCRIPTION("A simple test driver");
MODULE_LICENSE("GPL"); // Important to leave as GPL