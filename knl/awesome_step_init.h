#ifndef AWESOME_STEP_INIT_H
#define AWESOME_STEP_INIT_H

#include "awesome_nvme_knl.h"

struct awesome_init_step {
    int (*init)(struct awesome_nvme_drv_knl *drv);
    void (*exit)(struct awesome_nvme_drv_knl *drv);
    const char *desc;
};

int  awesome_step_init(struct awesome_init_step *steps, int len,
    struct awesome_nvme_drv_knl *drv);

void awesome_step_exit(struct awesome_init_step *steps, int len,
    struct awesome_nvme_drv_knl *drv);

#endif /* AWESOME_STEP_INIT_H */