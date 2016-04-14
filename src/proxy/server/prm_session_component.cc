/* Genode includes */
#include <base/printf.h>

/* proxy includes */
#include "include/prm_session_component.h"

using namespace Genode;
using namespace Proxy;

/*
*	Prm_attachment --- Constructor
*	
*	\param : ds
*	\param : size
*	\param : off
*	\param : use_local_addr
*	\param : addr
*	\param : executable
*	\param : session_comp
*
*	params are explained in Prm_session_component.h
*
*	\return : -
*
*	\functionality: creates an object of Prm_attachment
*/

Prm_attachment::Prm_attachment(Genode::Dataspace_capability ds, Genode::size_t size,
								Genode::off_t off, bool use_local_addr, Genode::addr_t addr,
								bool executable, Prm_session_component *session_comp)
: _ds(ds), _size(size), _off(off),
	_use_local_addr(use_local_addr),
	_addr(addr),
	_executable(executable),
	_session_comp(session_comp)
{ 
	PINF("Prm_attachment created!"); 
}

// -------------------------------------------------------------------------------------------------------------------------

/*
*	Prm_session_component --- Constructor
*	
*	\param : -
*	\param : ds
*	\param : ds
*	\param : ds
*
*	\return : bool
*
*	\functionality: Copies the content of src to dst. the length of the content has to be defined in size
*/

Prm_session_component::Prm_session_component(Genode::Rpc_entrypoint *ds_ep,
												Genode::Allocator *md_alloc,
												Genode::addr_t start,
												Genode::size_t size)
: 	_ds_ep(ds_ep), _att_alloc(md_alloc),
	_rm(start,size), _vm_start(start),
	_vm_size(size) { }


/*
*	Prm_session_component --- attach
*	
*	\param : ds_cap - capability of the dataspace which is attached
*	\param : size - size of the dataspace
*	\param : offset - offset within the dataspace (if this is greater than 0, only a part of the dataspace is attached)
*	\param : use_local_addr - indicates whether the param local_addr is used
*	\param : local_addr
*	\param : executable - indicates whether the content of the dataspace is executable code
*
*	\return : start address of the part of the virtual memory to which the dataspace is attached
*
*	\functionality: hand the dataspace capability to the  proxy entrypoint to find out if the dataspace is a
*					pdataspace. if it is a pdataspace its managed dataspace is attached.
*/

Rm_session::Local_addr Prm_session_component::attach(Dataspace_capability ds_cap, size_t size,
											off_t offset, bool use_local_addr,
											Local_addr local_addr,
											bool executable)
{
	Lock::Guard lock_guard(_lock);

	auto lambda = [&] (Pdataspace_component *pds)
	{
		Genode::addr_t addr;

		if(!pds)
		{
			try
			{
				addr = _rm.attach(ds_cap, size, offset, use_local_addr, local_addr, executable);
			}
			catch(Exception)
			{
				throw;
			}

			PINF("Non-snapshotable dataspace attached!");

			return addr;
		}

		

		try
		{
			addr = _rm.attach(pds->managed_dataspace(), size, offset, use_local_addr, local_addr, executable);
		}
		catch(Exception)
		{
			throw;
		}

		PINF("Snapshotable dataspace attached!");

		Prm_attachment *pa;

		try
		{
			pa = new (&_att_alloc) Prm_attachment(ds_cap, size, offset, use_local_addr, addr, executable, this);
		}
		catch(Allocator::Out_of_memory)
		{
			PERR("Error ocurred during creation of Prm_attachment!");
			throw Rm_session::Out_of_metadata();
		}

		_attachments.insert(pa);

		pds->attached_to(pa);

		return addr;
	};

	return _ds_ep->apply(ds_cap, lambda);
}


/*
*	Pdataspace_component --- detach
*	
*	\param : local_addr - start address of the part of the virtual memory which the client wants to free
*
*	\return : void
*
*	\functionality: searches the Prm_attachment list to find the capability of the dataspace the client wants to detach and detaches it
*/

void Prm_session_component::detach(Local_addr local_addr)
{
	Lock::Guard lock_guard(_lock);

	Prm_attachment *pa = _attachments.first();
	for (; pa; pa = pa->next()) {
		if(pa->addr() == (addr_t) local_addr) break;
	}

	if (pa) {

		_ds_ep->apply(pa->dataspace(), [pa] (Pdataspace_component *pds)
		{
			if(!pds)
			{
				PINF("Detachment of a dataspace which is no Pdataspace!");
				return;
			}

			pds->detached_from(pa);
			PINF("Pdataspace informed about detachment!");
			return;
		});

		_attachments.remove(pa);
		destroy(_att_alloc, pa);
		PINF("Prm_attachment destroyed!");
	}

	_rm.detach(local_addr);
}


/*
*	Prm_session_component --- add_client
*	
*	\param : thread - capability of the thread which is associated with the RM-Session
*
*	\return : capability of the pager which is responsible for the thread
*
*	\functionality: associates a thread with the internal RM-Session
*/

Pager_capability Prm_session_component::add_client(Thread_capability thread)
{
	try
	{
		return _rm.add_client(thread);
	}
	catch(Exception)
	{
		throw;
	}
}


/*
*	Prm_session_component --- remove_client
*	
*	\param : pager - capability of the pager which is responsible for the thread which is disassociated with the RM-Session
*
*	\return : void
*
*	\functionality: revokes the association of a thread with the internal RM-Session
*/

void Prm_session_component::remove_client(Pager_capability pager)
{
	_rm.remove_client(pager);
}


/*
*	Pdataspace_component --- fault_handler
*	
*	\param : context - capability of the Signal context
*
*	\return : void
*
*	\functionality: associates a fault handler with the internal RM-Session
*/

void Prm_session_component::fault_handler(Signal_context_capability context)
{
	_rm.fault_handler(context);
}


/*
*	Prm_session_component --- state
*	
*	\param : -
*
*	\return : state of the internal RM-Session
*
*	\functionality: calls the state methode of the internal RM-Session
*/

Rm_session::State Prm_session_component::state()
{
	return _rm.state();
}


/*
*	Prm_session_component --- dataspace
*	
*	\param : -
*
*	\return : capability of the dataspace representing the PRM-Session
*
*	\functionality: calls the dataspace method of the internal RM-Session
*/

Dataspace_capability Prm_session_component::dataspace()
{
	return _rm.dataspace();
}