#ifndef _PROXY__INCLUDE__PRAM_SESSION_COMPONENT_H_
#define _PROXY__INCLUDE__PRAM_SESSION_COMPONENT_H_

#include <base/rpc_server.h>
#include <pram_session/pram_session.h>
#include <ram_session/connection.h>
#include <util/list.h>
#include <base/allocator_guard.h>
#include <base/tslab.h>
#include <base/synced_allocator.h>
#include "pdataspace_component.h"

namespace Proxy {

	/*
	*	Pram_session_component
	*
	*	Objects of this class represent sessions to the PRAM-Service
	*
	*	## undocumented methods are either getter or described in Pram_session_component.cc ##
	*/

	class Pram_session_component : public Genode::Rpc_object<Pram_session>,
									public Pdataspace_owner
	{
		private:


			Genode::Rpc_entrypoint *_ep;							// entrypoint of proxy - pram_sessions and pdataspace objects are associated with it

			Genode::Tslab<Pdataspace_component, 4096> _ds_alloc;	// allocator for objects of Pdataspace_component

			Genode::Ram_connection _ram;							// session to core's RAM-Service - used to allocate Core-Datspaces

			const char* _label;										// label the client chose for its session
			unsigned long _phys_start;
			unsigned long _phys_size;

		public:

			Pram_session_component(Genode::Rpc_entrypoint *ep,
									Genode::Allocator *md_alloc, const char *label,
									unsigned long phys_start, unsigned long phys_size);

			~Pram_session_component() { } //Destructor
			

			/*********************/
			/**Session interface**/
			/*********************/
			
			Genode::Ram_dataspace_capability alloc(Genode::size_t, Genode::Cache_attribute);
			void free(Genode::Ram_dataspace_capability);
			int ref_account(Genode::Ram_session_capability);
			int transfer_quota(Genode::Ram_session_capability, Genode::size_t);
			Genode::size_t quota();
			Genode::size_t used();
			Genode::Ram_session_capability ram_cap();
	};
}

#endif /* _PROXY__INCLUDE__PRAM_SESSION_COMPONENT_H_ */
