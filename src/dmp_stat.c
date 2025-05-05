#include "proxy_type.h"
#include <linux/device-mapper.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/device.h>
#include <linux/blk_types.h>
#include <linux/module.h>

#define MODULE_KOBJ ((((struct module*) THIS_MODULE)->mkobj).kobj)

static ssize_t stats_show(struct device* dev, struct device_attribute* attr, char* buf)
{
  struct stat_t* stats = (struct stat_t*) dev_get_drvdata(dev);
  if (NULL == stats)
  {
    return -EINVAL;
  }
  ssize_t total_out = 0;
  total_out += sprintf(buf, "read:\n");
  total_out += sprintf(buf, " reqs: %lu\n", stats->read_rq_num);
  total_out += sprintf(
      buf, " avg size: %lu\n",
      (stats->total_read/stats->read_rq_num)
      );
  total_out += sprintf(buf, "write:\n");
  total_out += sprintf(buf, " reqs: %lu\n", stats->write_rq_num);
  total_out += sprintf(
      buf, " avg size: %lu\n",
      (stats->total_write/stats->write_rq_num)
      );
  total_out += sprintf(buf, "total:\n");
  total_out += sprintf(
      buf, " reqs: %lu\n",
      stats->read_rq_num + stats->write_rq_num
      );
  total_out += sprintf(
      buf, " avg size: %lu\n",
      (stats->total_read+stats->total_write)/(stats->read_rq_num+stats->write_rq_num)
      );
  return total_out;
}

int create_dmp_stat_file(struct sysfs_helper_t* reciever, struct stat_t* stats)
{
  struct device_attribute* attr = kmalloc(sizeof(struct device_attribute), GFP_KERNEL);
  if (NULL == attr)
  {
    return -ENOMEM;
  }
  static DEVICE_ATTR_RO(stats);
  *attr = dev_attr_stats;
  struct device* module_dev = kobj_to_dev(&MODULE_KOBJ);
  if (module_dev == NULL)
  {
    return -EINVAL;
  }
  device_create_file(module_dev, attr);
  dev_set_drvdata(module_dev, stats);
  reciever->raw_device = module_dev;
  reciever->dev_attr = attr;
  return 0;
}

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
