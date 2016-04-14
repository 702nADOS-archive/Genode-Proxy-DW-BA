#ifndef	_INCLUDE__PRAM_SESSION__CLIENT_H_
#define _INCLUDE__PRAM_SESSION__CLIENT_H_

#include <pram_session/pram_session.h>
#include <pram_session/capability.h>
#include <base/rpc_client.h>

namespace Proxy { struct Pram_session_client; }

struct Proxy::Pram_session_client : Genode::Rpc_client<Pram_session>
{
	Pram_session_client(Pram_session_capability cap)
	: Genode::Rpc_client<Pram_session>(cap) { }

	Genode::Ram_dataspace_capability alloc(Genode::size_t size, Genode::Cache_attribute cached = Genode::CACHED) override {
		return call<Rpc_alloc>(size, cached); }

	void free(Genode::Ram_dataspace_capability ds) override { call<Rpc_free>(ds); }

	int ref_account(Genode::Ram_session_capability ram_session) override {
			return call<Rpc_ref_account>(ram_session); }

	int transfer_quota(Genode::Ram_session_capability ram_session, Genode::size_t amount) override {
		return call<Rpc_transfer_quota>(ram_session, amount); }

	Genode::size_t quota() override { return call<Rpc_quota>(); }

	Genode::size_t used() override { return call<Rpc_used>(); }

	Genode::Ram_session_capability ram_cap() override { return call<Rpc_ram_cap>(); }
};

#endif /* _INCLUDE__PRAM_SESSION__CLIENT_H_ */
