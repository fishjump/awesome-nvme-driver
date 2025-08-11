#ifndef AWESOME_PCI_H
#define AWESOME_PCI_H

#include "awesome_nvme_knl.h"

int  awesome_pci_register_driver(struct awesome_nvme_drv_knl *drv);
void awesome_pci_unregister_driver(struct awesome_nvme_drv_knl *drv);

#endif /* AWESOME_PCI_H */