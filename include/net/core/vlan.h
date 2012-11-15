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

//! \file include/net/vlan.h VLAN header

#ifndef __VLAN_H
#define __VLAN_H

#include <stdlib.h>

#include <lib/binary.h>

struct netbuff;

#define IEEE8021Q_ETHERNET_TYPE 0x8100

#define TCI_VLAN_ID_MASK 0xFFF
#define TCI_FORMAT_SHIFT 12
#define TCI_FORMAT_MASK B1
#define TCI_PRIO_SHIFT 13
#define TCI_PRIO_MASK B111

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

#ifdef __DOXYGEN__
#else
__DECL
#endif
extern __32be vlan_inflate(struct vlan_tag *tag);
extern struct vlan_tag *vlan_extract(struct netbuff *nb);
#ifdef __DOXYGEN__
#else
__DECL_END
#endif
#endif