#ifndef DMP_STAT_H
#define DMP_STAT_H

/**
 * Creates file in sysfs and initializes `sysfs` member
 * of the proxy_t struct. after file is not needed, it
 * should be freed with `release_dmp_stat_file`
 */
int create_dmp_stat_file(struct sysfs_helper_t* reciever);

/**
 * Releases file and set zeros in `sysfs` member of
 * proxy_t
 */
int release_dmp_stat_file(struct sysfs_helper_t* reciever);

#endif

