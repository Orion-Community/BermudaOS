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

//! \file include/net/netbuff.h Netbuff header file.

/**
 * \addtogroup netCore
 * @{
 */

#ifndef __NETBUFF_H
#define __NETBUFF_H

struct netdev;
struct netbuff;

/**
 * \brief Type definition of the netbuff flags data field(s).
 */
typedef uint16_t netbuff_features_t;

/**
 * \def NETIF_TX_VLAN_TAG
 * \brief Defines that there is a VLAN transmission tag set.
 */
#define NETIF_TX_VLAN_TAG B1

/**
 * \brief Defines that the packet may not be fragmented.
 * \warning Packets which are to large and are flagged with this flag are discarded.
 */
#define NETBUFF_NO_FRAG B10

/**
 * \def NETIF_TX_QUEUE_FLAG
 * \brief Flag to mark a transmit queue.
 */
#define NETIF_TX_QUEUE_FLAG B1

/**
 * \def NETIF_RX_QUEUE_FLAG
 * \brief Flag to mark a receive queue.
 */
#define NETIF_RX_QUEUE_FLAG B10

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

/**
 * \brief Queues used to to queue up packets waiting for processing.
 */
struct netbuff_queue
{
	struct netbuff_queue *next; //!< Next packet in the list.
	
	struct netbuff *packet;     //!< Queued packet.
	
	/**
	 * \brief Type of the queue.
	 * \see NETIF_TX_QUEUE_FLAG
	 * \see NETIF_RX_QUEUE_FLAG
	 * 
	 * This member can be set to <i>NETIF_TX_QUEUE_FLAG</i> <b>AND/OR</b> <i>NETIF_RX_QUEUE_FLAG</i>.
	 */
	uint8_t type : 2;
};

/**
 * \brief A netbuff is a data structure which represents a network packet.
 */
struct netbuff
{
	struct netbuff *next; //!< Next pointer.
	
	struct netdev *dev; //!< Linked network device.
	struct packet_type *ptye; //!< Protocol identifier.
	
	netbuff_features_t features; //!< Options active on this netbuff.

	__32be raw_vlan; //!< Raw VLAN tag. It is tagged or detagged by the core layer.
	struct vlan_tag *vlan; //!< The VLAN tag.
	
	size_t 	length, //!< Total length of this segment.
			data_length; //!< Total length of the payload in this segment.
	
	
	void 	*transport_hdr, //!< Transport header pointer.
			*network_hdr,   //!< Network header pointer.
			*link_hdr;      //!< Link layer header pointer.
	
	uint8_t *head,       //!< Data head pointer.
			*data,       //!< Data start pointer.
			*tail,       //!< Tail pointer (end of data).
			*end;        //!< End of packet pointer.
};

#ifdef __DOXYGEN__
#else
__DECL
#endif
/**
 * \brief Returns the features of the netbuff.
 * \param nb Netbuff whose features you want.
 * \return <i>nb</i>'s features.
 */
static inline netbuff_features_t netbuff_get_features(struct netbuff *nb)
{
	return nb->features;
}

/**
 * \brief Returns wether this packet is in a VLAN or not.
 * \param nb Packet to check.
 * \return If in a VLAN <b>TRUE</b>, if not <b>FALSE</b>.
 */
static inline bool nb_has_tx_tag(struct netbuff *nb)
{
	return (nb->features & NETIF_TX_VLAN_TAG);
}

/**
 * \brief Get the netdev of this netbuff.
 * \param nb Buffer whose netdev you want.
 */
static inline struct netdev *dev(struct netbuff *nb)
{
	return nb->dev;
}
#ifdef __DOXYGEN__
#else
__DECL_END
#endif

#endif

/**
 * @}
 */
