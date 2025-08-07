#ifndef AWESOME_NVME_H
#define AWESOME_NVME_H

#include <linux/types.h>

struct nvme_sqe {
    uint8_t  opcode;
    uint8_t  fuse  : 2;
    uint8_t  rsvd1 : 6;
    uint16_t cid;
    uint32_t nsid;
    uint64_t rsvd2;
    uint64_t mptr;
    union {
        struct {
            uint64_t prp1;
            uint64_t prp2;
        } prp;
        struct {
            uint64_t sgl1;
            uint64_t sgl2;
        } sgl;
    } dptr;
    uint32_t cdw10;
    uint32_t cdw11;
    uint32_t cdw12;
    uint32_t cdw13;
    uint32_t cdw14;
    uint32_t cdw15;
} __attribute__((packed));

struct nvme_cqe {
    uint32_t dw0;
    uint32_t dw1;
    uint16_t sq_head;
    uint16_t sq_id;
    uint16_t cid;
    uint16_t status;
} __attribute__((packed));

struct nvme_identify_controller {
    uint16_t pci_vendor_id;
    uint16_t pci_subsystem_vendor_id;
    uint8_t  sn[20];
    uint8_t  model[40];
} __attribute__((packed));

#endif /* AWESOME_NVME_H */