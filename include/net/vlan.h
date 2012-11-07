/*
 *  BermudaNet - VLAN data structure header.
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

#ifndef __VLAN_H
#define __VLAN_H

#include <stdlib.h>

struct netbuff;

/**
 * \brief Extracted representation of a VLAN.
 * \note Raw VLANS can be extracted using vlan_extract. Extracted VLANS can be converted to raw using
 *       vlan_inflate.
 */
struct vlan_tag
{
	uint16_t protocol_tag; //!< Protocol tag. Should be 0x8100 for a valid VLAN tag.
	unsigned char prio : 3; //!< Priority flags.
	unsigned char format : 1; //!< Canonical format idicator.
	uint16_t vlan_id : 12; //!< VLAN identifier.
};

/**
 * \brief Create a raw vlan tag based on the given vlan_tag structure.
 * \param tag VLAN-tag to convert to raw - network byte order orientated - format.
 * \return The raw VLAN-tag.
 */
static inline __32be vlan_inflate(struct vlan_tag *tag)
{
	return 0;
}

/**
 * \brief Create a VLAN from a raw VLAN-tag.
 * \param nb Netbuff containing the raw VLAN tag.
 * \return The raw VLAN tag. If it was impossible to create one, 0 is returned.
 * \note netbuff.raw_vlan must be initialized.
 */
static inline struct vlan_tag *vlan_extract(struct netbuff *nb)
{
	return NULL;
}

#endif