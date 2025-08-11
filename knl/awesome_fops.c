#include <linux/module.h>
#include <linux/fs.h>

#include "awesome_fops.h"

static struct file_operations g_awesome_fops = {
    .owner = THIS_MODULE,
};

int awesome_fops_init(struct awesome_nvme_drv_knl *drv)
{
    drv->cfg.fops = &g_awesome_fops;
    return 0;
}

void awesome_fops_exit(struct awesome_nvme_drv_knl *drv)
{
    drv->cfg.fops = NULL;
}
