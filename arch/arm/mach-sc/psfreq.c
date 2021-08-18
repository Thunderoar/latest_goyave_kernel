#include <linux/init.h>
#include <linux/suspend.h>
#include <linux/kobject.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <mach/pm_debug.h>
#include <mach/common.h>
#include <mach/hardware.h>
#include <mach/sci.h>
#include <mach/psfreq.h>
#include <linux/slab.h>

#define PSFREQ_DEV 11

enum {
	PS_FREQ_CMD,
	PS_CMD_MAX
};

struct handler_info {
	struct list_head link;
	int (*notify)(unsigned int, unsigned int);
};

struct freqconv_cmd {
	int cmd_type;
	unsigned int length;
	unsigned int freq;
	unsigned int suspended;
};

static DEFINE_MUTEX(freqconv_mutex_lock);
static LIST_HEAD(freqconv_handlers);

extern int ex_open_k (unsigned int minor);

extern int ex_release_k (unsigned int minor);

extern ssize_t ex_read_k (unsigned int minor, char * buf, size_t count);

extern ssize_t ex_write_k (unsigned int minor, const char * buf,size_t count);

void register_freqconv_change(int (*handler)(unsigned int, unsigned int))
{
	//struct list_head *l;
	struct handler_info *h;
	printk("Enter register_freqconv_change\n");
	list_for_each_entry(h, &freqconv_handlers, link) {
		if(h->notify == handler) {
			printk(KERN_ERR "register_freqconv_change handler duplicated.\n");
			return;
		}
	}
	h = kmalloc(sizeof(struct handler_info), GFP_KERNEL);
	if (!h) {
		printk(KERN_ERR "register fail\n");
		return;
	}
	INIT_LIST_HEAD(&h->link);
	h->notify = handler;
	mutex_lock(&freqconv_mutex_lock);
	list_add_tail(&h->link,&freqconv_handlers);
	mutex_unlock(&freqconv_mutex_lock);
}
EXPORT_SYMBOL(register_freqconv_change);

void unregister_freqconv_change(int (*handler)(unsigned int, unsigned int))
{
	struct handler_info *h;
	unsigned int found = 0;
	printk("Enter unregister_freqconv_change\n");
	list_for_each_entry(h, &freqconv_handlers, link) {
		if(h->notify == handler) {
			found = 1;
			break;
		}
	}
	if(found){
		mutex_lock(&freqconv_mutex_lock);
		list_del(&h->link);
		mutex_unlock(&freqconv_mutex_lock);
		kfree(h);
	}else{
		printk(KERN_ERR "unregister_freqconv_change handler not found.\n");
	}
}
EXPORT_SYMBOL(unregister_freqconv_change);

void freqconv_donotify(struct freqconv_cmd *pcmd)
{
	struct handler_info *h;
	if(pcmd == NULL) {
		printk(KERN_ERR "null point \n");
		return;
	}
	printk(KERN_INFO "freqconv_donotify data: %d, %d, %d, %d \n", pcmd->cmd_type, pcmd->length, pcmd->freq, pcmd->suspended);
	if(pcmd->cmd_type == PS_FREQ_CMD) {
		list_for_each_entry(h, &freqconv_handlers, link) {
			if(h->notify != NULL) {
				h->notify(pcmd->freq, pcmd->suspended);
			}
		}
	} else {
			/* do something here */
	}
}

static int freqconv_thread(void * data)
{
	int numread = 0;
	int fd = -1;
	struct freqconv_cmd freqconv_cmd = {0};
	printk("enter freqconv_thread \n");
	fd = ex_open_k(PSFREQ_DEV);
	if(fd < 0) {
		printk(KERN_WARNING "open freqconv failed %d \n", fd);
		return 0;
	}
	printk("freqconv open vbpipe success. \n");
	for(;;){
		printk(KERN_INFO "ready to read freqconv pipe %d\n", sizeof(struct freqconv_cmd));
		numread = ex_read_k(PSFREQ_DEV, (char*)&freqconv_cmd, sizeof(struct freqconv_cmd));
		printk(KERN_INFO "read %d numread form freqconv pipe \n", numread);
		if(numread == 0) {
			ex_release_k(PSFREQ_DEV);
			fd = ex_open_k(PSFREQ_DEV);
		        if(fd < 0) {
				printk(KERN_WARNING "open freqconv failed %d \n", fd);
				return 0;
			}
			continue;
		} else if(numread < 0) {
			printk(KERN_WARNING "Read error %d \n ", numread);
			msleep(1);
			continue;
		} else {
			printk(KERN_INFO "freqconv_thread numread is %d \n", numread);
			freqconv_donotify(&freqconv_cmd);
			/*feedback result to invoker*/
			if(ex_write_k(PSFREQ_DEV, (char*)&freqconv_cmd.cmd_type, sizeof(freqconv_cmd.cmd_type)) <= 0){
				printk(KERN_WARNING "freqconv feedback result failed %d \n");
			}
		}
	}
	return 0;
}
static int __init freqconv_init(void)
{
	struct task_struct * task;
	printk(KERN_INFO "Enter freqconv_init\n");
	task = kthread_create(freqconv_thread, NULL, "freqconv");
	if (task == NULL) {
		printk(KERN_ERR "Can't crate freqconv!\n");
	}else
		wake_up_process(task);

	return 0;
}

late_initcall(freqconv_init);