#pragma once

#include "engine/core/types.h"

// Erros comuns retornados pelas funções de rede.
enum net_err : i32
{
	NET_OK = 0,				  // Sem erro.
	NET_ERR_OPEN_SOCKET = -1, // Falha ao criar o socket.
	NET_ERR_DNS = -2,		  // Falha na resolução de nomes.
	NET_ERR_BIND = -3,		  // Falha ao associar o socket a um endereço.
	NET_ERR_INVALID = -4,	  // Argumento ou família de endereço inválido.
	NET_ERR_SEND = -5,		  // Falha ao enviar dados.
	NET_ERR_RECEIVE = -6,	  // Falha ao receber dados.
};

// Inicializa o subsistema de rede da plataforma.
// Deve ser chamada uma vez, antes de qualquer outra função de rede.
void net_platform_init();

// Finaliza o subsistema de rede da plataforma.
// Nenhuma função de rede deve ser usada depois dessa chamada.
void net_platform_terminate();

// Um endereço IPv4.
struct net_ip4
{
	u8 octets[4];
};

// Um endereço IPv6.
struct net_ip6
{
	u8 octets[16];
};

// Família de endereços IP.
enum net_family
{
	NET_V4, // IPv4.
	NET_V6, // IPv6.
};

// Um endereço IP de qualquer família. O campo ativo da união é indicado por family.
struct net_ip
{
	net_family family;
	union
	{
		net_ip4 v4;
		net_ip6 v6;
	};
};

// Converte IP em uma string terminada em nulo.
// Retorna `buf` ou `NULL` se a capacidade do buffer de escrita for insuficiente.
const char* net_ip_to_string(const net_ip* ip, char buf[], isize cap);

const char* net_ip4_to_string(const net_ip4* ip, char buf[], isize cap);

const char* net_ip6_to_string(const net_ip6* ip, char buf[], isize cap);

// Converte uma string num IP. Aceita apenas endereços puros, sem porta.
// Retorna `true` se a conversão foi bem sucedida.
bool net_try_parse_ip(const char* str, net_ip* out);

// Converte uma string num IP. Aceita apenas endereços puros, sem porta.
// Se a string for inválida, aborta o programa.
net_ip net_parse_ip(const char* str);

// Resolve `host` para endereços IP (v4 e v6) e os escreve em `buf`.
// Retorna a quantidade de endereços escritos ou NET_ERR_DNS em falha.
i32 net_resolve_host(const char* host, net_ip buf[], isize len);

// Um endereço de socket IPv4.
struct net_addr4
{
	net_ip4 ip;
	u16 port;
};

// Um endereço de socket IPv6.
struct net_addr6
{
	net_ip6 ip;
	u16 port;
};

// Um endereço de socket de qualquer família.
struct net_addr
{
	net_family family;
	union
	{
		net_addr4 v4;
		net_addr6 v6;
	};
};

// Inicializa addr a partir de um IP e uma porta.
void net_addr_init(net_addr* addr, net_ip ip, u16 port);

// Converte um endereço de socket para texto..
// Retorna `buf`, ou `NULL` se a capacidade for insuficiente ou a família for inválida.
const char* net_addr_to_string(const net_addr* addr, char buf[], isize cap);

const char* net_addr4_to_string(const net_addr4* addr, char buf[], isize cap);

const char* net_addr6_to_string(const net_addr6* addr, char buf[], isize cap);

// Representa um socket UDP.
struct net_udp_socket
{
	u64 handle;
};

// Cria um socket UDP e o associa a addr. Retorna NET_OK, ou um net_err em falha.
net_err net_udp_socket_bind(net_udp_socket* sock, const net_addr* addr, u32 flags);

// Envia `len` bytes para `to`.
// Retorna a quantidade de bytes enviada, ou um `net_err` negativo.
i32 net_udp_socket_send(net_udp_socket* sock, const net_addr* to, const u8 buf[], i32 len);

// Recebe um datagrama.
// Retorna a quantidade de bytes recebida, ou um net_err negativo.
i32 net_udp_socket_receive(net_udp_socket* sock, net_addr* from, u8 buf[], i32 cap);

// Fecha o socket e invalida o handle. É seguro chamar mais de uma vez.
void net_udp_socket_close(net_udp_socket* sock);
