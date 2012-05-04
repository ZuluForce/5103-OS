#ifndef _SCULLBUFFER_H_
#define _SCULLBUFFER_H_


#include <linux/configfs.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <linux/seq_file.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/ioctl.h>

#include <asm/system.h>
#include <asm/uaccess.h>

#define SCULL_MAJOR 0   /* dynamic major by default */
#define SCULL_NR_DEVS 1    /* scullBuffer0 */
#define SCULL_SIZE 64	/* default # items for buffer */
#define ITEM_SIZE 512

struct item {
	int size;
	char data[ITEM_SIZE];
};

struct scull_buffer {
	struct item *bufferPtr;		/* pointer to the item buffer */
	int readIndex,writeIndex;
	struct semaphore sem; 	/* mutual exclusion semaphore */
	struct semaphore item_sem;	/* couting semaphore for the # of items */
	struct semaphore space_sem;	/* couting semaphore for the buffer space */
	int readerCnt;			/* count of no of readers accessing the device */
	int writerCnt;			/* count of no of writers accessing the device */
	int inReadCnt;		/* count of number readers in read */
	int inWriteCnt;		/* count of number writers in write */
	int size;				/* amount of data held in the buffer currently (# items) */
	struct cdev cdev;		/* Char device structure		*/
};

/*
 * The different configurable parameters
 */
extern int scull_major;
extern int scull_minor;
extern int scull_size;

/* Function prototypes */
void scull_cleanup_module(void);
static void scull_setup_cdev(struct scull_buffer *dev);
int scull_init_module(void);
int scullBuffer_open(struct inode *inode, struct file *filp);
int scullBuffer_release(struct inode *inode, struct file *filp);
ssize_t scullBuffer_read(struct file *filp, char __user *buf, size_t count,loff_t *f_pos);
ssize_t scullBuffer_write(struct file *filp, const char __user *buf, size_t count,loff_t *f_pos);

#endif /* _SCULLBUFFER_H_ */