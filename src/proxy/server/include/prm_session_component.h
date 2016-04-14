#ifndef _PROXY_INCLUDE__PRM_SESSION_COMPONENT_H_
#define _PROXY_INCLUDE__PRM_SESSION_COMPONENT_H_

#include <base/printf.h>
#include <base/stdint.h>
#include <base/rpc_server.h>
#include <session/session.h>
#include <thread/capability.h>
#include <pager/capability.h>
#include <prm_session/prm_session.h>
#include <rm_session/connection.h>
#include <util/list.h>
#include <base/allocator_guard.h>
#include <base/tslab.h>
#include <base/lock.h>
#include "pdataspace_component.h"

namespace Proxy {

	class Prm_session_component; // forward declaration

	/*
	*	Prm_attachment
	*
	* 	Pram_attachment objects store data about the attachments of Pdataspaces
	*
	*	## undocumented methods are either getter or described in Prm_session_component.cc ##
	*/

	class Prm_attachment : public Genode::List<Prm_attachment>::Element 
	{
		private:
			
			Genode::Dataspace_capability _ds; 		// capability of the dataspace
			Genode::size_t _size;					// size of the attached dataspace
			Genode::off_t _off;						// offset within the dataspace (if greater than 0 only parts of the dataspace are attached)
			bool _use_local_addr;					// indicates if the client defined the start adress of the attachment
			Genode::addr_t _addr;					// start address of the adress space in which the dataspace is placed
			bool _executable;						// indicates  whether the content of the dataspace is executable code

			Prm_session_component *_session_comp;	// pointer to the PRM-Session the respective object belongs to

		public: 

			Prm_attachment(Genode::Dataspace_capability ds, Genode::size_t size,
							Genode::off_t off, bool use_local_addr, Genode::addr_t local_addr,
							bool executable, Prm_session_component *session_comp);
			
			~Prm_attachment() { }


			/***************
			 ** Accessors **
			 ***************/
			
			Genode::Dataspace_capability	dataspace() const { return _ds;				}
			Genode::size_t						 size() const { return _size;			}
			Genode::off_t					   offset() const { return _off;			}
			bool						   executable() const { return _executable;		}
			bool							use_local() const { return _use_local_addr;	}
			Genode::addr_t						 addr() const { return _addr;			}
			Prm_session_component*		 session_comp() const { return _session_comp; 	}
	};
	

	/*
	*	Prm_session_component
	*
	* 	Prm_session_component represent sessions to the PRM-Service
	*
	*	## undocumented methods are either getter or described in Prm_session_component.cc ##
	*/

	class Prm_session_component : public Genode::Rpc_object<Prm_session>
	{
		private:

			Genode::Rpc_entrypoint *_ds_ep;						// entrypoint

			Genode::Tslab<Prm_attachment, 1024> _att_alloc;		// allocator for Prm_attachment objects
			Genode::List<Prm_attachment> _attachments;			// list of all Prm_attachments
			
			Genode::Rm_connection _rm;							// session to Core's RM-Service

			Genode::addr_t _vm_start;							// start of the virtual adress space
			Genode::size_t _vm_size;							// size of the virtual adress space

			Genode::Lock _lock;									// lock - regulates the acces to the Prm_attachemnt list

		public:

			Prm_session_component(Genode::Rpc_entrypoint *ds_ep,
									Genode::Allocator *md_alloc,
									Genode::addr_t start,
									Genode::size_t size);

			~Prm_session_component() { }


			/********************************************
			 ** Proxy region manager session interface **
			 ********************************************/

			Genode::Rm_session::Local_addr      attach        (Genode::Dataspace_capability, Genode::size_t,
																Genode::off_t, bool, Genode::Rm_session::Local_addr, bool);
			void             					detach        (Genode::Rm_session::Local_addr);
			Genode::Pager_capability 			add_client    (Genode::Thread_capability);
			void             					remove_client (Genode::Pager_capability);
			void             					fault_handler (Genode::Signal_context_capability handler);
			Genode::Rm_session::State           state         ();
			Genode::Dataspace_capability 		dataspace     ();
	};
}

#endif /* _PROXY_INCLUDE__PRM_SESSION_COMPONENT_H_ */
