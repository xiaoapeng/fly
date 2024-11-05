/**
 * @file ether.h
 * @brief 以太网层数据链路层部分定义
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @version 1.0
 * @date 2024-10-30
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 * @par 修改日志:
 */
#ifndef _ETHER_H_
#define _ETHER_H_

#include <stdint.h>
#include "eh_types.h"
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


/*
 *	These are the defined Ethernet Protocol ID's.
 */

#define ETH_P_TSN	                0x22F0		                    /* TSN (IEEE 1722) packet	*/
#define ETH_P_ERSPAN2	            0x22EB		                    /* ERSPAN version 2 (type III)	*/
#define ETH_P_IP	                0x0800		                    /* Internet Protocol packet	*/
#define ETH_P_X25	                0x0805		                    /* CCITT X.25			*/
#define ETH_P_ARP	                0x0806		                    /* Address Resolution packet	*/
#define	ETH_P_BPQ	                0x08FF		                    /* G8BPQ AX.25 Ethernet Packet	[ NOT AN OFFICIALLY REGISTERED ID ] */
#define ETH_P_IEEEPUP	            0x0a00		                    /* Xerox IEEE802.3 PUP packet */
#define ETH_P_IEEEPUPAT	            0x0a01		                    /* Xerox IEEE802.3 PUP Addr Trans packet */
#define ETH_P_BATMAN	            0x4305		                    /* B.A.T.M.A.N.-Advanced packet [ NOT AN OFFICIALLY REGISTERED ID ] */
#define ETH_P_DEC                   0x6000                          /* DEC Assigned proto           */
#define ETH_P_DNA_DL                0x6001                          /* DEC DNA Dump/Load            */
#define ETH_P_DNA_RC                0x6002                          /* DEC DNA Remote Console       */
#define ETH_P_DNA_RT                0x6003                          /* DEC DNA Routing              */
#define ETH_P_LAT                   0x6004                          /* DEC LAT                      */
#define ETH_P_DIAG                  0x6005                          /* DEC Diagnostics              */
#define ETH_P_CUST                  0x6006                          /* DEC Customer use             */
#define ETH_P_SCA                   0x6007                          /* DEC Systems Comms Arch       */
#define ETH_P_TEB	                0x6558		                    /* Trans Ether Bridging		*/
#define ETH_P_RARP                  0x8035		                    /* Reverse Addr Res packet	*/
#define ETH_P_ATALK	                0x809B		                    /* Appletalk DDP		*/
#define ETH_P_AARP	                0x80F3		                    /* Appletalk AARP		*/
#define ETH_P_8021Q	                0x8100                          /* 802.1Q VLAN Extended Header  */
#define ETH_P_ERSPAN	            0x88BE		                    /* ERSPAN type II		*/
#define ETH_P_IPX	                0x8137		                    /* IPX over DIX			*/
#define ETH_P_IPV6	                0x86DD		                    /* IPv6 over bluebook		*/
#define ETH_P_PAUSE	                0x8808		                    /* IEEE Pause frames. See 802.3 31B */
#define ETH_P_SLOW	                0x8809		                    /* Slow Protocol. See 802.3ad 43B */
#define ETH_P_WCCP	                0x883E		                    /* Web-cache coordination protocol
					                                                 * defined in draft-wilson-wrec-wccp-v2-00.txt */
#define ETH_P_MPLS_UC	            0x8847		                    /* MPLS Unicast traffic		*/
#define ETH_P_MPLS_MC	            0x8848		                    /* MPLS Multicast traffic	*/
#define ETH_P_ATMMPOA	            0x884c		                    /* MultiProtocol Over ATM	*/
#define ETH_P_PPP_DISC	            0x8863		                    /* PPPoE discovery messages     */
#define ETH_P_PPP_SES	            0x8864		                    /* PPPoE session messages	*/
#define ETH_P_LINK_CTL	            0x886c		                    /* HPNA, wlan link local tunnel */
#define ETH_P_ATMFATE	            0x8884		                    /* Frame-based ATM Transport
					                                                 * over Ethernet
					                                                 */
#define ETH_P_PAE	                0x888E		                    /* Port Access Entity (IEEE 802.1X) */
#define ETH_P_PROFINET	            0x8892		                    /* PROFINET			*/
#define ETH_P_REALTEK	            0x8899                          /* Multiple proprietary protocols */
#define ETH_P_AOE	                0x88A2		                    /* ATA over Ethernet		*/
#define ETH_P_ETHERCAT	            0x88A4		                    /* EtherCAT			*/
#define ETH_P_8021AD	            0x88A8                          /* 802.1ad Service VLAN		*/
#define ETH_P_802_EX1	            0x88B5		                    /* 802.1 Local Experimental 1.  */
#define ETH_P_PREAUTH	            0x88C7		                    /* 802.11 Preauthentication */
#define ETH_P_TIPC	                0x88CA		                    /* TIPC 			*/
#define ETH_P_LLDP	                0x88CC		                    /* Link Layer Discovery Protocol */
#define ETH_P_MRP	                0x88E3		                    /* Media Redundancy Protocol	*/
#define ETH_P_MACSEC	            0x88E5		                    /* 802.1ae MACsec */
#define ETH_P_8021AH	            0x88E7                          /* 802.1ah Backbone Service Tag */
#define ETH_P_MVRP	                0x88F5                          /* 802.1Q MVRP                  */
#define ETH_P_1588	                0x88F7		                    /* IEEE 1588 Timesync */
#define ETH_P_NCSI	                0x88F8		                    /* NCSI protocol		*/
#define ETH_P_PRP	                0x88FB		                    /* IEC 62439-3 PRP/HSRv0	*/
#define ETH_P_CFM	                0x8902		                    /* Connectivity Fault Management */
#define ETH_P_FCOE	                0x8906		                    /* Fibre Channel over Ethernet  */
#define ETH_P_IBOE	                0x8915		                    /* Infiniband over Ethernet	*/
#define ETH_P_TDLS	                0x890D                          /* TDLS */
#define ETH_P_FIP	                0x8914		                    /* FCoE Initialization Protocol */
#define ETH_P_80221	                0x8917		                    /* IEEE 802.21 Media Independent Handover Protocol */
#define ETH_P_HSR	                0x892F		                    /* IEC 62439-3 HSRv1	*/
#define ETH_P_NSH	                0x894F		                    /* Network Service Header */
#define ETH_P_LOOPBACK	            0x9000		                    /* Ethernet loopback packet, per IEEE 802.3 */
#define ETH_P_QINQ1	                0x9100		                    /* deprecated QinQ VLAN [ NOT AN OFFICIALLY REGISTERED ID ] */
#define ETH_P_QINQ2	                0x9200		                    /* deprecated QinQ VLAN [ NOT AN OFFICIALLY REGISTERED ID ] */
#define ETH_P_QINQ3	                0x9300		                    /* deprecated QinQ VLAN [ NOT AN OFFICIALLY REGISTERED ID ] */
#define ETH_P_EDSA	                0xDADA		                    /* Ethertype DSA [ NOT AN OFFICIALLY REGISTERED ID ] */
#define ETH_P_DSA_8021Q	            0xDADB		                    /* Fake VLAN Header for DSA [ NOT AN OFFICIALLY REGISTERED ID ] */
#define ETH_P_DSA_A5PSW	            0xE001		                    /* A5PSW Tag Value [ NOT AN OFFICIALLY REGISTERED ID ] */
#define ETH_P_IFE	                0xED3E		                    /* ForCES inter-FE LFB type */
#define ETH_P_AF_IUCV               0xFBFB		                    /* IBM af_iucv [ NOT AN OFFICIALLY REGISTERED ID ] */
#define ETH_P_802_3_MIN	            0x0600		                    /* If the value in the ethernet type is more than this value
					                                                 * then the frame is Ethernet II. Else it is 802.3 */

#define ETH_HWADDR_LEN    6

struct eth_addr {
	uint8_t addr[ETH_HWADDR_LEN];
}eh_aligned(1);

struct eth_hdr{
	struct eth_addr 	dest;
	struct eth_addr  	src;
    uint16_t	 		type_or_len;
}eh_aligned(1);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _ETHER_H_