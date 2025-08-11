#include <linux/kernel.h>

#include "awesome_step_init.h"

int awesome_step_init(struct awesome_init_step *steps, int len,
    struct awesome_nvme_drv_knl *drv)
{
    for (int i = 0; i < len; i++) {
        if (steps[i].init == NULL) {
            continue;
        }
        int ret = steps[i].init(drv);
        if (ret) {
            printk(KERN_ERR "step %d init failed: %d, desc: %s.",
                    i, ret, steps[i].desc);
            awesome_step_exit(steps, i, drv);
            return ret;
        }
    }

    return 0;
}

void awesome_step_exit(struct awesome_init_step *steps, int len,
    struct awesome_nvme_drv_knl *drv)
{
    for (int i = len; i > 0; i--) {
        if (steps[i - 1].exit == NULL) {
            continue;
        }
        steps[i - 1].exit(drv);
    }

}
