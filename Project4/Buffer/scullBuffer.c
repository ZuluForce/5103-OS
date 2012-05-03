#include "scullBuffer.h"

/* Parameters which can be set at load time */
int scull_major = SCULL_MAJOR;
int scull_minor = 0;
int scull_size = SCULL_SIZE;	/* number of scull Buffer items */

module_param(scull_major, int, S_IRUGO);
module_param(scull_minor, int, S_IRUGO);
module_param(scull_size, int, S_IRUGO);

MODULE_AUTHOR("Andrew Helgeson, Kevin Melhaff, Dylan Betterman");
MODULE_LICENSE("Dual BSD/GPL");

struct scull_buffer scullBufferDevice;	/* instance of the scullBuffer structure */

/* file ops struct indicating which method is called for which io operation */
struct file_operations scullBuffer_fops = {
	.owner =    THIS_MODULE,
	.read =     scullBuffer_read,
	.write =    scullBuffer_write,
	.open =     scullBuffer_open,
	.release =  scullBuffer_release,
};

/*
 * Method used to allocate resources and set things up when the module
 * is being loaded.
*/
int scull_init_module(void)
{
	int result = 0;
	dev_t dev = 0;

	/* first check if someone has passed a major number */
	if (scull_major) {
		dev = MKDEV(scull_major, scull_minor);
		result = register_chrdev_region(dev, SCULL_NR_DEVS, "scullBuffer");
	} else {
		/* we need a dynamically allocated major number..*/
		result = alloc_chrdev_region(&dev, scull_minor, SCULL_NR_DEVS,
				"scullBuffer");
		scull_major = MAJOR(dev);
	}
	if (result < 0) {
		printk(KERN_WARNING "scullBuffer: can't get major %d\n", scull_major);
		return result;
	}

	/* alloc space for the buffer (scull_size bytes) */
	scullBufferDevice.bufferPtr = kmalloc( scull_size * sizeof(struct item), GFP_KERNEL);
	memset((void*) scullBufferDevice.bufferPtr, 0, scull_size * sizeof(struct item));
	if(scullBufferDevice.bufferPtr == NULL)
	{
		scull_cleanup_module();
		return -ENOMEM;
	}
	/* Set the read/write pointers */
	scullBufferDevice.readIndex =
	scullBufferDevice.writeIndex = 0;

	/* Init the count vars */
	scullBufferDevice.readerCnt = 0;
	scullBufferDevice.writerCnt = 0;
	scullBufferDevice.inReadCnt = 0;
	scullBufferDevice.inWriteCnt = 0;
	scullBufferDevice.size = 0;

	/* Initialize the semaphores */
	sema_init(&scullBufferDevice.sem, 1);
	sema_init(&scullBufferDevice.item_sem,0);
	sema_init(&scullBufferDevice.space_sem,scull_size);

	/* Finally, set up the c dev. Now we can start accepting calls! */
	scull_setup_cdev(&scullBufferDevice);
	printk(KERN_DEBUG "scullBuffer: Done with init module ready for requests buffer size= %d\n",scull_size);
	return result;
}

/*
 * Set up the char_dev structure for this device.
 * inputs: dev: Pointer to the device struct which holds the cdev
 */
static void scull_setup_cdev(struct scull_buffer *dev)
{
	int err, devno = MKDEV(scull_major, scull_minor);

	cdev_init(&dev->cdev, &scullBuffer_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &scullBuffer_fops;
	err = cdev_add (&dev->cdev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		printk(KERN_NOTICE "Error %d adding scullBuffer\n", err);
}

/*
 * Method used to cleanup the resources when the module is being unloaded
 * or when there was an initialization error
 */
void scull_cleanup_module(void)
{
	dev_t devno = MKDEV(scull_major, scull_minor);

	/* if buffer was allocated, get rid of it */
	if(scullBufferDevice.bufferPtr != NULL)
		kfree(scullBufferDevice.bufferPtr);

	/* Get rid of our char dev entries */
	cdev_del(&scullBufferDevice.cdev);

	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno, SCULL_NR_DEVS);

	printk(KERN_DEBUG "scullBuffer: Done with cleanup module \n");
}

module_init(scull_init_module);
module_exit(scull_cleanup_module);

/*
 * Implementation of the open system call
*/
int scullBuffer_open(struct inode *inode, struct file *filp)
{
	struct scull_buffer *dev;
	/* get and store the container scull_buffer */
	dev = container_of(inode->i_cdev, struct scull_buffer, cdev);
	filp->private_data = dev;

	printk(KERN_DEBUG "scullBuffer: scullBuffer open was called\n");
	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	if (filp->f_mode & FMODE_READ)
		dev->readerCnt++;
	if (filp->f_mode & FMODE_WRITE)
		dev->writerCnt++;

	up(&dev->sem);
	return 0;
}

/*
 * Implementation of the close system call
*/
int scullBuffer_release(struct inode *inode, struct file *filp)
{
	struct scull_buffer *dev = (struct scull_buffer *)filp->private_data;
	if (down_interruptible(&dev->sem) )
		return -ERESTARTSYS;

	if (filp->f_mode & FMODE_READ)
		dev->readerCnt--;
	if (filp->f_mode & FMODE_WRITE)
		dev->writerCnt--;

	up(&dev->sem);
	return 0;
}

/*
 * Implementation of the read system call
*/
ssize_t scullBuffer_read(
	struct file *filp,
	char __user *buf,
	size_t count,
	loff_t *f_pos)
{
	struct scull_buffer *dev = (struct scull_buffer *)filp->private_data;
	ssize_t countRead = 0;
	struct item *forUser;
	
	printk(KERN_DEBUG "scullBuffer: Read system call was made\n");

	/* get exclusive access */
	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	printk(KERN_DEBUG "scullBuffer: read called count= %d\n", (int)count);
	printk(KERN_DEBUG "scullBuffer: cur pos= (%d,%d), size= %d \n",
			dev->readIndex, dev->writeIndex, dev->size);
	printk(KERN_DEBUG "scullBuffer: #writers = %d   #readers = %d\n",dev->writerCnt, dev->readerCnt);

	if (down_trylock(&dev->item_sem)) {
		
		//If no writers currently present return
		if ( dev->inWriteCnt == 0 ) {
			up(&dev->sem);
			return 0;
		}

		up(&dev->sem); //Let someone else use the device
		
		//Wait for an item to come in
		if (down_interruptible(&dev->item_sem))
			return -ERESTARTSYS;

		if (down_interruptible(&dev->sem)) {
			up(&dev->item_sem);
			return -ERESTARTSYS;
		}
	}

	if ( count == 0 ) {
		/* Do we treat a 0 read as consuming an item without
		 * actually reading it or do we not consume any items? */
		return 0;
	}

	dev->inReadCnt++;

	//Get the item from the buffer
	forUser = &dev->bufferPtr[dev->readIndex];

	//Get actual # of bytes to copy
	count = count < forUser->size ? count : forUser->size;

	if (copy_to_user(buf, forUser->data, count)) {
		countRead = -EFAULT;
		goto out;
	}
	
	countRead = count;

	--dev->size;
	dev->readIndex = (dev->readIndex+1) % scull_size; //Move ring buffer

	//Set the size of the item to -1 to mark unused
	forUser->size = -1;

	//Notify any waiting producers that there is space
	up(&dev->space_sem);

	/* now we're done, release the semaphore */
	out:
	dev->inReadCnt--;
	up(&dev->sem);
	return countRead;
}

/*
 * Implementation of the write system call
*/
ssize_t scullBuffer_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
	int countWritten = 0;
	struct scull_buffer *dev = (struct scull_buffer *)filp->private_data;
	struct item *newItem;
	
	printk(KERN_DEBUG "scullBuffer: write called on device\n");

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	/* have we crossed the size of the buffer? */
	printk(KERN_DEBUG "scullBuffer: write called count= %d\n", (int) count);
	printk(KERN_DEBUG "scullBuffer: cur pos= (%d,%d), size= %d \n",
			dev->readIndex, dev->writeIndex, (int)dev->size);
	printk(KERN_DEBUG "scullBuffer: (write) #write = %d   #read = %d\n",
			dev->writerCnt, dev->readerCnt);

        
	if (down_trylock(&dev->space_sem)) {
		if ( dev->inReadCnt == 0 ) {
			up(&dev->sem);
			return 0;
		}

		up(&dev->sem); //Let someone else use the device
		
		//Wait for space to open up
		if (down_interruptible(&dev->space_sem))
			return -ERESTARTSYS;

		/* Since we have already acquired the resource through
		 * this single channel then nobody else can get it meaning
		 * that there is no race condition between there and getting
		 * the binary semaphore.
		 */
		if (down_interruptible(&dev->sem)) {
			up(&dev->space_sem);
			return -ERESTARTSYS;
		}
	}

	dev->inWriteCnt++;

	//Get struct from buffer to insert the new item
	newItem = &dev->bufferPtr[dev->writeIndex];

	//truncate to at most ITEM_SIZE
	count = count > ITEM_SIZE ? ITEM_SIZE : count;
	printk(KERN_DEBUG "scullBuffer: writing %d bytes \n", (int)count);
	
	//Put the user's data into item struct
	newItem->size = count;
	if (copy_from_user((void*)newItem->data, buf, count)) {
		newItem->size = -1; //Identity for unused item struct
		countWritten = -EFAULT;
		goto out;
	}

	++dev->size;
	
	//Move ring buffer
	dev->writeIndex = (dev->writeIndex+1) % scull_size;
	countWritten = count;

	printk(KERN_DEBUG "scullBuffer: new pos= (%d,%d), new size= %d \n",
			dev->readIndex, dev->writeIndex, (int)dev->size);
	out:
	up(&dev->item_sem); //Notify that there are items
	dev->inWriteCnt--;
	up(&dev->sem);
	return countWritten;
}
