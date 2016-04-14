#ifndef _INCLUDE__PRM_SESSION__CLIENT_H_
#define _INCLUDE__PRM_SESSION__CLIENT_H_

#include <prm_session/capability.h>
#include <prm_session/prm_session.h>
#include <base/rpc_client.h>

namespace Proxy { struct Prm_session_client; }

struct Proxy::Prm_session_client : Genode::Rpc_client<Prm_session>
{
	explicit Prm_session_client(Prm_session_capability session)
	: Genode::Rpc_client<Prm_session>(session) { }

	Genode::Rm_session::Local_addr attach(Genode::Dataspace_capability ds, Genode::size_t size = 0,
	                  Genode::off_t offset = 0, bool use_local_addr = false,
	                  Genode::Rm_session::Local_addr local_addr = (void *)0,
	                  bool executable = false) override
	{
		return call<Rpc_attach>(ds, size, offset,
		                        use_local_addr, local_addr,
		                        executable);
	}

	void detach(Genode::Rm_session::Local_addr local_addr) override {
		call<Rpc_detach>(local_addr); }

	Genode::Pager_capability add_client(Genode::Thread_capability thread) override {
		return call<Rpc_add_client>(thread); }

	void remove_client(Genode::Pager_capability pager) override {
		call<Rpc_remove_client>(pager); }

	void fault_handler(Genode::Signal_context_capability handler) override {
		call<Rpc_fault_handler>(handler); }

	Genode::Rm_session::State state() override {
		return call<Rpc_state>(); }

	Genode::Dataspace_capability dataspace() override {
		return call<Rpc_dataspace>(); }
};

#endif /* _INCLUDE__PRM_SESSION__CLIENT_H_ */
