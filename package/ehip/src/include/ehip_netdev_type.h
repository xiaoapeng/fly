/**
 * @file ehip_netdev_type.h
 * @brief  网络设备类型
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-11-04
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */
#ifndef _EHIP_NETDEV_TYPE_H_
#define _EHIP_NETDEV_TYPE_H_


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/* ARP protocol HARDWARE identifiers. */
#define EHIP_ARP_NETROM	0		                /* from KA9Q: NET/ROM pseudo	*/
#define EHIP_ARP_ETHER 	1		                /* Ethernet 10Mbps		*/
#define	EHIP_ARP_EETHER	2		                /* Experimental Ethernet	*/
#define	EHIP_ARP_AX25	3		                /* AX.25 Level 2		*/
#define	EHIP_ARP_PRONET	4		                /* PROnet token ring		*/
#define	EHIP_ARP_CHAOS	5		                /* Chaosnet			*/
#define	EHIP_ARP_IEEE802	6		            /* IEEE 802.2 Ethernet/TR/TB	*/
#define	EHIP_ARP_ARCNET	7		                /* ARCnet			*/
#define	EHIP_ARP_APPLETLK	8		            /* APPLEtalk			*/
#define EHIP_ARP_DLCI	15		                /* Frame Relay DLCI		*/
#define EHIP_ARP_ATM	19		                /* ATM 				*/
#define EHIP_ARP_METRICOM	23		            /* Metricom STRIP (new IANA id)	*/
#define	EHIP_ARP_IEEE1394	24		            /* IEEE 1394 IPv4 - RFC 2734	*/
#define EHIP_ARP_EUI64	27		                /* EUI-64                       */
#define EHIP_ARP_INFINIBAND 32		            /* InfiniBand			*/



enum ehip_netdev_type{
    EHIP_NETDEV_TYPE_ETHERNET = EHIP_ARP_ETHER,
    EHIP_NETDEV_TYPE_DUMMY_START = 33,
    EHIP_NETDEV_TYPE_PPP = EHIP_NETDEV_TYPE_DUMMY_START,
    EHIP_NETDEV_TYPE_LOOPBACK,
    EHIP_NETDEV_TYPE_MAX,
};




#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _EHIP_NETDEV_TYPE_H_