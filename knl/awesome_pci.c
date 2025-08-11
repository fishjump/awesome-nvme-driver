#include <linux/pci.h>

#include "awesome_bar.h"
#include "awesome_cdev.h"

#include "awesome_pci.h"

static struct pci_device_id awesome_nvme_ids[] = {
    { PCI_DEVICE(0x15ad, 0x07f0) }, // VMware NVMe Controller
    { 0 }
};
MODULE_DEVICE_TABLE(pci, awesome_nvme_ids);

static struct awesome_nvme_dev *awesome_pci_create_nvme_device(
    struct awesome_nvme_drv_knl *drv, struct pci_dev *pdev)
{
    struct awesome_nvme_dev *dev = kzalloc(sizeof(struct awesome_nvme_dev),
        GFP_KERNEL);
    if (dev == NULL) {
        return NULL;
    }

    dev->pdev = pdev;
    INIT_LIST_HEAD(&dev->list);
    return dev;
}

static void awesome_pci_destroy_nvme_device(struct awesome_nvme_dev *dev)
{
    list_del(&dev->list);
    kfree(dev);
}

static int awesome_nvme_probe(struct pci_dev *pdev,
        const struct pci_device_id *id)
{
    /* 用户态驱动下，内核态仅作内存映射。内核态初始化流程如下：
     * 1. 为每个设备初始化struct nvme_device
     * 2. 映射BAR0空间到内存
     * 3. 创建字符设备/dev/nvme_bar0_<dev_id>
     */

    struct awesome_nvme_drv_knl *drv = awesome_nvme_get_driver();

    struct awesome_nvme_dev *dev = awesome_pci_create_nvme_device(drv, pdev);
    if (dev == NULL) {
        return -ENOMEM;
    }

    int ret = awesome_nvme_bar_map(dev);
    if (ret) {
        dev_err(&dev->pdev->dev, "nvme bar map failed: %d.", ret);
        goto err_destroy_device;
    }

    ret = awesome_nvme_create_cdev(dev);
    if (ret) {
        dev_err(&dev->pdev->dev, "nvme create cdev failed: %d.", ret);
        goto err_bar_unmap;
    }

    pci_set_drvdata(pdev, dev);
    list_add_tail(&dev->list, &drv->dev_list);
    return 0;

err_bar_unmap:
    awesome_nvme_bar_unmap(dev);

err_destroy_device:
    awesome_pci_destroy_nvme_device(dev);
    return ret;
}

static void awesome_nvme_remove(struct pci_dev *pdev)
{
    struct awesome_nvme_dev *dev = pci_get_drvdata(pdev);
    awesome_nvme_destroy_cdev(dev);
    awesome_nvme_bar_unmap(dev);
    awesome_pci_destroy_nvme_device(dev);
}

static struct pci_driver awesome_nvme_pci_driver = {
    .name = DRIVER_NAME,
    .id_table = awesome_nvme_ids,
    .probe = awesome_nvme_probe,
    .remove = awesome_nvme_remove,
};

int awesome_pci_register_driver(struct awesome_nvme_drv_knl *drv)
{
    drv->cfg.pci_driver = &awesome_nvme_pci_driver;
    return pci_register_driver(drv->cfg.pci_driver);
}

void awesome_pci_unregister_driver(struct awesome_nvme_drv_knl *drv)
{
    pci_unregister_driver(drv->cfg.pci_driver);
    drv->cfg.pci_driver = NULL;
}

// static irqreturn_t nvme_stub_irq_handler(int irq, void *dev_id) {
//     return IRQ_HANDLED;
// }

// struct awesome_nvme_drv_knl g_nvme_driver;

// static int awesome_nvme_probe(struct pci_dev *pdev, const struct pci_device_id *id)
// {
//     int ret;

//     // 1. 分配设备结构体
//     g_nvme_driver.dev[0] = devm_kzalloc(&pdev->dev,
//         sizeof(struct awesome_nvme_dev), GFP_KERNEL);
//     if (g_nvme_driver.dev[0] == NULL) {
//         return -ENOMEM;
//     }
//     g_nvme_driver.dev[0]->id = 0;
//     g_nvme_driver.dev[0]->pdev = pdev;


//     ret = awesome_nvme_bar_map(g_nvme_driver.dev[0]);
//     if (ret) {
//         dev_err(&g_nvme_driver.dev[0]->pdev->dev,
//                 "nvme bar map failed: %d.", ret);
//         return ret;
//     }

//     pci_set_drvdata(pdev, g_nvme_driver.dev[0]);
//     return 0;
// //     stub_dev->pdev = pdev;

// //     // 2. 使能PCI设备
// //     ret = pci_enable_device(pdev);
// //     if (ret) {
// //         dev_err(&pdev->dev, "pci enable device failed: %d.", ret);
// //         return ret;
// //     }
// //     pci_set_master(pdev);

// //     ret = devm_request_irq(&pdev->dev, pdev->irq,
// //                           nvme_stub_irq_handler,
// //                           IRQF_SHARED, "nvme_stub", stub_dev);
// //     if (ret) {
// //         dev_err(&pdev->dev, "request irq failed: %d.", ret);
// //         goto err_disable_device;
// //     }

// //     // 3. 分配DMA内存(64KB)
// //     stub_dev->dma_size = 4096 * 16;
// //     stub_dev->dma_cpu_addr = dma_alloc_coherent(
// //         &pdev->dev, stub_dev->dma_size, &stub_dev->dma_phys_addr, GFP_KERNEL);
// //     if (!stub_dev->dma_cpu_addr) {
// //         dev_err(&pdev->dev, "dma alloc coherent failed.");
// //         ret = -ENOMEM;
// //         goto err_disable_device;
// //     }

// //     // 4. 映射BAR0区域
// //     stub_dev->bar0_len = pci_resource_len(pdev, 0);
// //     stub_dev->bar0 = pci_iomap(pdev, 0, stub_dev->bar0_len);
// //     if (!stub_dev->bar0) {
// //         dev_err(&pdev->dev, "bar0 map failed.");
// //         ret = -ENOMEM;
// //         goto err_free_dma;
// //     }

// //     // 5. 控制器初始化
// //     volatile uint32_t *cc = (uint32_t *)((uint8_t *)stub_dev->bar0 + 0x14);    // CC寄存器
// //     volatile uint32_t *csts = (uint32_t *)((uint8_t *)stub_dev->bar0 + 0x1C);  // CSTS寄存器

// //     // 5.1 复位控制器
// //     writel(0x0, cc);  // 禁用控制器
// //     while (readl(csts) & 0x1) {
// //         dev_info(&pdev->dev, "wait cc disable, csts: 0x%x.", readl(csts));
// //     }

// //     // 5.2 配置Admin队列
// //     volatile uint32_t *aqa = (uint32_t *)((uint8_t *)stub_dev->bar0 + 0x24);
// //     volatile uint32_t *asq_lo = (uint32_t *)((uint8_t *)stub_dev->bar0 + 0x28);
// //     volatile uint32_t *asq_hi = (uint32_t *)((uint8_t *)stub_dev->bar0 + 0x2C);
// //     volatile uint32_t *acq_lo = (uint32_t *)((uint8_t *)stub_dev->bar0 + 0x30);
// //     volatile uint32_t *acq_hi = (uint32_t *)((uint8_t *)stub_dev->bar0 + 0x34);
// //     writel((63 << 16) | 63, aqa);  // SQ/CQ大小均为64项
// //     writel(stub_dev->dma_phys_addr, asq_lo);               // SQ物理地址(低32位)
// //     writel(stub_dev->dma_phys_addr >> 32, asq_hi);     // SQ物理地址(高32位)
// //     writel(stub_dev->dma_phys_addr + 0x1000, acq_lo);      // CQ物理地址(低32位)
// //     writel((stub_dev->dma_phys_addr + 0x1000) >> 32, acq_hi);  // CQ物理地址(高32位)

// //     // 5.3 启用控制器
// //     writel(0x1, cc);  // 设置EN位
// //     while (!(readl(csts) & 0x1)) {  // 等待RDY位
// //         dev_info(&pdev->dev, "wait cc enable, csts: 0x%x.", readl(csts));
// //     }

// //     // 6. 发送Identify Controller命令
// //     struct nvme_sqe *sqe = (struct nvme_sqe *)stub_dev->dma_cpu_addr;
// //     memset(sqe, 0, sizeof(*sqe));
// //     sqe->opcode = 0x06;          // Identify命令
// //     sqe->cid = 1;                // 命令ID
// //     sqe->nsid = 0;               // 控制器识别
// //     sqe->cdw10 = 1;              // 识别控制器
// //     sqe->dptr.prp.prp1 = stub_dev->dma_phys_addr + 0x2000;  // 数据缓冲区地址

// //     // 7. 提交命令(写入SQ门铃)
// //     volatile uint32_t *sq_doorbell = (uint32_t *)((uint8_t *)stub_dev->bar0 + 0x1000);  // SQ门铃寄存器
// //     volatile uint32_t *cq_doorbell = (uint32_t *)((uint8_t *)stub_dev->bar0 + 0x1004);  // CQ门铃寄存器
// //     writel(1, sq_doorbell);  // 提交1个命令

// //     // 8. 轮询CQ结果
// //     struct nvme_cqe *cqe = (struct nvme_cqe *)(stub_dev->dma_cpu_addr + 0x1000);
// //     int timeout = 0;
// //     while (!(readl(&cqe->status) & 0x1)) {  // 等待相位位翻转
// //         if (timeout++ > UINT_MAX) {
// //             dev_err(&pdev->dev, "identify command timeout.");
// //             ret = -ETIMEDOUT;
// //             goto err_unmap_bar;
// //         }
// //     }

// //     // 9. 验证命令结果
// //     if ((readl(&cqe->status) >> 1) != 0) {
// //         dev_err(&pdev->dev, "identify command failed: 0x%x", readl(&cqe->status) >> 1);
// //         ret = -EIO;
// //         writel(1, cq_doorbell);
// //         goto err_unmap_bar;
// //     }
// //     struct nvme_identify_controller *identify_controller =
// //         (struct nvme_identify_controller *)((uint8_t *)stub_dev->dma_cpu_addr + 0x2000);
// //     dev_info(&pdev->dev, "pci_vendor_id: 0x%x", identify_controller->pci_vendor_id);
// //     dev_info(&pdev->dev, "pci_subsystem_vendor_id: 0x%x", identify_controller->pci_subsystem_vendor_id);
// //     dev_info(&pdev->dev, "sn: %s", identify_controller->sn);
// //     dev_info(&pdev->dev, "model: %s", identify_controller->model);
// //     dev_info(&pdev->dev, "identify command success.");
// //     writel(1, cq_doorbell);

// //     pci_set_drvdata(pdev, stub_dev);
// //     dev_info(&pdev->dev, "nvme controller initialize success, bar0 size: %pa, dma size: %zu.",
// //              &stub_dev->bar0_len, stub_dev->dma_size);
// //     return 0;

// // err_unmap_bar:
// //     pci_iounmap(pdev, stub_dev->bar0);
// // err_free_dma:
// //     dma_free_coherent(&pdev->dev, stub_dev->dma_size,
// //                       stub_dev->dma_cpu_addr, stub_dev->dma_phys_addr);
// // err_disable_device:
// //     pci_disable_device(pdev);
// //     return ret;
// }

// static void awesome_nvme_remove(struct pci_dev *pdev)
// {
//     pci_iounmap(pdev, stub_dev->bar0);
//     dma_free_coherent(&pdev->dev, stub_dev->dma_size,
//                       stub_dev->dma_cpu_addr, stub_dev->dma_phys_addr);
//     pci_disable_device(pdev);
// }