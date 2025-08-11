#ifndef AWESOME_FOPS_H
#define AWESOME_FOPS_H

#include "awesome_nvme_knl.h"

int awesome_fops_init(struct awesome_nvme_drv_knl *drv);
void awesome_fops_exit(struct awesome_nvme_drv_knl *drv);

#endif /* AWESOME_FOPS_H */