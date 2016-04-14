#ifndef _INCLUDE__PRM_SESSION__CONNECTION_H_
#define _INCLUDE__PRM_SESSION__CONNECTION_H_

#include <prm_session/client.h>
#include <base/connection.h>

namespace Proxy { struct Prm_connection; }

struct Proxy::Prm_connection : Genode::Connection<Prm_session>, Proxy::Prm_session_client
{
	enum { RAM_QUOTA = 64*1024 };

	Prm_connection(Genode::addr_t start = ~0UL, Genode::size_t size = 0) :
		Connection<Proxy::Prm_session>(
			session("ram_quota=%u, start=0x%p, size=0x%zx",
			        RAM_QUOTA, start, size)),
		Prm_session_client(cap()) { }
};

#endif /* _INCLUDE__PRM_SESSION__CONNECTION_H_ */
