#ifndef _INCLUDE__PDATASPACE__CLIENT_H_
#define _INCLUDE__PDATASPACE__CLIENT_H_

#include <pdataspace/capability.h>
#include <pdataspace/pdataspace.h>
#include <base/rpc_client.h>

namespace Proxy { struct Pdataspace_client; }


struct Genode::Pdataspace_client : Genode::Rpc_client<Pdataspace>
{
	explicit Dataspace_client(Genode::Pdataspace_capability pds)
	: Genode::Rpc_client<Pdataspace>(pds) { }

	Genode::size_t size()      override { return call<Rpc_size>();      }
	Genode::addr_t phys_addr() override { return call<Rpc_phys_addr>(); }
	bool		   writable()  override { return call<Rpc_writable>();  }
};

#endif /* _INCLUDE__PDATASPACE__CLIENT_H_ */
