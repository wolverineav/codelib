/*
 * virtual dev - static block device mydev
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h> //printk()
#include <linux/fs.h>     //everything
#include <linux/errno.h>  //error codes
#include <linux/types.h>  //size_t
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>

//to access /proc file (fs.h too)
#include <linux/syscalls.h>
#include <linux/file.h>
#include <asm/uaccess.h>

//to pass data via netlink sockets
#include <linux/netlink.h>
#include <net/genetlink.h>

/* netlink data */

static struct genl_info incoming;

/* attributes (variables): the index in this enum is used as a reference for the type,
 *             userspace application has to indicate the corresponding type
 *             the policy is used for security considerations 
 */
enum {
	DOC_EXMPL_A_UNSPEC,
	DOC_EXMPL_A_MSG,
        __DOC_EXMPL_A_MAX,
};
#define DOC_EXMPL_A_MAX (__DOC_EXMPL_A_MAX - 1)

/* attribute policy: defines which attribute has which type (e.g int, char * etc)
 * possible values defined in net/netlink.h 
 */
static struct nla_policy doc_exmpl_genl_policy[DOC_EXMPL_A_MAX + 1] = {
	[DOC_EXMPL_A_MSG] = { .type = NLA_NUL_STRING },
};

#define VERSION_NR 1
/* family definition */
static struct genl_family doc_exmpl_gnl_family = {
	.id = GENL_ID_GENERATE,         //genetlink should generate an id
	.hdrsize = 0,
	.name = "CONTROL_EXMPL",        //the name of this family, used by userspace application
	.version = VERSION_NR,                   //version number  
	.maxattr = DOC_EXMPL_A_MAX,
};

/* commands: enumeration of all commands (functions), 
 * used by userspace application to identify command to be ececuted
 */
enum {
	DOC_EXMPL_C_UNSPEC,
	DOC_EXMPL_C_ECHO,
	__DOC_EXMPL_C_MAX,
};
#define DOC_EXMPL_C_MAX (__DOC_EXMPL_C_MAX - 1)


/* an echo command, receives a message, prints it and sends another message back */
int doc_exmpl_echo(struct sk_buff *skb_2, struct genl_info *info)
{
        struct nlattr *na;
        struct sk_buff *skb;
        int rc;
	void *msg_head;
	char * mydata;
	
        if (info == NULL)
                goto out;
  
        /*for each attribute there is an index in info->attrs which points to a nlattr structure
         *in this structure the data is given
         */
	incoming = *info;
        na = info->attrs[DOC_EXMPL_A_MSG];
       	if (na) {
		mydata = (char *)nla_data(na);
		if (mydata == NULL)
			printk("error while receiving data\n");
		else
			printk("received: %s\n", mydata);
		}
	else
		printk("no info->attrs %i\n", DOC_EXMPL_A_MSG);

        /* send a message back*/
        /* allocate some memory, since the size is not yet known use NLMSG_GOODSIZE*/	
        skb = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
	if (skb == NULL)
		goto out;

	/* create the message headers */
        /* arguments of genlmsg_put: 
           struct sk_buff *, 
           int (sending) pid, 
           int sequence number, 
           struct genl_family *, 
           int flags, 
           u8 command index (why do we need this?)
        */
       	msg_head = genlmsg_put(skb, 0, info->snd_seq+1, &doc_exmpl_gnl_family, 0, DOC_EXMPL_C_ECHO);
	if (msg_head == NULL) {
		rc = -ENOMEM;
		goto out;
	}
	/* add a DOC_EXMPL_A_MSG attribute (actual value to be sent) */
	rc = nla_put_string(skb, DOC_EXMPL_A_MSG, "registered user daemon with kernel.\n");
	if (rc != 0)
		goto out;
	
        /* finalize the message */
	genlmsg_end(skb, msg_head);

        /* send the message back */
	rc = genlmsg_unicast(skb,info->snd_pid );
	if (rc != 0)
		goto out;
	return 0;

 out:
        printk("an error occured in doc_exmpl_echo:\n");
  
      return 0;
}
/* commands: mapping between the command enumeration and the actual function*/
struct genl_ops doc_exmpl_gnl_ops_echo = {
	.cmd = DOC_EXMPL_C_ECHO,
	.flags = 0,
	.policy = doc_exmpl_genl_policy,
	.doit = doc_exmpl_echo,
	.dumpit = NULL,
};


/* end of netlink data */


//store a byte of data
//typedef unsigned char u8;

	
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Aditya Vaja");

static int major_num = 0;
module_param(major_num, int, 0);
static int hardsect_size = 512;
module_param(hardsect_size, int, 0);
static int nsectors = 512;  //How big the drive is
module_param(nsectors, int, 0);

// read write counters
int rcount=0,wcount=0;

//kernel sector size
#define KERNEL_SECTOR_SIZE 512

//the request queue
static struct request_queue *Queue;


//our device representation
static struct mydev_device {
	unsigned long size;
	spinlock_t lock;
	u8 *data;
	struct gendisk *gd;
} Device;


//send data to app

//io req handler
static void mydev_transfer(struct mydev_device *dev, unsigned long sector, unsigned long nsect, char *buffer, int write)
{
	/* netlink decl */
	struct sk_buff *skb;
	int rc;
	void *msg_head;
	//struct genl_info *info;
	//normal declaration
	unsigned long offset = sector*hardsect_size;
	unsigned long nbytes = nsect*hardsect_size;
	unsigned long i;
	unsigned long sect = sector;
	int size = sizeof(unsigned long);

	u8 *as[100];
	if ((offset + nbytes) > dev->size) {
		printk (KERN_NOTICE "mydev: Beyond-end write (%ld %ld)\n", offset, nbytes);
		return;
	}
	if (write)	//the section required to be monitored.
	{
		memcpy(dev->data + offset, buffer, nbytes);
		/*send sector no*/
		{
			if(incoming.snd_pid!=NULL)
			{
			/* send a message back*/
			/* allocate some memory, since the size is not yet known use NLMSG_GOODSIZE*/	
			skb = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
			if (skb == NULL)
				goto out;

		       	msg_head = genlmsg_put(skb, 0, incoming.snd_seq+1, &doc_exmpl_gnl_family, 0, DOC_EXMPL_C_ECHO);
			if (msg_head == NULL) {
				rc = -ENOMEM;
				printk(KERN_INFO "msg null\n");
				goto out;
			}
			/* add a DOC_EXMPL_A_MSG attribute (actual value to be sent) */
		
			rc = nla_put_u64(skb, NLA_U64,sect);
			printk(KERN_INFO "sent to user:%lx\n",sect);
			if (rc != 0){
				printk(KERN_INFO "put string me prob\n");
				goto out;
			}
			/* finalize the message */
			genlmsg_end(skb, msg_head);

			/* send the message back */
			rc = genlmsg_unicast(skb,incoming.snd_pid );
			if (rc != 0){
				printk(KERN_INFO "unicast me prob\n");
				goto out;
			}
	
			out:
			printk("an error occured in doc_exmpl_echo in send:\n");
			printk(KERN_INFO "rc=%d.\n", rc);
		  
		    
			}
		}

		/*send no of sectors*/
		{
			if(incoming.snd_pid!=NULL)
			{
			/* send a message back*/
			/* allocate some memory, since the size is not yet known use NLMSG_GOODSIZE*/	
			skb = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
			if (skb == NULL)
				goto out1;

		       	msg_head = genlmsg_put(skb, 0, incoming.snd_seq+1, &doc_exmpl_gnl_family, 0, DOC_EXMPL_C_ECHO);
			if (msg_head == NULL) {
				rc = -ENOMEM;
				printk(KERN_INFO "msg null\n");
				goto out1;
			}
			/* add a DOC_EXMPL_A_MSG attribute (actual value to be sent) */
		
			rc = nla_put_u64(skb, NLA_U64,nsect);
			//printk(KERN_INFO "sent to user:%lx\n",sect);
			if (rc != 0){
				printk(KERN_INFO "put string me prob\n");
				goto out1;
			}
			/* finalize the message */
			genlmsg_end(skb, msg_head);

			/* send the message back */
			rc = genlmsg_unicast(skb,incoming.snd_pid );
			if (rc != 0){
				printk(KERN_INFO "unicast me prob\n");
				goto out1;
			}
	
			out1:
			printk("an error occured in doc_exmpl_echo in send:\n");
			printk(KERN_INFO "rc=%d.\n", rc);
		  
		    
			}
		}


		printk(KERN_INFO "write %lx sectors:\n", nsect);
		printk(KERN_INFO "starting at sector:%ld\n", sector);
		//write_file("/proc/kernshare", (dev->data+offset));
		
		printk(KERN_INFO "write %d\n", wcount++);
		}
		else
		{
			memcpy(buffer, dev->data + offset, nbytes);
			printk(KERN_INFO "read %d.\n", rcount++);
		}
}

static void mydev_request(request_queue_t* q)
{
	struct request* req;

	while ((req = elv_next_request(q)) != NULL) {
	if (! blk_fs_request(req)) {
		printk (KERN_NOTICE "Skip non-CMD request\n");
		end_request(req, 0);
		continue;
	}
	mydev_transfer(&Device, req->sector, req->current_nr_sectors,req->buffer, rq_data_dir(req));
	end_request(req, 1);
	}
}

//ioctl handler
int mydev_ioctl (struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	long size;
	struct hd_geometry geo;

	switch(cmd) {
		
		//The only command we need to interpret is HDIO_GETGEO, since
		//we can't partition the drive otherwise.  We have no real
		//geometry, of course, so make something up.
		
		case HDIO_GETGEO:
		size = Device.size*(hardsect_size/KERNEL_SECTOR_SIZE);
		geo.cylinders = (size & ~0x3f) >> 6;
		geo.heads = 4;
		geo.sectors = 16;
		geo.start = 4;
		if (copy_to_user((void *) arg, &geo, sizeof(geo)))
			return -EFAULT;
		return 0;
	}

	return -ENOTTY; //command unknown
}




//operations suported by the device
static struct block_device_operations mydev_ops = {
	.owner	=	THIS_MODULE,
	.ioctl	=	mydev_ioctl,
	//.getgeo	     = mydev_blk_getgeo
};

static int __init mydev_init(void)
{

	int rc;
	//setup the device
	Device.size = nsectors*hardsect_size;
	spin_lock_init(&Device.lock);
	Device.data = vmalloc(Device.size);
	if (Device.data == NULL)
		return -ENOMEM;

	//get a request queue
	Queue = blk_init_queue(mydev_request, &Device.lock);
	if (Queue == NULL)
		goto out;
	blk_queue_hardsect_size(Queue, hardsect_size);

	//register the device
	major_num = register_blkdev(major_num, "mydev");
	if (major_num <= 0) {
		printk(KERN_WARNING "mydev: unable to get major number\n");
		goto out;
	}
	else
	{
		printk(KERN_INFO "mydev registered with major number: %d \n",major_num);
	}

	//add the general disk struct
	Device.gd = alloc_disk(16);
	if (! Device.gd)
		goto out_unregister;
	Device.gd->major = major_num;
	Device.gd->first_minor = 0;
	Device.gd->fops = &mydev_ops;
	Device.gd->private_data = &Device;
	strcpy (Device.gd->disk_name, "mydev0");
	set_capacity(Device.gd, nsectors*(hardsect_size/KERNEL_SECTOR_SIZE));
	Device.gd->queue = Queue;
	add_disk(Device.gd);

	
        printk("INIT GENERIC NETLINK EXEMPLE MODULE\n");
        
        /*register new family*/
	rc = genl_register_family(&doc_exmpl_gnl_family);
	if (rc != 0)
		goto failure;
        /*register functions (commands) of the new family*/
	rc = genl_register_ops(&doc_exmpl_gnl_family, &doc_exmpl_gnl_ops_echo);
	if (rc != 0){
                printk("register ops: %i\n",rc);
                genl_unregister_family(&doc_exmpl_gnl_family);
		goto failure;
        }

	return 0;

	failure:
        	printk("an error occured while inserting the generic netlink example module\n");
		return -1;
	out_unregister:
		unregister_blkdev(major_num, "mydev");
	out:
		vfree(Device.data);
	return -ENOMEM;
}

static void __exit mydev_exit(void)
{
	int ret;
	
	del_gendisk(Device.gd);
	put_disk(Device.gd);
	unregister_blkdev(major_num, "mydev");
	blk_cleanup_queue(Queue);
	vfree(Device.data);
	printk(KERN_INFO "mydev removed from system.\n");
	
	/* netlink unregister stuff */
	
        /*unregister the functions*/
	ret = genl_unregister_ops(&doc_exmpl_gnl_family, &doc_exmpl_gnl_ops_echo);
	if(ret != 0){
                printk("unregister ops: %i\n",ret);
                return;
        }
        /*unregister the family*/
	ret = genl_unregister_family(&doc_exmpl_gnl_family);
	if(ret !=0){
                printk("unregister family %i\n",ret);
        }

}
	
module_init(mydev_init);
module_exit(mydev_exit);
