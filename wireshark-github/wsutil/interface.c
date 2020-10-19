/* interface.c
 * Utility functions to get infos from interfaces
 *
 * Copyright 2016, Dario Lombardo
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "config.h"

#include "interface.h"

#include <string.h>
#include <wsutil/inet_addr.h>

#ifdef HAVE_SYS_TYPES_H
	#include <sys/types.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
	#include <sys/socket.h>
#endif

#ifdef HAVE_NETINET_IN_H
	#include <netinet/in.h>
#endif

#ifdef HAVE_ARPA_INET_H
	#include <arpa/inet.h>
#endif

#ifdef HAVE_IFADDRS_H
	#include <ifaddrs.h>
#endif

#ifdef _WIN32
	#include <winsock2.h>
	#include <iphlpapi.h>
	#include <Ws2tcpip.h>
#endif

#define WORKING_BUFFER_SIZE 15000

#ifdef HAVE_GETIFADDRS
static GSList* local_interfaces_to_list_nix(void)
{
	GSList *interfaces = NULL;
	struct ifaddrs *ifap;
	struct ifaddrs *ifa;
	int family;
	char ip[INET6_ADDRSTRLEN];

	if (getifaddrs(&ifap)) {
		goto end;
	}

	for (ifa = ifap; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
			continue;

		family = ifa->ifa_addr->sa_family;

		memset(ip, 0x0, INET6_ADDRSTRLEN);

		switch (family) {
			case AF_INET:
			{
				struct sockaddr_in *addr4 = (struct sockaddr_in *)ifa->ifa_addr;
				ws_inet_ntop4(&addr4->sin_addr, ip, sizeof(ip));
				break;
			}

			case AF_INET6:
			{
				struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)ifa->ifa_addr;
				ws_inet_ntop6(&addr6->sin6_addr, ip, sizeof(ip));
				break;
			}

			default:
				break;
		}

		/* skip loopback addresses */
		if (!g_strcmp0(ip, "127.0.0.1") || !g_strcmp0(ip, "::1"))
			continue;

		if (*ip) {
			interfaces = g_slist_prepend(interfaces, g_strdup(ip));
		}
	}
	freeifaddrs(ifap);
end:
	return interfaces;
}
#endif /* HAVE_GETIFADDRS */

#ifdef _WIN32
static GSList* local_interfaces_to_list_win(void)
{
	GSList *interfaces = NULL;
	PIP_ADAPTER_ADDRESSES pAddresses = NULL;
	ULONG outBufLen = WORKING_BUFFER_SIZE;
	ULONG family = AF_UNSPEC;
	PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
	PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
	char ip[100];
	guint iplen = 100;

	pAddresses = (IP_ADAPTER_ADDRESSES *)g_malloc0(outBufLen);
	if (pAddresses == NULL)
		return NULL;

	if (GetAdaptersAddresses(family, 0, NULL, pAddresses, &outBufLen) != NO_ERROR)
		goto end;

	pCurrAddresses = pAddresses;
	while (pCurrAddresses) {

		for (pUnicast = pCurrAddresses->FirstUnicastAddress; pUnicast != NULL; pUnicast = pUnicast->Next) {
			if (pUnicast->Address.lpSockaddr->sa_family == AF_INET) {
				SOCKADDR_IN* sa_in = (SOCKADDR_IN *)pUnicast->Address.lpSockaddr;
				ws_inet_ntop4(&(sa_in->sin_addr), ip, iplen);
				if (!g_strcmp0(ip, "127.0.0.1"))
					continue;
				if (*ip)
					interfaces = g_slist_prepend(interfaces, g_strdup(ip));
			}
			if (pUnicast->Address.lpSockaddr->sa_family == AF_INET6) {
				SOCKADDR_IN6* sa_in6 = (SOCKADDR_IN6 *)pUnicast->Address.lpSockaddr;
				ws_inet_ntop6(&(sa_in6->sin6_addr), ip, iplen);
				if (!g_strcmp0(ip, "::1"))
					continue;
				if (*ip)
					interfaces = g_slist_prepend(interfaces, g_strdup(ip));
			}
		}
		pCurrAddresses = pCurrAddresses->Next;
	}
end:
	if (pAddresses)
		g_free(pAddresses);

	return interfaces;
}
#endif /* _WIN32 */

GSList* local_interfaces_to_list(void)
{
#if defined(_WIN32)
	return local_interfaces_to_list_win();
#elif defined(HAVE_GETIFADDRS)
	return local_interfaces_to_list_nix();
#else
	return NULL;
#endif
}

/*
 * Editor modelines  -  https://www.wireshark.org/tools/modelines.html
 *
 * Local variables:
 * c-basic-offset: 4
 * tab-width: 8
 * indent-tabs-mode: t
 * End:
 *
 * vi: set shiftwidth=4 tabstop=8 noexpandtab:
 * :indentSize=4:tabSize=8:noTabs=false:
 */
