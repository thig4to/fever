#include "engine/core/net.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(PLATFORM_WINDOWS)
#include <winsock2.h>
#include <ws2tcpip.h>

typedef SOCKET impl_socket;

#define IMPL_INVALID_SOCKET INVALID_SOCKET

void net_platform_init()
{
	WSAData api_info;
	WSAStartup(MAKEWORD(2, 2), &api_info);
}


void net_platform_terminate()
{
	WSACleanup();
}


static void impl_close_socket(impl_socket s)
{
	closesocket(s);
}
#endif

#if defined(PLATFORM_LINUX)
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

typedef i32 impl_socket;

#define IMPL_INVALID_SOCKET (-1)

void net_platform_init()
{
}


void net_platform_terminate()
{
}


static void impl_close_socket(impl_socket sock)
{
	close(sock);
}
#endif

// Lida com o erro de chamada de `getaddrinfo`.
static net_err handle_dns_resolve_fail(i32 _)
{
	return NET_ERR_DNS;
}


// Inicializa uma estrutura `sockaddr_storage` a partir de um `net_addr`. Retorna o tamanho da
// estrutura, ou 0 se a família for inválida.
static i32 net_addr_to_platform_addr(const net_addr* addr, sockaddr_storage* p_addr)
{
	memset(p_addr, 0, sizeof(sockaddr_storage));

	if (addr->family == NET_V4)
	{
		sockaddr_in* p_addr4 = (sockaddr_in*)p_addr;
		p_addr4->sin_family = AF_INET;
		p_addr4->sin_port = htons(addr->v4.port);
		memcpy(&p_addr4->sin_addr, addr->v4.ip.octets, 4);

		return sizeof(sockaddr_in);
	}
	else if (addr->family == NET_V6)
	{
		sockaddr_in6* p_addr6 = (sockaddr_in6*)p_addr;
		p_addr6->sin6_family = AF_INET6;
		p_addr6->sin6_port = htons(addr->v6.port);
		memcpy(&p_addr6->sin6_addr, addr->v6.ip.octets, 16);

		return sizeof(sockaddr_in6);
	}

	return 0;
}


// Inicializa uma estrutura `net_addr` a partir de um `sockaddr_storage`. Retorna se a conversão
// foi bem sucedida.
static bool platform_addr_to_net_addr(const sockaddr_storage* p_addr, net_addr* addr)
{
	if (p_addr->ss_family == AF_INET)
	{
		const sockaddr_in* p_addr4 = (sockaddr_in*)p_addr;
		addr->family = NET_V4;
		addr->v4.port = ntohs(p_addr4->sin_port);
		memcpy(addr->v4.ip.octets, &p_addr4->sin_addr, 4);

		return true;
	}
	else if (p_addr->ss_family == AF_INET6)
	{
		const sockaddr_in6* p_addr6 = (const sockaddr_in6*)p_addr;
		addr->family = NET_V6;
		addr->v6.port = ntohs(p_addr6->sin6_port);
		memcpy(addr->v6.ip.octets, &p_addr6->sin6_addr, 16);

		return true;
	}

	return false;
}


// Inicializa uma estrutura `net_ip` a partir de um `sockaddr`. Retorna se a conversão foi
// bem sucedida.
static bool platform_addr_to_net_ip(const sockaddr* p_addr, net_ip* ip)
{
	if (p_addr->sa_family == AF_INET)
	{
		const sockaddr_in* p_addr4 = (sockaddr_in*)p_addr;
		ip->family = NET_V4;
		memcpy(ip->v4.octets, &p_addr4->sin_addr, 4);
		return true;
	}
	else if (p_addr->sa_family == AF_INET6)
	{
		const sockaddr_in6* p_addr6 = (sockaddr_in6*)p_addr;
		ip->family = NET_V6;
		memcpy(ip->v6.octets, &p_addr6->sin6_addr, 16);
		return true;
	}

	return false;
}


i32 net_resolve_host(const char* name, net_ip buf[], isize len)
{
	addrinfo hints = {};
	hints.ai_socktype = SOCK_STREAM;

	addrinfo* addr_list = NULL;
	i32 code = getaddrinfo(name, NULL, &hints, &addr_list);
	if (code != 0)
	{
		return (i32)handle_dns_resolve_fail(code);
	}

	addrinfo* curr = addr_list;
	isize count = 0;
	while (curr != NULL && count < len)
	{
		bool ok = platform_addr_to_net_ip(curr->ai_addr, &buf[count]);
		if (ok)
		{
			count++;
		}

		curr = curr->ai_next;
	}

	freeaddrinfo(addr_list);
	return (i32)count;
}


const char* net_ip_to_string(const net_ip* ip, char buf[], isize cap)
{
	switch (ip->family)
	{
		case NET_V4: return net_ip4_to_string(&ip->v4, buf, cap);
		case NET_V6: return net_ip6_to_string(&ip->v6, buf, cap);
		default:	 return NULL;
	}
}


const char* net_ip4_to_string(const net_ip4* ip, char buf[], isize cap)
{
	if (cap <= 0)
		return NULL;

	return inet_ntop(AF_INET, ip->octets, buf, cap);
}


const char* net_ip6_to_string(const net_ip6* ip, char buf[], isize cap)
{
	if (cap <= 0)
		return NULL;

	return inet_ntop(AF_INET6, ip->octets, buf, cap);
}


bool net_try_parse_ip(const char* str, net_ip* ip)
{
	if (inet_pton(AF_INET, str, ip->v4.octets) == 1)
	{
		ip->family = NET_V4;
		return true;
	}
	else if (inet_pton(AF_INET6, str, ip->v6.octets) == 1)
	{
		ip->family = NET_V6;
		return true;
	}

	return false;
}


net_ip net_parse_ip(const char* str)
{
	net_ip ip;
	bool ok = net_try_parse_ip(str, &ip);
	if (!ok)
		abort();

	return ip;
}


void net_addr_init(net_addr* addr, net_ip ip, u16 port)
{
	addr->family = ip.family;

	if (ip.family == NET_V4)
	{
		addr->v4.ip = ip.v4;
		addr->v4.port = port;
	}
	else if (ip.family == NET_V6)
	{
		addr->v6.ip = ip.v6;
		addr->v6.port = port;
	}
}


const char* net_addr4_to_string(const net_addr4* addr, char buf[], isize cap)
{
	if (cap <= 0)
		return NULL;

	char ip_str[64];
	const char* ptr = net_ip4_to_string(&addr->ip, ip_str, sizeof(ip_str));
	if (!ptr)
		return NULL;

	int written = snprintf(buf, (size_t)cap, "%s:%u", ip_str, (u32)addr->port);
	if (written < 0 || written >= cap)
		return NULL;

	return buf;
}


const char* net_addr6_to_string(const net_addr6* addr, char buf[], isize cap)
{
	if (cap <= 0)
		return NULL;

	char ip_str[64];
	const char* ptr = net_ip6_to_string(&addr->ip, ip_str, sizeof(ip_str));
	if (!ptr)
		return NULL;

	int written = snprintf(buf, (size_t)cap, "[%s]:%u", ip_str, (u32)addr->port);
	if (written < 0 || written >= cap)
		return NULL;

	return buf;
}


const char* net_addr_to_string(const net_addr* addr, char buf[], isize cap)
{
	switch (addr->family)
	{
		case NET_V4: return net_addr4_to_string(&addr->v4, buf, cap);
		case NET_V6: return net_addr6_to_string(&addr->v6, buf, cap);
		default:	 return NULL;
	}
}


net_err net_udp_socket_bind(net_udp_socket* sock, const net_addr* addr, u32 flags)
{
	(void)flags; // Reservado para opções futuras.

	sock->handle = IMPL_INVALID_SOCKET;

	sockaddr_storage p_addr;
	i32 size = net_addr_to_platform_addr(addr, &p_addr);
	if (size == 0)
	{
		return NET_ERR_INVALID;
	}

	impl_socket p_sock = socket(p_addr.ss_family, SOCK_DGRAM, IPPROTO_UDP);
	if (p_sock == IMPL_INVALID_SOCKET)
	{
		return NET_ERR_OPEN_SOCKET;
	}

	if (p_addr.ss_family == AF_INET6)
	{
		int v6_only = 1;
		setsockopt(p_sock, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&v6_only, sizeof(v6_only));
	}

	i32 res = bind(p_sock, (sockaddr*)&p_addr, size);
	if (res == -1)
	{
		impl_close_socket(p_sock);
		return NET_ERR_BIND;
	}

	sock->handle = (u64)p_sock;
	return NET_OK;
}


i32 net_udp_socket_send(net_udp_socket* sock, const net_addr* to, const u8 buffer[], i32 len)
{
	sockaddr_storage plat_addr;
	i32 size = net_addr_to_platform_addr(to, &plat_addr);
	if (size == 0)
	{
		return NET_ERR_INVALID;
	}

	i32 n = sendto(sock->handle, (const char*)buffer, len, 0, (sockaddr*)&plat_addr, size);
	if (n < 0)
	{
		return NET_ERR_SEND;
	}

	return n;
}


i32 net_udp_socket_receive(net_udp_socket* sock, net_addr* from, u8 buffer[], i32 cap)
{
	sockaddr_storage plat_addr;
	i32 size = sizeof(plat_addr);

	i32 n = (i32)recvfrom(sock->handle, (char*)buffer, cap, 0, (sockaddr*)&plat_addr, &size);
	if (n < 0)
	{
		return NET_ERR_RECEIVE;
	}

	bool ok = platform_addr_to_net_addr(&plat_addr, from);
	if (!ok)
	{
		return NET_ERR_INVALID;
	}

	return n;
}


void net_udp_socket_close(net_udp_socket* sock)
{
	if (sock->handle == IMPL_INVALID_SOCKET)
		return;

	impl_close_socket((impl_socket)sock->handle);
	sock->handle = IMPL_INVALID_SOCKET;
}
