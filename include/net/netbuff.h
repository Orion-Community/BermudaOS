/*
 *  BermudaNet - Network buffer header
 *  Copyright (C) 2012   Michel Megens
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <net/netdev.h>

#ifndef __NETBUFF_H
#define __NETBUFF_H

struct netbuff;

/**
 * Type definition of the netbuff flags data field(s).
 */
typedef uint16_t netbuff_features_t;

/**
 * \brief Network packet identifier.
 * 
 * The size of a packet can be calculated very easily. The size of the total packet should be equal
 * to <i><b>total := netbuff.end - netbuff.head</b></i>.
 */
struct packet_type
{
	struct packet_type *next; //!< Next pointer.
	
	__16be type; //!< 16-bit big-endian (network order) protocol identifier.
	struct netbuff* (*gso_segment)(struct netbuff *nb, uint16_t mtu); //!< Function to segment a packet.
};

struct netbuff
{
	struct netbuff *next; //!< Next pointer.
	
	struct netdev *dev; //!< Linked network device.
	struct packet_type *ptye; //!< Protocol identifier.
	
	netbuff_features_t features; //!< Options active on this netbuff.

	__32be raw_vlan;
	
	
	
	void *transport_hdr, //!< Transport header pointer.
		 *network_hdr,   //!< Network header pointer.
		 *link_hdr;      //!< Link layer header pointer.
	
	uint8_t *head,       //!< Data head pointer.
			*data,       //!< Data start pointer.
			*tail,       //!< Tail pointer (end of data).
			*end;        //!< End of packet pointer.
};

#endif