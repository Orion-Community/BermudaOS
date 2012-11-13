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

#ifdef HAVE_CONFIG_H
#include <netconfig.h>
#endif
#include <stdlib.h>

#include <net/netbuff.h>
#include <net/netdev.h>

#include <net/core/vlan.h>
#include <net/core/dev.h>

/**
 * \brief Create a VLAN from a raw VLAN-tag.
 * \param nb Netbuff containing the raw VLAN tag.
 * \return The raw VLAN tag. If it was impossible to create one, 0 is returned.
 * \note netbuff.raw_vlan must be initialized.
 */
PUBLIC struct vlan_tag *vlan_extract(struct netbuff *nb)
{
	return NULL;
}
