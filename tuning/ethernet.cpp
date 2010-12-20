/*
 * Copyright 2010, Intel Corporation
 *
 * This file is part of PowerTOP
 *
 * This program file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program in a file named COPYING; if not, write to the
 * Free Software Foundation, Inc,
 * 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 * or just google for it.
 *
 * Authors:
 *	Arjan van de Ven <arjan@linux.intel.com>
 */

#include "tuning.h"
#include "tunable.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>
#include <iostream>
#include <fstream>
#include <unistd.h> 
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <linux/types.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>

#include <linux/ethtool.h>

#include "../lib.h"
#include "ethernet.h"

ethernet_tunable::ethernet_tunable(const char *iface) : tunable("", 0.3, "Good", "Bad", "Unknown")
{
	printf("iface is %s \n", iface);
	memset(interf, 0, sizeof(interf));
	strncpy(interf, iface, sizeof(interf));
	sprintf(desc, "Wake-on-lan status for device %s", iface);
}


int ethernet_tunable::good_bad(void)
{
	int sock;
	struct ifreq ifr;
	struct ethtool_wolinfo wol;
	int ret;
	int result = TUNE_GOOD;

	memset(&ifr, 0, sizeof(struct ifreq));

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock<0)
		return result;

	strcpy(ifr.ifr_name, interf);

	/* Check if the interf is up */
	ret = ioctl(sock, SIOCGIFFLAGS, &ifr);
	if (ret<0) {
		close(sock);
		return result;
	}

	if (ifr.ifr_flags & (IFF_UP | IFF_RUNNING)) {
		close(sock);
		return result;
	}

	memset(&wol, 0, sizeof(wol));

	wol.cmd = ETHTOOL_GWOL;
	ifr.ifr_data = (caddr_t)&wol;
        ioctl(sock, SIOCETHTOOL, &ifr);

	if (wol.wolopts)
		result = TUNE_BAD;

	close(sock);

	return result;
}

void ethernet_tunable::toggle(void)
{
	int sock;
	struct ifreq ifr;
	struct ethtool_wolinfo wol;
	int ret;

	memset(&ifr, 0, sizeof(struct ifreq));

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock<0)
		return;

	strcpy(ifr.ifr_name, interf);

	/* Check if the interface is up */
	ret = ioctl(sock, SIOCGIFFLAGS, &ifr);
	if (ret<0) {
		close(sock);
		return;
	}

	if (ifr.ifr_flags & (IFF_UP | IFF_RUNNING)) {
		close(sock);
		return;
	}

	memset(&wol, 0, sizeof(wol));

	wol.cmd = ETHTOOL_GWOL;
	ifr.ifr_data = (caddr_t)&wol;
        ioctl(sock, SIOCETHTOOL, &ifr);
	wol.cmd = ETHTOOL_SWOL;
	wol.wolopts = 0;
        ioctl(sock, SIOCETHTOOL, &ifr);

	close(sock);
}




void add_ethernet_tunable(void)
{
	class ethernet_tunable *eth;

	eth = new class ethernet_tunable("eth0");
	if (eth)
		all_tunables.push_back(eth);
}
