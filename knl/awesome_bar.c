#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>

#include "awesome_bar.h"

// 映射BAR空间
int awesome_nvme_bar_map(struct awesome_nvme_dev *dev)
{
    int ret = 0;

//     /* 映射BAR0空间到内存 */
//     dev->bar_size = pci_resource_len(dev->pdev, 0);
//     dev->bar = pci_iomap(dev->pdev, 0, dev->bar_size);
//     if (dev->bar == NULL) {
//         dev_err(&dev->pdev->dev, "pci iomap failed.");
//         ret = -ENOMEM;
//         goto err_unmap_bar;
//     }

//     return 0;

// err_unmap_bar:
//     pci_iounmap(dev->pdev, dev->bar);
    return ret;
}

// 解除BAR映射
void awesome_nvme_bar_unmap(struct awesome_nvme_dev *dev)
{
    // pci_iounmap(dev->pdev, dev->bar);
}