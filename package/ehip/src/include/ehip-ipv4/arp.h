/**
 * @file arp.h
 * @brief arp protocol
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-11-13
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */

#ifndef __IPV4_ARP_H__
#define __IPV4_ARP_H__

#ifndef _ARP_H_
#define _ARP_H_

#include <stdint.h>
#include <eh_types.h>
#include <ehip_netdev_trait.h>
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

struct arp_hdr{
	uint16_t		ar_hrd;		/* format of hardware address	*/
	uint16_t		ar_pro;		/* format of protocol address	*/
    uint8_t	        ar_hln;		/* length of hardware address	*/
	uint8_t	        ar_pln;		/* length of protocol address	*/
	uint16_t		ar_op;		/* ARP opcode (command)		*/
}eh_aligned(sizeof(char));


/* ARP protocol opcodes. */
#define	ARPOP_REQUEST	1		/* ARP request			*/
#define	ARPOP_REPLY		2		/* ARP reply			*/
#define	ARPOP_RREQUEST	3		/* RARP request			*/
#define	ARPOP_RREPLY	4		/* RARP reply			*/
#define	ARPOP_InREQUEST	8		/* InARP request		*/
#define	ARPOP_InREPLY	9		/* InARP reply			*/
#define	ARPOP_NAK		10		/* (ATM)ARP NAK			*/


static inline unsigned int arp_hdr_len(const ehip_netdev_t *dev)
{
	return sizeof(struct arp_hdr) + ((size_t)ehip_netdev_trait_hw_addr_len(dev) + sizeof(uint32_t)) * 2;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _ARP_H_



#endif