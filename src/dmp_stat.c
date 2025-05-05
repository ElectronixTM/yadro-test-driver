#include "proxy_type.h"
#include <linux/device-mapper.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/device.h>
#include <linux/blk_types.h>

static ssize_t stats_show(struct device* dev, struct device_attribute* attr, char* buf);
int create_dmp_stat_file(struct sysfs_helper_t* reciever)
{
  struct device_attribute* attr = kmalloc(sizeof(struct device_attribute), GFP_KERNEL);
  if (NULL == attr)
  {
    return -ENOMEM;
  }
  static DEVICE_ATTR_RO(stats);
  *attr = dev_attr_stats;
  struct kobject* kobj = kobject_create_and_add("stat", kernel_kobj);
  struct device* raw_dev = kobj_to_dev(kobj);
  if (raw_dev == NULL)
  {
    return -EINVAL;
  }
  reciever->raw_device = raw_dev;
  reciever->dev_attr = attr;
  return 0;
}
