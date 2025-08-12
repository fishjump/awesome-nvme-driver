#include <linux/module.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/anon_inodes.h>
#include <linux/uaccess.h>

#include "awesome_nvme_knl.h"

#include "awesome_fops.h"

#define BAR_NAME_FMT DRIVER_NAME"_bar%d"

static int awesome_open(struct inode *inode, struct file *file)
{
    struct awesome_nvme_dev *dev = container_of(inode->i_cdev,
        struct awesome_nvme_dev, cdev.cdev);
    file->private_data = dev;
    pr_info("awesome_open, dev = %p, private_data = %p", dev, file->private_data);
    return 0;
}

static int awesome_release(struct inode *inode, struct file *file)
{
    return 0;
}

static int awesome_bar_mmap(struct file *file, struct vm_area_struct *vma)
{
    struct awesome_nvme_dev *dev = file->private_data;
    if (dev == NULL) {
        pr_err("dev is NULL");
        return -EINVAL;
    }

    unsigned long phys_start = pci_resource_start(dev->pdev, 0);
    unsigned long bar_size = pci_resource_len(dev->pdev, 0);
    unsigned long vsize = vma->vm_end - vma->vm_start;

    if (vsize > bar_size) {
        dev_err(&dev->pdev->dev, "mmap size %ld > bar size %ld",
            vsize, bar_size);
        return -EINVAL;
    }

    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
    if (remap_pfn_range(vma, vma->vm_start,
                        phys_start >> PAGE_SHIFT,
                        vsize, vma->vm_page_prot)) {
        dev_err(&dev->pdev->dev, "remap_pfn_range failed");
        return -EAGAIN;
    }

    return 0;
}

static int awesome_gen_fd(const char *name, struct file_operations *fops,
    struct awesome_nvme_dev *dev)
{
    int fd = anon_inode_getfd(name, fops, dev, O_RDWR);
    if (fd < 0) {
        pr_err("anon_inode_getfd failed, ret = %d", fd);
        return -EAGAIN;
    }

    return fd;
}

static int awesome_get_bar_fd(struct awesome_nvme_dev *dev)
{
    static struct file_operations bar_fops = {
        .owner = THIS_MODULE,
        .mmap = awesome_bar_mmap,
    };

    char name[32];
    int len = snprintf(name, sizeof(name), BAR_NAME_FMT, MINOR(dev->cdev.devt));
    if (len >= sizeof(name)) {
        pr_err("BAR_NAME_FMT is too long, len = %d", len);
        return -EINVAL;
    }

    return awesome_gen_fd(name, &bar_fops, dev);
}

static long awesome_drv_ioctl(struct file *file, unsigned int cmd,
    unsigned long arg)
{
    struct awesome_nvme_dev *dev = file->private_data;
    int ret = 0;
    int fd = 0;
    switch (cmd) {
    case 0:
        fd = awesome_get_bar_fd(dev);
        if (fd < 0) {
            pr_err("awesome_get_bar_fd failed, ret = %d", fd);
            return -EINVAL;
        }
        ret = copy_to_user((void __user *)arg, &fd, sizeof(fd));
        if (ret) {
            pr_err("copy_to_user failed, ret = %d", ret);
            return -EINVAL;
        }
        break;
    default:
        ret = -ENOTTY;
        break;
    }

    return 0;
}

int awesome_fops_init(struct awesome_nvme_drv_knl *drv)
{
    static struct file_operations cdev_fops = {
        .owner = THIS_MODULE,
        .open = awesome_open,
        .release = awesome_release,
        .unlocked_ioctl = awesome_drv_ioctl,
    };

    drv->cfg.cdev_fops = &cdev_fops;
    return 0;
}

void awesome_fops_exit(struct awesome_nvme_drv_knl *drv)
{
    drv->cfg.cdev_fops = NULL;
}
