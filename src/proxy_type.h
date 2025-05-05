#ifndef PROXY_TYPE_H
#define PROXY_TYPE_H

#include <linux/types.h>

/**
 * This struct is needed to provide sysfs file with
 * statistics. it contains device and attribute.
 *
 * device struct should be constructed on top of dm_dev
 * of proxied device
 */
struct sysfs_helper_t
{
  struct device* raw_device; /**< device, needed to track sysfs files */
  struct device_attribute* dev_attr; /**< attribute of the proxy in sysfs */
};

/**
 * Struct to store statistics of the proxy. Considered a private field of the
 * device struct from sysfs_helper_t
 */
struct stat_t
{
  size_t read_rq_num; /**< total number of read requests on the device */
  size_t write_rq_num; /**< total number of write requests on the device */
  size_t total_read; /**< total bytes read via bios */
  size_t total_write; /**< total bytes written to device */
};

/**
 * The struct, containing minimum required info to provide to sysfs, such
 * as amount of read and write requests and total sizes of all blocks, from
 * requests
 */
struct proxy_t
{
  struct dm_dev* dev; /**< device to forward all bios to */
  struct sysfs_helper_t sysfs;
};

#endif
