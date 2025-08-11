#include <linux/cdev.h>
#include <linux/device.h>

#include "awesome_cdev.h"

#define DEVICE_NODE_NAME DRIVER_NAME"%d"
#define DRIVER_MAX_DEV_CNT 256

int awesome_nvme_cdev_init(struct awesome_nvme_drv_knl *drv)
{
    int ret = alloc_chrdev_region(&drv->cfg.devt_base, 0,
        DRIVER_MAX_DEV_CNT, DRIVER_NAME);
    if (ret) {
        goto err_return;
    }

    drv->cfg.nvme_class = class_create(DRIVER_NAME);
    if (IS_ERR(drv->cfg.nvme_class)) {
        ret = PTR_ERR(drv->cfg.nvme_class);
        goto err_del_chrdev_region;
    }

    ida_init(&drv->cfg.dev_ida);
    return 0;

err_del_chrdev_region:
    unregister_chrdev_region(drv->cfg.devt_base, DRIVER_MAX_DEV_CNT);
err_return:
    return ret;
}

void awesome_nvme_cdev_exit(struct awesome_nvme_drv_knl *drv)
{
    /* init成功代表此处所有资源一定初始化，否则ko会加载失败；
     * 所以此处无需检查是否初始化 */
    class_destroy(drv->cfg.nvme_class);
    unregister_chrdev_region(drv->cfg.devt_base, DRIVER_MAX_DEV_CNT);
    ida_destroy(&drv->cfg.dev_ida);
}

int awesome_nvme_create_cdev(struct awesome_nvme_dev *dev)
{
    struct awesome_nvme_drv_knl *drv = awesome_nvme_get_driver();
    int ret = 0;

    /* 初始化字符设备结构体 */
    cdev_init(&dev->bar_cdev, drv->cfg.fops);
    dev->bar_cdev.owner = THIS_MODULE;

    int minor = ida_alloc_max(&drv->cfg.dev_ida, DRIVER_MAX_DEV_CNT, GFP_KERNEL);
    if (minor < 0) {
        dev_err(&dev->pdev->dev, "nvme alloc minor id failed: %d.", minor);
        ret = -EINVAL;
        goto err;
    }
    dev->devt = MKDEV(MAJOR(drv->cfg.devt_base), minor);

    /* 注册字符设备 */
    ret = cdev_add(&dev->bar_cdev, dev->devt, 1);
    if (ret) {
        dev_err(&dev->pdev->dev, "nvme create cdev failed: %d.", ret);
        goto err_release_minor;
    }

    dev->device = device_create(drv->cfg.nvme_class,        /* 所属 class */
                                NULL,                       /* 父设备 */
                                dev->devt,                  /* 设备号 */
                                NULL,                       /* drvdata */
                                DEVICE_NODE_NAME,           /* 节点名格式 */
                                MINOR(dev->devt));          /* 节点编号 */
    if (IS_ERR(dev->device)) {
        ret = PTR_ERR(dev->device);
        dev_err(&dev->pdev->dev, "nvme create device failed: %d.", ret);
        goto err_cdev_del;
    }

    return 0;

err_cdev_del:
    cdev_del(&dev->bar_cdev);
err_release_minor:
    ida_free(&drv->cfg.dev_ida, minor);
err:
    return ret;
}

void awesome_nvme_destroy_cdev(struct awesome_nvme_dev *dev)
{
    if (dev->device) {
        device_destroy(class_create(DRIVER_NAME), dev->devt);
    }
    if (dev->devt) {
        unregister_chrdev_region(dev->devt, 1);
    }
    if (dev->bar_cdev.dev) {
        cdev_del(&dev->bar_cdev);
    }
}
