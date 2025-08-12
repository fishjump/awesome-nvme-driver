#ifndef AWESOME_NVME_KNL_H
#define AWESOME_NVME_KNL_H

#include <linux/pci.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/idr.h>
#include <linux/ioctl.h>

#define DRIVER_NAME "awesome_nvme"

struct awesome_nvme_cdev {
    dev_t devt;
    struct cdev cdev;
    struct device *dev;
};

struct awesome_nvme_dev {
    struct list_head list;

    /* pcie device */
    struct pci_dev *pdev;
    struct awesome_nvme_cdev cdev;

    /* bar map */
    int bar_fd;

    /* dma */
    void *dma_cpu_addr;
    dma_addr_t dma_phys_addr;
    size_t dma_size;
    int dma_fd;
};

struct awesome_nvme_class {
    const char *name;
    const char *cdev_fmt;
    struct class *class;
    struct ida ida;
    dev_t devt_base;
};

struct awesome_nvme_config {
    struct pci_driver *pci_driver;
    struct file_operations *cdev_fops;
    struct awesome_nvme_class cdev_class;
};

struct awesome_nvme_drv_knl {
    struct awesome_nvme_config cfg;

    /* TODO: 并发probe可能会导致链表错乱 */
    struct list_head dev_list; /* @struct awesome_nvme_dev */
};

struct awesome_nvme_drv_knl *awesome_nvme_get_driver(void);

#endif /* AWESOME_NVME_KNL_H */