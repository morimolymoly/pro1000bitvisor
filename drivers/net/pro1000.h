#ifndef PRO1000_H
#define PRO1000_H

#include <core.h>
#include <core/initfunc.h>
#include <core/list.h>
#include <core/mmio.h>
#include <net/netapi.h>
#include "../pci.h"
#include "virtio_net.h"


#define BUFSIZE		2048
#define NUM_OF_TDESC	256
#define TDESC_SIZE	((NUM_OF_TDESC) * 16)
#define NUM_OF_TDESC_PAGES (((TDESC_SIZE) + (PAGESIZE - 1)) / PAGESIZE)
#define NUM_OF_RDESC	256
#define RDESC_SIZE	((NUM_OF_RDESC) * 16)
#define NUM_OF_RDESC_PAGES (((RDESC_SIZE) + (PAGESIZE - 1)) / PAGESIZE)
#define TBUF_SIZE	PAGESIZE
#define RBUF_SIZE	PAGESIZE
#define SENDVIRT_MAXSIZE 1514

struct tdesc {
	u64 addr;		/* buffer address */
	uint len : 16;		/* length per segment */
	uint cso : 8;		/* checksum offset */
	uint cmd_eop : 1;	/* command field: end of packet */
	uint cmd_ifcs : 1;	/* command field: insert FCS */
	uint cmd_ic : 1;	/* command field: insert checksum */
	uint cmd_rs : 1;	/* command field: report status */
	uint cmd_rsv : 1;	/* command field: reserved */
	uint cmd_dext : 1;	/* command field: extension */
	uint cmd_vle : 1;	/* command field: VLAN packet enable */
	uint cmd_ide : 1;	/* command field: interrupt delay enable */
	uint sta_dd : 1;	/* status field: descriptor done */
	uint sta_ec : 1;	/* status field: excess collisions */
	uint sta_lc : 1;	/* status field: late collision */
	uint sta_rsv : 1;	/* status field: reserved */
	uint reserved : 4;	/* reserved */
	uint css : 8;		/* checksum start */
	uint special : 16;	/* special field */
} __attribute__ ((packed));

struct tdesc_dext0 {
	uint ipcss : 8;		/* IP checksum start */
	uint ipcso : 8;		/* IP checksum offset */
	uint ipcse : 16;	/* IP checksum ending */
	uint tucss : 8;		/* TCP/UDP checksum start */
	uint tucso : 8;		/* TCP/UDP checksum offset */
	uint tucse : 16;	/* TCP/UDP checksum ending */
	uint paylen : 20;	/* packet length field */
	uint dtyp : 4;		/* descriptor type */
	uint tucmd_tcp : 1;	/* TCP/UDP command field: packet type */
	uint tucmd_ip : 1;	/* TCP/UDP command field: packet type */
	uint tucmd_tse : 1;	/* TCP/UDP command field: */
				/*  TCP segmentation enable */
	uint tucmd_rs : 1;	/* TCP/UDP command field: report status */
	uint tucmd_rsv : 1;	/* TCP/UDP command field: reserved */
	uint tucmd_dext : 1;	/* TCP/UDP command field: */
				/*  descriptor extension */
	uint tucmd_snap : 1;	/* TCP/UDP command field: what's this? */
	uint tucmd_ide : 1;	/* TCP/UDP command field: */
				/*  interrupt delay enable */
	uint sta_dd : 1;	/* TCP/UDP status field: descriptor done */
	uint sta_rsv : 3;	/* TCP/UDP status field: reserved */
	uint rsv : 4;		/* reserved */
	uint hdrlen : 8;	/* header length */
	uint mss : 16;		/* maximum segment size */
} __attribute__ ((packed));

struct tdesc_dext1 {
	u64 addr;		/* data buffer address */
	uint dtalen : 20;	/* data length field */
	uint dtyp : 4;		/* data type (descriptor type) */
	uint dcmd_eop : 1;	/* descriptor command field: end of packet */
	uint dcmd_ifcs : 1;	/* descriptor command field: insert FCS */
	uint dcmd_tse : 1;	/* descriptor command field: */
				/*  TCP segmentation enable */
	uint dcmd_rs : 1;	/* descriptor command field: report status */
	uint dcmd_rsv : 1;	/* descriptor command field: reserved */
	uint dcmd_dext : 1;	/* descriptor command field: */
				/*  descriptor extension */
	uint dcmd_vle : 1;	/* descriptor command field: VLAN enable */
	uint dcmd_ide : 1;	/* descriptor command field: */
				/*  interrupt delay enable */
	uint sta_dd : 1;	/* TCP/IP status field: descriptor done */
	uint sta_rsv1 : 1;	/* TCP/IP status field: reserved */
	uint sta_rsv2 : 1;	/* TCP/IP status field: reserved */
	uint sta_rsv3 : 1;	/* TCP/IP status field: reserved */
	uint rsv : 4;		/* reserved */
	uint popts_ixsm : 1;	/* packet option field: insert IP checksum */
	uint popts_txsm : 1;	/* packet option field: */
				/*  insert TCP/UDP checksum */
	uint popts_rsv : 6;	/* packet option field: reserved */
	uint vlan : 16;		/* VLAN field */
} __attribute__ ((packed));

struct rdesc {
	u64 addr;		/* buffer address */
	uint len : 16;		/* length */
	uint checksum : 16;	/* packet checksum */
	uint status_dd : 1;	/* status field: descriptor done */
	uint status_eop : 1;	/* status field: end of packet */
	uint status_ixsm : 1;	/* status field: ignore checksum indication */
	uint status_vp : 1;	/* status field: packet is 802.1Q */
	uint status_udpcs : 1;	/* status field: UDP checksum calculated */
				/* on packet */
	uint status_tcpcs : 1;	/* status field: TCP checksum calculated */
				/* on packet */
	uint status_ipcs : 1;	/* status field: IPv4 checksum calculated */
				/* on packet */
	uint status_pif : 1;	/* status field: passed in-exact filter */
	uint err_ce : 1;	/* errors field: CRC error */
				/* or alignment error */
	uint err_se : 1;	/* errors field: symbol error */
	uint err_seq : 1;	/* errors field: sequence error */
	uint err_rcv : 2;	/* errors field: reserved */
	uint err_tcpe : 1;	/* errors field: TCP/UDP checksum error */
	uint err_ipe : 1;	/* errors field: IPv4 checksum error */
	uint err_rxe : 1;	/* errors field: RX data error */
	uint vlantag : 16;	/* VLAN Tag */
} __attribute__ ((packed));

struct rdesc_ext1 {
	uint mrq : 32;		/* MRQ */
	uint rsshash : 32;	/* RSS hash */
	uint ex_sta : 20;	/* extended status */
	uint ex_err : 12;	/* extended error */
	uint len : 16;		/* length */
	uint vlantag : 16;	/* VLAN tag */
} __attribute__ ((packed));

struct desc_shadow {
	bool initialized;
	union {
		u64 ll;
		u32 l[2];
	} base;
	u32 len;
	u32 head, tail;
	union {
		struct {
			struct tdesc *td;
			phys_t td_phys;
			void *tbuf[NUM_OF_TDESC];
		} t;
		struct {
			struct rdesc *rd;
			phys_t rd_phys;
			void *rbuf[NUM_OF_RDESC];
			long rbuf_premap[NUM_OF_RDESC];
		} r;
	} u;
};

struct data;

struct data2 {
	spinlock_t lock;
	u8 *buf;
	long buf_premap;
	uint len;
	bool dext1_ixsm, dext1_txsm;
	uint dext0_tucss, dext0_tucso, dext0_tucse;
	uint dext0_ipcss, dext0_ipcso, dext0_ipcse;
	uint dext0_mss, dext0_hdrlen, dext0_paylen, dext0_ip, dext0_tcp;
	bool tse_first, tse_tcpfin, tse_tcppsh;
	u16 tse_iplen, tse_ipchecksum, tse_tcpchecksum;
	struct desc_shadow tdesc[2], rdesc[2];
	struct data *d1;
	struct netdata *nethandle;
	bool initialized;
	net_recv_callback_t *recvphys_func, *recvvirt_func;
	void *recvphys_param, *recvvirt_param;
	u32 rctl, rfctl, tctl;
	u8 macaddr[6];
	struct pci_device *pci_device;
	u32 regs_at_init[PCI_CONFIG_REGS32_NUM];
	bool seize;
	bool conceal;
	LIST1_DEFINE (struct data2);

	void *virtio_net;
	char virtio_net_bar_emul;
	struct pci_msi *virtio_net_msi;
};

struct data {
	int i;
	int e;
	int io;
	int hd;
	bool disable;
	void *h;
	void *map;
	uint maplen;
	phys_t mapaddr;
	struct data2 *d;
};

struct data2* dp_1000;

#endif