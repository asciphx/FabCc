  
#include "uv.h"
#include "internal.h"

#include <errno.h>
#include <stddef.h>

#include <ifaddrs.h>
#include <net/if.h>
#if !defined(__CYGWIN__) && !defined(__MSYS__) && !defined(__GNU__)
#include <net/if_dl.h>
#endif

#if defined(__HAIKU__)
#define IFF_RUNNING IFF_LINK
#endif
 static int uv__ifaddr_exclude(struct ifaddrs *ent, int exclude_type) { if (!((ent->ifa_flags & IFF_UP) && (ent->ifa_flags & IFF_RUNNING))) return 1; if (ent->ifa_addr == NULL) return 1;
#if !defined(__CYGWIN__) && !defined(__MSYS__) && !defined(__GNU__)
  if (exclude_type == UV__EXCLUDE_IFPHYS) return (ent->ifa_addr->sa_family != AF_LINK);
#endif
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__DragonFly__) || defined(__HAIKU__)
   if (ent->ifa_addr->sa_family == AF_LINK) return 1;
#elif defined(__NetBSD__) || defined(__OpenBSD__)
 if (ent->ifa_addr->sa_family != PF_INET && ent->ifa_addr->sa_family != PF_INET6) return 1;
#endif
 return 0; } int uv_interface_addresses(uv_interface_address_t** addresses, int* count) { struct ifaddrs* addrs; struct ifaddrs* ent; uv_interface_address_t* address;
#if !(defined(__CYGWIN__) || defined(__MSYS__)) && !defined(__GNU__)
 int i;
#endif
 *count = 0; *addresses = NULL;  if (getifaddrs(&addrs) != 0) return UV__ERR(errno);  for (ent = addrs; ent != NULL; ent = ent->ifa_next) { if (uv__ifaddr_exclude(ent, UV__EXCLUDE_IFADDR)) continue; (*count)++; }  if (*count == 0) { freeifaddrs(addrs); return 0; }  *addresses = uv__calloc(*count, sizeof(**addresses));  if (*addresses == NULL) { freeifaddrs(addrs); return UV_ENOMEM; }  address = *addresses;  for (ent = addrs; ent != NULL; ent = ent->ifa_next) { if (uv__ifaddr_exclude(ent, UV__EXCLUDE_IFADDR)) continue;  address->name = uv__strdup(ent->ifa_name);  if (ent->ifa_addr->sa_family == AF_INET6) { address->address.address6 = *((struct sockaddr_in6*) ent->ifa_addr); } else { address->address.address4 = *((struct sockaddr_in*) ent->ifa_addr); }  if (ent->ifa_netmask == NULL) { memset(&address->netmask, 0, sizeof(address->netmask)); } else if (ent->ifa_netmask->sa_family == AF_INET6) { address->netmask.netmask6 = *((struct sockaddr_in6*) ent->ifa_netmask); } else { address->netmask.netmask4 = *((struct sockaddr_in*) ent->ifa_netmask); }  address->is_internal = !!(ent->ifa_flags & IFF_LOOPBACK);  ++address; } 
#if !(defined(__CYGWIN__) || defined(__MSYS__)) && !defined(__GNU__)
  for (ent = addrs; ent != NULL; ent = ent->ifa_next) { if (uv__ifaddr_exclude(ent, UV__EXCLUDE_IFPHYS)) continue;  address = *addresses;  for (i = 0; i < *count; ++i) { if (strcmp(address->name, ent->ifa_name) == 0) { struct sockaddr_dl* sa_addr; sa_addr = (struct sockaddr_dl*)(ent->ifa_addr); memcpy(address->phys_addr, LLADDR(sa_addr), sizeof(address->phys_addr)); } ++address; } }
#endif
 freeifaddrs(addrs);  return 0; }  void uv_free_interface_addresses(uv_interface_address_t* addresses,  int count) { int i;  for (i = 0; i < count; ++i) { uv__free(addresses[i].name); }  uv__free(addresses); } 