#ifndef _INCLUDE__PRAM_SESSION__PRAM_SESSION_H_
#define _INCLUDE__PRAM_SESSION__PRAM_SESSION_H_

#include <ram_session/ram_session.h>

namespace Proxy { struct Pram_session; }

struct Proxy::Pram_session : Genode::Ram_session
{
	static const char *service_name() { return "PRAM"; }

	virtual Genode::Ram_session_capability ram_cap() = 0;

	GENODE_RPC(Rpc_ram_cap, Genode::Ram_session_capability, ram_cap);

	GENODE_RPC_INTERFACE(Rpc_alloc, Rpc_free, Rpc_ref_account,
	                     Rpc_transfer_quota, Rpc_quota, Rpc_used,
	                     Rpc_ram_cap);
};

#endif /* _INCLUDE__PRAM_SESSION__PRAM_SESSION_H_ */
