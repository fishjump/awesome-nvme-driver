#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#define IOCTL_GET_BAR_FD 0

int main()
{
    int dev_fd, bar_fd;
    void *bar_mem;
    size_t bar_size = 0x1000; // 假设BAR大小1KB，实际大小应由驱动告知或约定
    off_t offset = 0;          // mmap 偏移，通常0

    dev_fd = open("/dev/awesome_nvme0", O_RDWR);
    if (dev_fd < 0) {
        perror("open");
        return 1;
    }

    // ioctl返回bar空间的fd（假设传入int*）
    if (ioctl(dev_fd, IOCTL_GET_BAR_FD, &bar_fd) < 0) {
        perror("ioctl");
        close(dev_fd);
        return 1;
    }

    printf("Got BAR fd = %d\n", bar_fd);

    // mmap映射bar fd
    bar_mem = mmap(NULL, bar_size, PROT_READ | PROT_WRITE, MAP_SHARED, bar_fd, offset);
    if (bar_mem == MAP_FAILED) {
        perror("mmap");
        close(bar_fd);
        close(dev_fd);
        return 1;
    }

    printf("BAR memory mapped at %p\n", bar_mem);

    volatile uint32_t *cc = (uint32_t *)((uint8_t *)bar_mem + 0x14);    // CC寄存器
    volatile uint32_t *csts = (uint32_t *)((uint8_t *)bar_mem + 0x1C);  // CSTS寄存器

    // 5.1 复位控制器
    uint32_t cc_val = *cc;
    uint32_t csts_val = *csts;
    *cc = 0x0;
    while (csts_val & 0x1) {
        printf("wait cc disable, csts: 0x%x.\n", csts_val);
        csts_val = *csts;
    }
    printf("cc disable, csts: 0x%x.\n", csts_val);

    // 用完释放
    munmap(bar_mem, bar_size);
    close(bar_fd);
    close(dev_fd);

    return 0;
}
