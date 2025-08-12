#include <linux/cdev.h>
#include <linux/device.h>

#include "awesome_cdev.h"

#define DRV_NODE_NAME DRIVER_NAME
#define DRV_NODE_NAME_FMT DRIVER_NAME"%d"

#define DRIVER_MAX_DEV_CNT 256

static int awesome_nvme_class_init(struct awesome_nvme_class *cls,
    const char *name, const char *cdev_fmt)
{
    int ret = alloc_chrdev_region(&cls->devt_base, 0, DRIVER_MAX_DEV_CNT, name);
    if (ret) {
        goto err;
    }

    cls->class = class_create(name);
    if (IS_ERR(cls->class)) {
        ret = PTR_ERR(cls->class);
        goto err_del_chrdev_region;
    }

    ida_init(&cls->ida);

    cls->name = name;
    cls->cdev_fmt = cdev_fmt;

    return 0;

err_del_chrdev_region:
    unregister_chrdev_region(cls->devt_base, DRIVER_MAX_DEV_CNT);
err:
    return ret;
}

static void awesome_nvme_class_exit(struct awesome_nvme_class *cls)
{
    ida_destroy(&cls->ida);
    class_destroy(cls->class);
    unregister_chrdev_region(cls->devt_base, DRIVER_MAX_DEV_CNT);
}

int awesome_nvme_cdev_init(struct awesome_nvme_drv_knl *drv)
{
    return awesome_nvme_class_init(&drv->cfg.cdev_class,
        DRV_NODE_NAME, DRV_NODE_NAME_FMT);
}

void awesome_nvme_cdev_exit(struct awesome_nvme_drv_knl *drv)
{
    /* init成功代表此处所有资源一定初始化，否则ko会加载失败；
     * 所以此处无需检查是否初始化 */
    awesome_nvme_class_exit(&drv->cfg.cdev_class);
}

int awesome_nvme_create_cdev(struct awesome_nvme_cdev *cdev,
    struct awesome_nvme_class *cls, struct file_operations *fops)
{
    /* 初始化字符设备结构体 */
    cdev_init(&cdev->cdev, fops);
    cdev->cdev.owner = THIS_MODULE;

    int ret = 0;
    int minor = ida_alloc_max(&cls->ida, DRIVER_MAX_DEV_CNT, GFP_KERNEL);
    if (minor < 0) {
        pr_err("nvme alloc minor id failed: %d.", minor);
        ret = -EINVAL;
        goto err;
    }

    cdev->devt = MKDEV(MAJOR(cls->devt_base), minor);
    ret = cdev_add(&cdev->cdev, cdev->devt, 1);
    if (ret) {
        pr_err("nvme create cdev failed: %d.", ret);
        goto err_ida_free;
    }

    cdev->dev = device_create(cls->class, NULL, cdev->devt,
        NULL, cls->cdev_fmt, MINOR(cdev->devt));
    if (IS_ERR(cdev->dev)) {
        ret = PTR_ERR(cdev->dev);
        pr_err("nvme create device failed: %d.", ret);
        goto err_cdev_del;
    }

    return 0;

err_cdev_del:
    cdev_del(&cdev->cdev);
err_ida_free:
    ida_free(&cls->ida, minor);
err:
    return ret;
}

void awesome_nvme_destroy_cdev(struct awesome_nvme_cdev *cdev,
    struct awesome_nvme_class *cls)
{
    device_destroy(cls->class, cdev->devt);
    cdev_del(&cdev->cdev);
    ida_free(&cls->ida, MINOR(cdev->devt));
}
