#ifndef AWESOME_NVME_KNL_H
#define AWESOME_NVME_KNL_H

#include <linux/pci.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/idr.h>

#define DRIVER_NAME "awesome_nvme"

struct awesome_nvme_dev {
    struct list_head list;

    /* pcie device */
    struct pci_dev *pdev;

    /* bar map */
    void __iomem *bar;
    resource_size_t bar_size;
    struct cdev bar_cdev;
    struct device *device;
    dev_t devt;

    /* dma */
    void *dma_cpu_addr;
    dma_addr_t dma_phys_addr;
    size_t dma_size;
};

struct awesome_nvme_config {
    struct file_operations *fops;
    struct pci_driver *pci_driver;
    struct class *nvme_class;
    struct ida dev_ida;
    dev_t devt_base;
};

struct awesome_nvme_drv_knl {
    struct awesome_nvme_config cfg;

    /* TODO: 并发probe可能会导致链表错乱 */
    struct list_head dev_list; /* @struct awesome_nvme_dev */
};

struct awesome_nvme_drv_knl *awesome_nvme_get_driver(void);

#endif /* AWESOME_NVME_KNL_H */