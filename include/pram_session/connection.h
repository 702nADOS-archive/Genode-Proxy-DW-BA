#ifndef _INCLUDE__PRAM_SESSION__CONNECTION_H_
#define _INCLUDE__PRAM_SESSION__CONNECTION_H_

#include <pram_session/client.h>
#include <base/connection.h>

namespace Proxy { struct Pram_connection; }

struct Proxy::Pram_connection : Genode::Connection<Pram_session>, Proxy::Pram_session_client
{
	enum { RAM_QUOTA = 64*1024 };

	Pram_connection(const char *label = "", unsigned long phys_start = 0UL,
	               unsigned long phys_size = 0UL)
	:
		Connection<Proxy::Pram_session>(
			session("ram_quota=%u, phys_start=0x%lx, phys_size=0x%lx, "
			        "label=\"%s\"", RAM_QUOTA, phys_start, phys_size, label)),
		Pram_session_client(cap()) { }
};
#endif /* _INCLUDE__PRAM_SESSION__CONNECTION_H_ */
