#ifndef _PROXY__INCLUDE__PDATASPACE_COMPONENT_H_
#define _PROXY__INCLUDE__PDATASPACE_COMPONENT_H_

#include <pdataspace/pdataspace.h>
#include <dataspace/client.h>
#include <rm_session/connection.h>
#include <base/rpc_server.h>
#include <util/list.h>
#include <base/lock_guard.h>
#include <base/thread.h>
#include <base/signal.h>

namespace Proxy {

	class Prm_attachment; 			// forward declaration
	class Pdataspace_component;		// forward declaration

	/*
	*	Pdataspace_fault_handler
	*	
	*
	*	## undocumented methods are described in Pdataspace_component.cc ##
	*/

	class Pdataspace_fault_handler : public Genode::Thread<8192>
	{
		private:

			Genode::Signal_receiver *_receiver;		// pointer to the Signal_receiver which has to be observed

			Pdataspace_component *_pdataspace;		// pinter to the Pdataspace tht fault handler is responsible for

		public:

			Pdataspace_fault_handler(Genode::Signal_receiver *receiver, Pdataspace_component *pdataspace);
			
			~Pdataspace_fault_handler() { }	

			void fault_handle();
			
			void entry();
	};


	/*
	*	Pdataspace_owner
	*
	*	Every pdataspace_component should only be destroyed by the Pram_session_component
	*	which initially created it.
	*	Therefore each Pdataspace holds a pointer to its respective owner.
	*
	*	Pdataspace_owner is a parent class of Pram_session_component.
	*/

	class Pdataspace_owner { };


	/*
	*	Pdataspace_component
	*
	*	## undocumented methods are described in Pdataspace_component.cc ##
	*/

	class Pdataspace_component : public Genode::Rpc_object<Pdataspace>
	{
		private:

			enum { START_OF_SUB_RM = 0 };

			Genode::Rm_connection _rm;								// rm_session - used as managed dataspace
			Genode::Dataspace_capability _managed_dataspace;		// dataspace representation of _rm

			Genode::Dataspace_capability _dataspace;				// dataspace backing the managed dataspace
			Genode::Dataspace_client _ds_client;					// used to provide the dataspace interface functionality
	
			bool _dataspace_attached;								// indicates if backing store is attached to managed dataspace

			bool _snapshotable;										// indicates if snapshotable dataspace is available
			Genode::Dataspace_capability _snapshotable_dataspace;	// dataspace used for snapshot next
			Genode::Lock _lock_snap;								// lock for _dataspace_attached and _snapshotable

			Genode::List<Prm_attachment> _attachments;				// list of prm_attachments holding a capability of this Pdataspace
			Genode::Lock _lock_att;									// lock for Prm_attachment-list

			Pdataspace_owner const *_owner;							// owner of the Pdataspace (Pram_session_component)
			Genode::Ram_session *_owner_ram;						// Ram_session of the respective owner -> used to create new backing dataspaces
			
		
			Genode::Signal_receiver _receiver;
			Genode::Signal_context _context;

			Pdataspace_fault_handler _handler;
		

			/*********************
			 ** Private methods **
			 *********************/

			void _attach_dataspace();	// unsynchronized!

			void _detach_dataspace();	// unsynchronized!

			bool _create_new_dataspace();

			bool _copy_dataspace_content(Genode::Dataspace_capability src,
											Genode::Dataspace_capability dst,
											Genode::size_t size);
			
		public:

			Pdataspace_component(Genode::size_t sub_size,
									Genode::Dataspace_capability ds,
									Pdataspace_owner *owner,
									Genode::Ram_session *owner_ram);
			
			~Pdataspace_component();

			void attached_to(Prm_attachment *prm_att);
				
			void detached_from(Prm_attachment *prm_att);

			void detach_from_prm_session();

			bool is_shared(); // currently not in use

			void create_snapshotable();

			void free_snapshotable();	

			void reattach(); 

			bool owner(Pdataspace_owner * const o) const { return _owner == o; }	// check the ownership of the Pdataspace_component

			//Genode::List<Prm_attachment> *attachments() { return &_attachments; }


			/***************
			 ** Accessors **
			 ***************/

			Genode::Dataspace_capability 			  dataspace() const { return _dataspace; 		 		}
			Genode::Dataspace_client				  ds_client() const { return _ds_client;				}
			Genode::Dataspace_capability	  managed_dataspace() const { return _managed_dataspace;	 	}
			bool							 dataspace_attached() const { return _dataspace_attached;		}
			bool								   snapshotable() const { return _snapshotable;				}
			Genode::Dataspace_capability snapshotable_dataspace() const { return _snapshotable_dataspace;	}


			/**************************
			 ** Dataspace interface **
			 **************************/

			Genode::size_t size()            { return _ds_client.size();	  }
			Genode::addr_t phys_addr()       { return _ds_client.phys_addr(); }
			bool		   writable()        { return _ds_client.writable();  }
	};
}

#endif /* _PROXY__INCLUDE__PDATASPACE_COMPONENT_H_ */
