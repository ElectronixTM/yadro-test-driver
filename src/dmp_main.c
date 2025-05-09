#include <linux/device-mapper.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/bio.h>
#include <linux/bvec.h>

#include "proxy_type.h"
#include "dmp_stat.h"

// global stats from all the proxies accumulated
struct stat_t global_stats;
// sysfs 'volumes' file info
struct sysfs_helper_t global_volumes_info;

/**
 * Aside from mandatory parameters takes a path to device to be proxied.
 * Tries to get it and saves to context structure and initializes it's
 * statistics
 */
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
  memset(proxy_context, 0, sizeof(struct proxy_t));

  if (dm_get_device(
        ti, argv[0],
        dm_table_get_mode(ti->table),
        &proxy_context->dev
        )
      )
  {
    printk(KERN_WARNING "[dmp_ctr] opening device failed");
    ti->error = "dm-proxy: Device lookup failed";
    goto error;
  }
  proxy_context->stats = &global_stats;
  proxy_context->sysfs = &global_volumes_info;

  ti->private = proxy_context;
  printk(
        KERN_DEBUG "[dmp_ctr] dm-proxy for %s has been "
                   "successfully created\n", argv[0]
        );
  return 0;

error:
    kfree(proxy_context);
    return -EINVAL;
}

/**
 * Logging specified in task info into device context and redirects
 * read and write calls to device. Other calls are dropped because
 * unspecified in the task text.
 */
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

  // redirecting bio to proxied device
  bio_set_dev(bio, proxy_context->dev->bdev);
  if (bio->bi_bdev == NULL)
  {
    printk(KERN_ERR "[dmp_map] block device ptr is NULL\n");
    return DM_MAPIO_KILL;
  }

  switch (bio_op(bio))
  {
    case REQ_OP_READ:
      proxy_context->stats->read_rq_num += 1;
      proxy_context->stats->total_read += bio->bi_iter.bi_size;
      break;
    case REQ_OP_WRITE_ZEROES:
    case REQ_OP_WRITE:
      proxy_context->stats->write_rq_num += 1;
      proxy_context->stats->total_write += bio->bi_iter.bi_size;
      break;
    default:
      printk(KERN_WARNING "[dmp_map] unsupported bio operation\n");
      return DM_MAPIO_KILL;
  }
  submit_bio_noacct(bio);

  return DM_MAPIO_SUBMITTED;
}

static void dmp_dtr(struct dm_target* ti)
{
  struct proxy_t* dmp_target = (struct proxy_t*) ti->private;
  dm_put_device(ti, dmp_target->dev);
  kfree(dmp_target);
  printk(
      KERN_DEBUG "[dmp_dtr] Succesfully destructed dm proxy for "
                 "device %s\n", dmp_target->dev->name
        );
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
    printk(KERN_ERR "\n [dmp_init] Error while registering new target \n");
    return 1;
  }
  if (create_dmp_stat_file(&global_volumes_info, &global_stats) != 0)
  {
    printk(KERN_ERR "[dmp_ctr] unable to create stats file\n");
    dm_unregister_target(&dmp_target);
    return 1;
  }
  printk(KERN_INFO "[dmp_init] dm proxy succesfully initialized\n");
  return 0;
}

static void __exit dmp_exit(void)
{
  printk(KERN_INFO "[dmp_exit] destructing proxy");
  release_dmp_stat_file(&global_volumes_info);
  printk(KERN_DEBUG "[dmp_exit] stat/volumes file removed");
  dm_unregister_target(&dmp_target);
}

module_init(dmp_init);
module_exit(dmp_exit);
MODULE_LICENSE("GPL");
