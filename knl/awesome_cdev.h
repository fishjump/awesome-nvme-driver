#ifndef AWESOME_CDEV_H
#define AWESOME_CDEV_H

#include "awesome_nvme_knl.h"

int awesome_nvme_cdev_init(struct awesome_nvme_drv_knl *drv);
void awesome_nvme_cdev_exit(struct awesome_nvme_drv_knl *drv);

int awesome_nvme_create_cdev(struct awesome_nvme_dev *dev);
void awesome_nvme_destroy_cdev(struct awesome_nvme_dev *dev);

#endif /* AWESOME_CDEV_H */