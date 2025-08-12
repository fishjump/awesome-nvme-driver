#include <linux/module.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/ioctl.h>

#include "protocol/nvme.h"
#include "awesome_bar.h"
#include "awesome_step_init.h"
#include "awesome_pci.h"
#include "awesome_fops.h"
#include "awesome_cdev.h"

#include "awesome_nvme_knl.h"

#define AWESOME_ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yue Yu");
MODULE_DESCRIPTION("Awesome NVMe driver(kernel), export bar0 and dma address");

static int awesome_nvme_driver_knl_init(struct awesome_nvme_drv_knl *drv)
{
    INIT_LIST_HEAD(&drv->dev_list);
    return 0;
}

static struct awesome_nvme_drv_knl g_awesome_nvme_driver;

static struct awesome_init_step steps[] = {
    {
        .init = awesome_nvme_driver_knl_init,
        .exit = NULL,
        .desc = "init driver",
    },
    {
        .init = awesome_fops_init,
        .exit = awesome_fops_exit,
        .desc = "init file operations",
    },
    {
        .init = awesome_nvme_cdev_init,
        .exit = awesome_nvme_cdev_exit,
        .desc = "init cdev",
    },
    {
        .init = awesome_pci_register_driver,
        .exit = awesome_pci_unregister_driver,
        .desc = "register kernel pci driver",
    },
};

static int __init awesome_nvme_init(void)
{
    return awesome_step_init(steps, AWESOME_ARRAY_LEN(steps),
        &g_awesome_nvme_driver);
}

static void __exit awesome_nvme_exit(void)
{
    awesome_step_exit(steps, AWESOME_ARRAY_LEN(steps), &g_awesome_nvme_driver);
}

struct awesome_nvme_drv_knl *awesome_nvme_get_driver(void)
{
    return &g_awesome_nvme_driver;
}

module_init(awesome_nvme_init);
module_exit(awesome_nvme_exit);
