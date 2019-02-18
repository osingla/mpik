#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/ioctl.h>
#include <linux/slab.h>

#define MPIK_DEVICENAME 	"mpik"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Olivier Singla");
MODULE_DESCRIPTION("MPIK (Message Passing In Kernel)");

#include "mpik_ioctl.h"
#include "mpik.h"

static dev_t mpik_dev_nb;
static struct cdev* mpik_cdev;
static struct class* mpik_class;
static struct device* mpik_dev;

// Channels
static struct mutex mutex_channels;
static int nb_channels;
static channel_t *channels[MAXNB_CHANNELS];

// procfs
static int procfs_sz;
static int procfs_len;
static char *procfs_buff;

// Function prototypes
static ssize_t DriverRead(struct file* instanz, char __user* user, size_t count, loff_t* offset);
static int mpik_open(struct inode* geraete_datei, struct file* instanz);
static int mpik_close(struct inode* geraete_datei, struct file* instanz);

static struct proc_dir_entry *procfs_ent;

/* **
 * Return the base  of a filename (last part of the filename)
 */
static inline const char * __file__( const char *filename ) {
	char const *p = strrchr( filename, '/' );
	if ( p )
		return p+1;
	else
		return filename;
}

#define DEBUG(fmt, ...) debug(__FILE__, __LINE__, __FUNCTION__, fmt, ## __VA_ARGS__)
static void debug(char const *file, int line, char const *fcnt, char const *fmt, ...) {
	va_list args;
	char buff[200];

	va_start(args, fmt);
	vsnprintf(buff, sizeof(buff)-1, fmt, args);
	va_end(args);
   	printk(KERN_INFO "%s.%d - %-22s - %d %d - %s\n", __file__(file), line, fcnt, current->pid, current->tgid, buff);
}

static void *scull_seq_start(struct seq_file *s, loff_t *pos)
{
//	if (*pos >= scull_nr_devs)
//		return NULL;   /* No more to read. */
//	return scull_devices + *pos;
	return 0;
}

static void *scull_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	(*pos)++;
//	if (*pos >= scull_nr_devs)
//		return NULL;
//	return scull_devices + *pos;
	return 0;
}

static void scull_seq_stop(struct seq_file *s, void *v)
{
	/* There's nothing to do here! */
}

static int scull_seq_show(struct seq_file *s, void *v)
{
	return 0;
}
	
/*
 * Set up the sequence operator pointers.
 */
static struct seq_operations scull_seq_ops = {
	.start = scull_seq_start,
	.next  = scull_seq_next,
	.stop  = scull_seq_stop,
	.show  = scull_seq_show
};

static ssize_t procfs_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos) 
{
	DEBUG("count=%d ppos=%d", count, *ppos);
	return -1;
}

static void add_procfs_buffer(char const *fmt, ...) {
	char line[128];
	
	va_list args;
	va_start(args, fmt);
	vsnprintf(line, sizeof(line)-1, fmt, args);
	va_end(args);
	
	if (procfs_buff == NULL) {
		procfs_buff = (char *)kmalloc(32768, GFP_KERNEL);
		strcpy(procfs_buff, "");
		procfs_sz = 32768;
		procfs_len = 0;
	}
	strcat(procfs_buff, line);
	procfs_len = strlen(procfs_buff)+1;
}

static void gen_procfs_buffer(void) {
	int n;
	
	add_procfs_buffer("%d channel%s\n\n", nb_channels, (nb_channels > 1) ? "s" : "");
	add_procfs_buffer("Name       Owner\n");
	for (n = 0; n < nb_channels; n++) {
		channel_t *s = channels[n];
		add_procfs_buffer("%-22s %3d %6d %6d\n", s->name, s->chid, s->pid_owner, s->tgid_owner);
	}
}
 
static ssize_t procfs_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos) 
{
	DEBUG("file=%p count=%d ppos=%p (%lld)", file, count, ppos, *ppos);
	if (*ppos == 0)
		gen_procfs_buffer();
	DEBUG("%d [%s]", procfs_len, procfs_buff);
	if (*ppos > 0)
		return 0;
	if (copy_to_user(ubuf, procfs_buff, procfs_len))
		return -EACCES;
	*ppos = procfs_len;
	return procfs_len;
}
 
static struct file_operations procfs_mpik = 
{
	.owner = THIS_MODULE,
	.read = procfs_read,
	.write = procfs_write,
};
 
static int mpik_open(struct inode* geraete_datei, struct file* instanz)
{
	DEBUG("mpik_open");

	return 0;
}

static int mpik_close(struct inode* geraete_datei, struct file* instanz)
{
	DEBUG("mpik_close");

	return 0;
}

static ssize_t DriverRead(struct file* instanz, char __user* user, size_t count, loff_t* offset)
{
	return 0;
}

static channel_t *search_channel_by_name(char const *name) {
	int n;
	
	for (n = 0; n < nb_channels; n++) {
		channel_t *c = channels[n];
		if (!strcmp(c->name, name))
			return c;
	}
	return NULL;
}

static channel_t *search_channel_by_id(int chid) {
	int n;
	
	for (n = 0; n < nb_channels; n++) {
		channel_t *c = channels[n];
DEBUG("%d: %d , %d - [%s]", n, c->chid, chid, c->name);
		if (c->chid == chid)
			return c;
	}
	return NULL;
}

static int mpik_channel_create(mpik_ioctl_channel_create_t *m) {

	mpik_ioctl_channel_create_t s;
	if (copy_from_user(&s, m, sizeof(mpik_ioctl_channel_create_t)))
		return -EACCES;

	DEBUG("name=[%s] msec_timeout=%d maxnb_msg_buffered=%d", 
		s.name, s.msec_timeout, s.maxnb_msg_buffered);
	
	mutex_lock(&mutex_channels);

	// Find a new channel ID
	int chid = -1;
	for (int n = 0; n < nb_channels; n++) {
		channel_t *c = channels[n];
		if (c->chid > chid)
			chid = c->chid;
	}

	channel_t *ch = (channel_t *)kmalloc(sizeof(channel_t), GFP_KERNEL);
	channels[nb_channels++] = ch;
	
	memset(ch, 0, sizeof(channel_t));
	strncpy(ch->name, m->name, sizeof(ch->name));
	ch->maxnb_msg_buffered = s.maxnb_msg_buffered;
	ch->pid_owner = current->pid;
	ch->tgid_owner = current->tgid;
    init_waitqueue_head(&ch->queue);
	
	ch->chid = chid+1;
		
	mutex_unlock(&mutex_channels);
	DEBUG("[%s] chid=%d", s.name, chid);

	return chid+1;
}

static int mpik_channel_delete(mpik_ioctl_channel_delete_t *m) {

	mpik_ioctl_channel_delete_t s;
	if (copy_from_user(&s, m, sizeof(mpik_ioctl_channel_delete_t)))
		return -EACCES;

	DEBUG("chid=%d msec_timeout=%d", 
		s.chid, s.msec_timeout);
	
	int status = 0;
	mutex_lock(&mutex_channels);

	channel_t *ch = search_channel_by_id(s.chid);
	if (ch == NULL) {
		status = -ENOENT;
		goto done;
	}

done:	
	mutex_lock(&mutex_channels);
	return status;
}

static int mpik_channel_connect(mpik_ioctl_channel_connect_t *m) {

	mpik_ioctl_channel_connect_t s;
	channel_t *ch;
	
	if (copy_from_user(&s, m, sizeof(mpik_ioctl_channel_connect_t)))
		return -EACCES;

	DEBUG("name=[%s] msec_timeout=%d", 
		s.name, s.msec_timeout);

	ch = search_channel_by_name(s.name);
	if (ch == NULL)
		return -ENOENT;
	
	return 0;
}

static int mpik_channel_disconnect(mpik_ioctl_channel_disconnect_t *m) {

	mpik_ioctl_channel_disconnect_t s;
	
	if (copy_from_user(&s, m, sizeof(mpik_ioctl_channel_disconnect_t)))
		return -EACCES;

	DEBUG("chid=%d msec_timeout=%d", 
		s.chid, s.msec_timeout);

	return 0;
}

static int mpik_receive(mpik_ioctl_receive_t *m) {

	mpik_ioctl_receive_t s;
	channel_t *ch;
	
	if (copy_from_user(&s, m, sizeof(mpik_ioctl_receive_t)))
		return -EACCES;

	DEBUG("mpik_receive: chid=%d recv_buffer=%p recv_buffer_sz=%d msec_timeout=%d", 
		s.chid, s.recv_buffer, s.recv_buffer_sz, s.msec_timeout);

	ch = search_channel_by_id(s.chid);
	if (ch == NULL)
		return -ENOENT;
		
	ch->recv_buffer = s.recv_buffer;
	ch->recv_buffer_sz = s.recv_buffer_sz;

	ch->index = -1;
	wait_event_interruptible(ch->queue, ch->index != -1);
	DEBUG("Woke up - index=%d", ch->index);
	return ch->index;
}

static int mpik_reply(mpik_ioctl_reply_t *m) {

	mpik_ioctl_reply_t s;
	
	if (copy_from_user(&s, m, sizeof(mpik_ioctl_reply_t)))
		return -EACCES;

	DEBUG("chid=%d index=%d reply_buffer=%p reply_len=%d msec_timeout=%d", 
		s.chid, s.index, s.reply_buffer, s.reply_len, s.msec_timeout);

	return 0;
}

static int mpik_send(mpik_ioctl_send_t *m) {

	mpik_ioctl_send_t s;
	channel_t *ch;
	send_waiting_t *sendw;
	int index;
	
	if (copy_from_user(&s, m, sizeof(mpik_ioctl_send_t)))
		return -EACCES;

	DEBUG("chid=%d send_buffer=%p send_len=%d reply_buffer=%p reply_maxlen=%d msec_timeout=%d", 
		s.chid, s.send_buffer, s.send_len, s.reply_buffer, s.reply_maxlen, s.msec_timeout);

	ch = search_channel_by_id(s.chid);
	if (ch == NULL)
		return -ENOENT;

	// Copy the message into the receiver memory
	DEBUG("### [%s]\n", s.send_buffer);
	int ok = access_ok(VERIFY_WRITE, ch->recv_buffer, s.send_len);
	DEBUG("### ok=%d - [%s] %d\n", ok, s.send_buffer, s.send_len);
	memcpy(ch->recv_buffer, s.send_buffer, s.send_len);
	DEBUG("### ok=%d\n", ok);

	index = 0;
	sendw = &ch->send_waiting[index];
	sendw->reply_buffer = s.reply_buffer;
	sendw->reply_maxlen = s.reply_maxlen;
    init_waitqueue_head(&sendw->queue);

	// Wake-up the receiving task which is blocked on mpik_receive()
	ch->index = index;
DEBUG("&&&");
	wake_up_interruptible(&ch->queue);
DEBUG("&&&");

	// Sender is now blocked - It will be wake up when the receiver task will reply
	wait_event_interruptible(sendw->queue, false);

	return index;
}

static int mpik_ping(mpik_ioctl_ping_t *m) {

	mpik_ioctl_ping_t s;
	
	if (copy_from_user(&s, m, sizeof(mpik_ioctl_ping_t)))
		return -EACCES;

	DEBUG("chid=%d msec_timeout=%d", 
		s.chid, s.msec_timeout);

	return 0;
}

/* This function is called whenever a process tries to 
 * do an ioctl on our device file. We get two extra 
 * parameters (additional to the inode and file 
 * structures, which all device functions get): the number
 * of the ioctl called and the parameter given to the 
 * ioctl function.
 *
 * If the ioctl is write or read/write (meaning output 
 * is returned to the calling process), the ioctl call 
 * returns the output of this function.
 */
long mpik_ioctl(
    struct file *filp,
    unsigned int cmd,
    unsigned long param)
{
	DEBUG("cmd=%X param=%lX", cmd, param);

	switch (cmd) {
		case IOCTL_MPIK_CHANNEL_CREATE:
			return mpik_channel_create((mpik_ioctl_channel_create_t *)param);
		case IOCTL_MPIK_CHANNEL_DELETE:
			return mpik_channel_delete((mpik_ioctl_channel_delete_t *)param);
		case IOCTL_MPIK_CHANNEL_CONNECT:
			return mpik_channel_connect((mpik_ioctl_channel_connect_t *)param);
		case IOCTL_MPIK_CHANNEL_DISCONNECT:
			return mpik_channel_disconnect((mpik_ioctl_channel_disconnect_t *)param);
		case IOCTL_MPIK_RECEIVE:
			return mpik_receive((mpik_ioctl_receive_t *)param);
		case IOCTL_MPIK_REPLY:
			return mpik_reply((mpik_ioctl_reply_t *)param);
		case IOCTL_MPIK_SEND:
			return mpik_send((mpik_ioctl_send_t *)param);
		case IOCTL_MPIK_PING:
			return mpik_ping((mpik_ioctl_ping_t *)param);
	}

	return 0;
}

// File operations
static struct file_operations FOPS = {
	.owner = THIS_MODULE,
	.read = DriverRead,
	.unlocked_ioctl = mpik_ioctl,
	.open = mpik_open, 
	.release = mpik_close,
};

// Module init point
static int __init mpik_proc_init(void)
{
	// Register a range of char device numbers
	if (alloc_chrdev_region(&mpik_dev_nb, 0, 1, MPIK_DEVICENAME) < 0)
	{
		return -EIO;
	}
	DEBUG("mpik_dev_nb=%X", mpik_dev_nb);

	// Get a cdev object from the kernel
	mpik_cdev = cdev_alloc();
	if(mpik_cdev == NULL)
	{
		goto Jump_Free_DeviceNumber;
	}

	mpik_cdev->owner = THIS_MODULE;
	mpik_cdev->ops = &FOPS;

	// Add a char device to the system
	if(cdev_add(mpik_cdev, mpik_dev_nb, 1))
	{
		goto Jump_Free_cdev;
	}

	// Create a struct class structure
	mpik_class = class_create(THIS_MODULE, MPIK_DEVICENAME);
	if(IS_ERR(mpik_class)) 
	{
		pr_err("No udev support!\n");
		goto Jump_Free_cdev;
	}

	// Creates a device and registers it with sysfs
	mpik_dev = device_create(mpik_class, NULL, mpik_dev_nb, NULL, "%s", MPIK_DEVICENAME);
	if(IS_ERR(mpik_dev)) 
	{
		pr_err("'device_create' failed!\n");
		goto Jump_Free_class;
	}

	// Create /proc/mpik
	procfs_ent = proc_create("mpik", 0666, NULL, &procfs_mpik);
	DEBUG("procfs_ent=%p", procfs_ent);
	//	seq_open(filp, &scull_seq_ops);

	// Channels
	mutex_init(&mutex_channels);
	nb_channels = 0;
	memset(channels, 0, sizeof(channels));

	return 0;

	// Jumppoints for different errors during the init process
	// The jump points represent the inverse initialization process
	Jump_Free_class:
		class_destroy(mpik_class);
	Jump_Free_cdev:
		kobject_put(&mpik_cdev->kobj);
	Jump_Free_DeviceNumber:
		unregister_chrdev_region(mpik_dev_nb, 1);
		return -EIO;
}

// Module exit point
static void __exit mpik_proc_exit(void)
{
	// Removes the created device
	device_destroy(mpik_class, mpik_dev_nb);

	// Remove the class object 
	class_destroy(mpik_class);

	// Unregister the cdev object
	cdev_del(mpik_cdev);

	// Release the registered device number
	unregister_chrdev_region(mpik_dev_nb, 1);

	// Release /proc/mpik
	proc_remove(procfs_ent);

	return;
}

module_init(mpik_proc_init);
module_exit(mpik_proc_exit);