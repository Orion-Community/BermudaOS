/*
 *  BermudaNet - Virtual Lan
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

/**
 * \file src/net/vlan.c
 * \brief VLAN support functions.
 * 
 * \addtogroup net
 * @{
 * \addtogroup vlan Virtual LAN
 * \brief Virtual LAN support.
 * @{
 */

#include <stdlib.h>

#include <net/netbuff.h>
#include <net/netdev.h>

#include <net/core/vlan.h>
#include <net/core/dev.h>

#include <netinet/in.h>

/**
 * \brief Create a VLAN from a raw VLAN-tag.
 * \param nb Netbuff containing the raw VLAN tag.
 * \return The raw VLAN tag. If it was impossible to create one, 0 is returned.
 * \note netbuff.raw_vlan must be initialized.
 */
PUBLIC struct vlan_tag *vlan_extract(struct netbuff *nb)
{
	struct vlan_tag *tag;
	uint16_t tci;
	
	if(!nb->raw_vlan) {
		return NULL;
	} else {
		tag = malloc(sizeof(*tag));
		tci = ntohs(nb->raw_vlan);
		tag->protocol_tag = IEEE8021Q_ETHERNET_TYPE;
		tag->vlan_id = tci & TCI_VLAN_ID_MASK;
		tag->format = (tci >> TCI_FORMAT_SHIFT) & TCI_FORMAT_MASK;
		tag->prio = (tci >> TCI_PRIO_SHIFT) & TCI_PRIO_MASK;
	}
	return tag;
}

/**
 * \brief Create a raw vlan tag based on the given vlan_tag structure.
 * \param tag VLAN-tag to convert to raw - network byte order orientated - format.
 * \return The raw VLAN-tag.
 */
PUBLIC __32be vlan_inflate(struct netbuff *nb)
{
	struct vlan_tag *tag = nb->vlan;
	__16be raw;
	__32be ret = 0;
	
	if(tag) {
		raw = htons(tag->vlan_id | (tag->format << TCI_FORMAT_SHIFT) | 
					(tag->prio << TCI_PRIO_SHIFT));
		ret = ((__32be)htons(IEEE8021Q_ETHERNET_TYPE) << 16) | raw;
	}
	
	return ret;
}

//@}
//@}
