#ifndef _NFLOG_H
#define _NFLOG_H

#define NFULA_PACKET_HDR		1	/* nflog_packet_hdr_t */
#define NFULA_MARK			2	/* packet mark from skbuff */
#define NFULA_TIMESTAMP			3	/* nflog_timestamp_t for skbuff's time stamp */
#define NFULA_IFINDEX_INDEV		4	/* ifindex of device on which packet received (possibly bridge group) */
#define NFULA_IFINDEX_OUTDEV		5	/* ifindex of device on which packet transmitted (possibly bridge group) */
#define NFULA_IFINDEX_PHYSINDEV		6	/* ifindex of physical device on which packet received (not bridge group) */
#define NFULA_IFINDEX_PHYSOUTDEV	7	/* ifindex of physical device on which packet transmitted (not bridge group) */
#define NFULA_HWADDR			8	/* nflog_hwaddr_t for hardware address */
#define NFULA_PAYLOAD			9	/* packet payload */
#define NFULA_PREFIX			10	/* text string - null-terminated, count includes NUL */
#define NFULA_UID			11	/* UID owning socket on which packet was sent/received */
#define NFULA_SEQ			12	/* sequence number of packets on this NFLOG socket */
#define NFULA_SEQ_GLOBAL		13	/* sequence number of pakets on all NFLOG sockets */
#define NFULA_GID			14	/* GID owning socket on which packet was sent/received */
#define NFULA_HWTYPE			15	/* ARPHRD_ type of skbuff's device */
#define NFULA_HWHEADER			16	/* skbuff's MAC-layer header */
#define NFULA_HWLEN			17	/* length of skbuff's MAC-layer header */

#define NO_NFLOG_ATTRI  18


typedef struct nflog_hdr {
	unsigned char    nflog_family;	/* address family */
	unsigned char		nflog_version;	/* version */
	unsigned short	nflog_rid;	/* resource ID */
} nflog_hdr_t;

typedef struct nflog_tlv {
	unsigned short	tlv_length;	/* tlv length */
	unsigned short	tlv_type;	/* tlv type */
	/* value follows this */
} nflog_tlv_t;

struct nflog{
    nflog_hdr_t hdr;
    nflog_tlv_t tlvs[NO_NFLOG_ATTRI];
};



struct ethernet{
    char dst[6];
    char src[6];
    char type[2];
};



int parser_nflog(const unsigned char *packet, struct nflog *nflog, int *);
#endif