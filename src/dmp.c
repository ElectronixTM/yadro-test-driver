#include <linux/device-mapper.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/bio.h>
#include <linux/bvec.h>

struct proxy_t
{
  struct dm_dev* dev;
  size_t read_rq_num;
  size_t write_rq_num;
  size_t total_read;
  size_t total_write;
};

static int dmp_ctr(struct dm_target* ti, unsigned int argc, char **argv)
{

  if (1 != argc)
  {
    ti->error = "Invalid amount of arguments. Only proxied device "
                " should be specified";
    return -EINVAL;
  }
  struct proxy_t* proxy_context;
  proxy_context = kmalloc(sizeof(struct proxy_t), GFP_KERNEL);
  if (NULL == proxy_context)
  {
    ti->error = "dmp: Cannot allocate linear context. "
                "Probably not enough memory";
    return -ENOMEM;
  }

  if (dm_get_device(
        ti, argv[0],
        dm_table_get_mode(ti->table),
        &proxy_context->dev
        )
      )
  {
    printk(KERN_WARNING "[dmp_ctr] opening device failed");
    ti->error = "dm-proxy: Device lookup failed";
    kfree(proxy_context);
    return -EINVAL;
  }
  printk(KERN_DEBUG "[dmp_ctr] new_dev_addr: %lx", (unsigned long) proxy_context->dev);
  ti->private = proxy_context;
  printk( KERN_DEBUG "[dmp_ctr] dm-proxy for %s has been successfully created\n", argv[0] );
  return 0;
}

static int dmp_map(struct dm_target* ti, struct bio* bio)
{
  struct proxy_t* proxy_context = (struct proxy_t*) ti->private;
  if (proxy_context == NULL)
  {
    printk(KERN_ERR "[dmp_map] context appared to be nullptr\n");
    ti->error = "No context for dmproxy provided";
    return DM_MAPIO_KILL;
  }

  if (proxy_context->dev == NULL)
  {
    printk(KERN_ERR "[dmp_map] no target device provided\n");
    return DM_MAPIO_KILL;
  }

  // bio->bi_bdev = proxy_context->dev->bdev;
  bio_set_dev(bio, proxy_context->dev->bdev);
  if (bio->bi_bdev == NULL)
  {
    printk(KERN_ERR "[dmp_map] block device ptr is NULL\n");
    return DM_MAPIO_KILL;
  }

  switch (bio_op(bio))
  {
    case REQ_OP_READ:
      proxy_context->read_rq_num += 1;
      proxy_context->total_read += bio->bi_iter.bi_size;
      break;
    case REQ_OP_WRITE_ZEROES:
    case REQ_OP_WRITE:
      proxy_context->write_rq_num += 1;
      proxy_context->total_write += bio->bi_iter.bi_size;
      break;
    default:
      printk(KERN_WARNING "[dmp_map] unsupported bio operation\n");
      return DM_MAPIO_KILL;
  }
  submit_bio(bio);
  printk(KERN_DEBUG "BIO proxied");
  return DM_MAPIO_SUBMITTED;
}

static void dmp_dtr(struct dm_target* ti)
{
  struct proxy_t* dmp_target = (struct proxy_t*) ti->private;
  printk(KERN_DEBUG "Destructing proxy for device %s\n", dmp_target->dev->name);
  dm_put_device(ti, dmp_target->dev);
  kfree(dmp_target);
  printk(KERN_DEBUG "Succesfully destructed dm proxy\n");
}

static struct target_type dmp_target = 
{
  .name = "dmp",
  .version={0,0,1},
  .module = THIS_MODULE,
  .ctr = dmp_ctr,
  .dtr = dmp_dtr,
  .map = dmp_map
};

static int __init dmp_init(void)
{
  if (dm_register_target(&dmp_target) < 0)
  {
    printk(KERN_CRIT "\n Error while registering new target \n");
  }
  printk(KERN_DEBUG "dm proxy succesfully initialized\n");
  return 0;
}

static void __exit dmp_exit(void)
{
  dm_unregister_target(&dmp_target);
  printk(KERN_DEBUG "proxy was unregistered");
}

module_init(dmp_init);
module_exit(dmp_exit);
MODULE_LICENSE("GPL");
