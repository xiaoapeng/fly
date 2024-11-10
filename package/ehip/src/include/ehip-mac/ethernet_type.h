/**
 * @file ethernet_type.h
 * @brief 以太网类型字节定义
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-11-09
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */
#ifndef _ETHERNET_TYPE_H_
#define _ETHERNET_TYPE_H_

/*
 *  These are the defined Ethernet Protocol ID's.
 */ 

#define EHIP_ETH_P_TSN                      0x22F0                              /* TSN (IEEE 1722) packet    */
#define EHIP_ETH_P_ERSPAN2                  0x22EB                              /* ERSPAN version 2 (type III)    */
#define EHIP_ETH_P_IP                       0x0800                              /* Internet Protocol packet    */
#define EHIP_ETH_P_X25                      0x0805                              /* CCITT X.25            */
#define EHIP_ETH_P_ARP                      0x0806                              /* Address Resolution packet    */
#define EHIP_ETH_P_BPQ                      0x08FF                              /* G8BPQ AX.25 Ethernet Packet    [ NOT AN OFFICIALLY REGISTERED ID ] */
#define EHIP_ETH_P_IEEEPUP                  0x0a00                              /* Xerox IEEE802.3 PUP packet */
#define EHIP_ETH_P_IEEEPUPAT                0x0a01                              /* Xerox IEEE802.3 PUP Addr Trans packet */
#define EHIP_ETH_P_BATMAN                   0x4305                              /* B.A.T.M.A.N.-Advanced packet [ NOT AN OFFICIALLY REGISTERED ID ] */
#define EHIP_ETH_P_DEC                      0x6000                              /* DEC Assigned proto           */
#define EHIP_ETH_P_DNA_DL                   0x6001                              /* DEC DNA Dump/Load            */
#define EHIP_ETH_P_DNA_RC                   0x6002                              /* DEC DNA Remote Console       */
#define EHIP_ETH_P_DNA_RT                   0x6003                              /* DEC DNA Routing              */
#define EHIP_ETH_P_LAT                      0x6004                              /* DEC LAT                      */
#define EHIP_ETH_P_DIAG                     0x6005                              /* DEC Diagnostics              */
#define EHIP_ETH_P_CUST                     0x6006                              /* DEC Customer use             */
#define EHIP_ETH_P_SCA                      0x6007                              /* DEC Systems Comms Arch       */
#define EHIP_ETH_P_TEB                      0x6558                              /* Trans Ether Bridging        */
#define EHIP_ETH_P_RARP                     0x8035                              /* Reverse Addr Res packet    */
#define EHIP_ETH_P_ATALK                    0x809B                              /* Appletalk DDP        */
#define EHIP_ETH_P_AARP                     0x80F3                              /* Appletalk AARP        */
#define EHIP_ETH_P_8021Q                    0x8100                              /* 802.1Q VLAN Extended Header  */
#define EHIP_ETH_P_ERSPAN                   0x88BE                              /* ERSPAN type II        */
#define EHIP_ETH_P_IPX                      0x8137                              /* IPX over DIX            */
#define EHIP_ETH_P_IPV6                     0x86DD                              /* IPv6 over bluebook        */
#define EHIP_ETH_P_PAUSE                    0x8808                              /* IEEE Pause frames. See 802.3 31B */
#define EHIP_ETH_P_SLOW                     0x8809                              /* Slow Protocol. See 802.3ad 43B */
#define EHIP_ETH_P_WCCP                     0x883E                              /* Web-cache coordination protocol
                                                                                 * defined in draft-wilson-wrec-wccp-v2-00.txt */
#define EHIP_ETH_P_MPLS_UC                  0x8847                              /* MPLS Unicast traffic        */
#define EHIP_ETH_P_MPLS_MC                  0x8848                              /* MPLS Multicast traffic    */
#define EHIP_ETH_P_ATMMPOA                  0x884c                              /* MultiProtocol Over ATM    */
#define EHIP_ETH_P_PPP_DISC                 0x8863                              /* PPPoE discovery messages     */
#define EHIP_ETH_P_PPP_SES                  0x8864                              /* PPPoE session messages    */
#define EHIP_ETH_P_LINK_CTL                 0x886c                              /* HPNA, wlan link local tunnel */
#define EHIP_ETH_P_ATMFATE                  0x8884                              /* Frame-based ATM Transport
                                                                                 * over Ethernet
                                                                                 */
#define EHIP_ETH_P_PAE                      0x888E                              /* Port Access Entity (IEEE 802.1X) */
#define EHIP_ETH_P_PROFINET                 0x8892                              /* PROFINET            */
#define EHIP_ETH_P_REALTEK                  0x8899                              /* Multiple proprietary protocols */
#define EHIP_ETH_P_AOE                      0x88A2                              /* ATA over Ethernet        */
#define EHIP_ETH_P_ETHERCAT                 0x88A4                              /* EtherCAT            */
#define EHIP_ETH_P_8021AD                   0x88A8                              /* 802.1ad Service VLAN        */
#define EHIP_ETH_P_802_EX1                  0x88B5                              /* 802.1 Local Experimental 1.  */
#define EHIP_ETH_P_PREAUTH                  0x88C7                              /* 802.11 Preauthentication */
#define EHIP_ETH_P_TIPC                     0x88CA                              /* TIPC             */
#define EHIP_ETH_P_LLDP                     0x88CC                              /* Link Layer Discovery Protocol */
#define EHIP_ETH_P_MRP                      0x88E3                              /* Media Redundancy Protocol    */
#define EHIP_ETH_P_MACSEC                   0x88E5                              /* 802.1ae MACsec */
#define EHIP_ETH_P_8021AH                   0x88E7                              /* 802.1ah Backbone Service Tag */
#define EHIP_ETH_P_MVRP                     0x88F5                              /* 802.1Q MVRP                  */
#define EHIP_ETH_P_1588                     0x88F7                              /* IEEE 1588 Timesync */
#define EHIP_ETH_P_NCSI                     0x88F8                              /* NCSI protocol        */
#define EHIP_ETH_P_PRP                      0x88FB                              /* IEC 62439-3 PRP/HSRv0    */
#define EHIP_ETH_P_CFM                      0x8902                              /* Connectivity Fault Management */
#define EHIP_ETH_P_FCOE                     0x8906                              /* Fibre Channel over Ethernet  */
#define EHIP_ETH_P_IBOE                     0x8915                              /* Infiniband over Ethernet    */
#define EHIP_ETH_P_TDLS                     0x890D                              /* TDLS */
#define EHIP_ETH_P_FIP                      0x8914                              /* FCoE Initialization Protocol */
#define EHIP_ETH_P_80221                    0x8917                              /* IEEE 802.21 Media Independent Handover Protocol */
#define EHIP_ETH_P_HSR                      0x892F                              /* IEC 62439-3 HSRv1    */
#define EHIP_ETH_P_NSH                      0x894F                              /* Network Service Header */
#define EHIP_ETH_P_LOOPBACK                 0x9000                              /* Ethernet loopback packet, per IEEE 802.3 */
#define EHIP_ETH_P_QINQ1                    0x9100                              /* deprecated QinQ VLAN [ NOT AN OFFICIALLY REGISTERED ID ] */
#define EHIP_ETH_P_QINQ2                    0x9200                              /* deprecated QinQ VLAN [ NOT AN OFFICIALLY REGISTERED ID ] */
#define EHIP_ETH_P_QINQ3                    0x9300                              /* deprecated QinQ VLAN [ NOT AN OFFICIALLY REGISTERED ID ] */
#define EHIP_ETH_P_EDSA                     0xDADA                              /* Ethertype DSA [ NOT AN OFFICIALLY REGISTERED ID ] */
#define EHIP_ETH_P_DSA_8021Q                0xDADB                              /* Fake VLAN Header for DSA [ NOT AN OFFICIALLY REGISTERED ID ] */
#define EHIP_ETH_P_DSA_A5PSW                0xE001                              /* A5PSW Tag Value [ NOT AN OFFICIALLY REGISTERED ID ] */
#define EHIP_ETH_P_IFE                      0xED3E                              /* ForCES inter-FE LFB type */
#define EHIP_ETH_P_AF_IUCV                  0xFBFB                              /* IBM af_iucv [ NOT AN OFFICIALLY REGISTERED ID ] */
#define EHIP_ETH_P_ETHERNET_II_MIN          0x0600                              /* If the value in the ethernet type is more than this value
                                                                                 * then the frame is Ethernet II. Else it is 802.3 */



#endif // _ETHERNET_TYPE_H_