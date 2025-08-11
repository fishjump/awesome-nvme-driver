#ifndef AWESOME_BAR_H
#define AWESOME_BAR_H

#include "awesome_nvme_knl.h"

// 映射BAR空间
int awesome_nvme_bar_map(struct awesome_nvme_dev *nvme_dev);

// 解除BAR映射
void awesome_nvme_bar_unmap(struct awesome_nvme_dev *nvme_dev);

#endif /* AWESOME_BAR_H */
