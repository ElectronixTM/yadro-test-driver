#include <linux/device-mapper.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/bio.h>

struct proxy_t
{
  struct dm_dev* dev;
  sector_t start;
  sector_t len;
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
    ti->error = "dm-proxy: Device lookup failed";
  }
  ti->private = proxy_context;
  printk( KERN_DEBUG "dm-proxy for %s has been successfully created", argv[0]);
  return 0;
}

// TODO: implement
static int dmp_map(struct dm_target* ti, struct bio* bio)
{
  return 0;
}

// TODO: implement
static void dmp_dtr(struct dm_target* ti)
{
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
  printk(KERN_DEBUG "dm proxy succesfully initialized");
  return 0;
}

static void __exit dmp_exit(void)
{
  dm_unregister_target(&dmp_target)
  printk(KERN_DEBUG "proxy was unregistered");
}

module_init(dmp_init);
module_exit(dmp_exit);
MODULE_LICENSE("GPL");
