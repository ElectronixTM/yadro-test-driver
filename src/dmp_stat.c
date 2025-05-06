#include "proxy_type.h"
#include <linux/device-mapper.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/device.h>
#include <linux/blk_types.h>
#include <linux/module.h>
#include <linux/printk.h>

#define MODULE_KOBJ ((((struct module*) THIS_MODULE)->mkobj).kobj)

/**
 * Utility function that performs devision of total amount of
 * bytes on amount of requests, but performs check on zero division
 */
static inline size_t calc_avg(size_t total, size_t amount)
{
  size_t avg = 0;
  if (amount != 0)
  {
    avg = total/amount;
  }
  return avg;
}

/**
 * Prints information into stat/volumes in formatted way as required in task spec.
 * relies on device_data in dev parameter. Returns amount of printed bytes
 */
static ssize_t volumes_show(struct device* dev, struct device_attribute* attr, char* buf)
{
  struct stat_t* stats = (struct stat_t*) dev_get_drvdata(dev);
  if (NULL == stats)
  {
    return -EINVAL;
  }

  printk(KERN_DEBUG "[volumes_show] read_rq_num: %lu\n", stats->read_rq_num);
  printk(KERN_DEBUG "[volumes_show] total_read: %lu\n", stats->total_read);
  printk(KERN_DEBUG "[volumes_show] read_rq_num: %lu\n", stats->read_rq_num);
  printk(KERN_DEBUG "[volumes_show] total_write: %lu\n", stats->total_write);

  ssize_t total_out = sprintf(
      buf,
      "read:\n"
      " reqs: %lu\n"
      " avg size: %lu\n"
      "write:\n"
      " reqs: %lu\n"
      " avg size: %lu\n"
      "total:\n"
      " reqs: %lu\n"
      " avg size: %lu\n",
      stats->read_rq_num,
      calc_avg(stats->total_read, stats->read_rq_num),
      stats->write_rq_num,
      calc_avg(stats->total_write, stats->write_rq_num),
      stats->read_rq_num+stats->write_rq_num,
      calc_avg(
        stats->total_read+stats->total_write,
        stats->read_rq_num+stats->write_rq_num
        )
      );
  return total_out;
}

/**
 * Creates file sys/module/dmp/stat/volumes and fills reciever with pointers,
 * needed by sysfs. Provided data should be freed when file is not needed with
 * `release_dmp_stat_file`
 *
 * @reciever - place in memory to store sysfs info to. Should then manually be
 * freed with `release_dmp_stat_file`
 *
 * @stats - user provided place to take stats from. Owned by user and threated as
 * read only
 */
int create_dmp_stat_file(struct sysfs_helper_t* reciever, struct stat_t* stats)
{
  struct device_attribute* attr = kmalloc(sizeof(struct device_attribute), GFP_KERNEL);
  if (NULL == attr)
  {
    return -ENOMEM;
  }
  static DEVICE_ATTR_RO(stats);
  *attr = dev_attr_stats;
  // struct device* module_dev = kobj_to_dev(&MODULE_KOBJ);
  struct kobject* stat_kobj = kobject_create_and_add("stat", &MODULE_KOBJ);
  struct device* raw_dev = kobj_to_dev(stat_kobj);
  if (raw_dev == NULL)
  {
    return -EINVAL;
  }
  device_create_file(raw_dev, attr);
  dev_set_drvdata(raw_dev, stats);
  reciever->raw_device = raw_dev;
  reciever->dev_attr = attr;
  return 0;
}

/**
 * Releases file in sysfs and frees allocated kernel memory,
 * used for `device` and `device_attribute`. It will implicitly
 * invalidate the given pointer to NULL
 *
 * @reciever struct for sysfs which will be invalidated (all fields
 * will be filled with NULL)
 */
int release_dmp_stat_file(struct sysfs_helper_t* reciever)
{
  if (NULL == reciever)
  {
    return -EINVAL;
  }
  device_remove_file(reciever->raw_device, reciever->dev_attr);
  kfree(reciever->dev_attr);
  reciever->dev_attr = NULL;
  kfree(reciever->raw_device);
  reciever->raw_device = NULL;
  return 0;
}
