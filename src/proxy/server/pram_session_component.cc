/* Genode includes */
#include <base/printf.h>
#include <rm_session/connection.h>
#include <ram_session/ram_session.h>

/* Proxy includes */
#include "include/pram_session_component.h"

using namespace Genode;
using namespace Proxy;

/*
*	Pram_session_component --- Constructor
*	
*	\param : -
*
*	\return : -
*
*	\functionality: Creates a Pram_session_component object
*/

Pram_session_component::Pram_session_component(Genode::Rpc_entrypoint *ep,
												Genode::Allocator *md_alloc, const char *label,
												unsigned long phys_start, unsigned long phys_size) 
: 	_ep(ep),
	_ds_alloc(md_alloc),
	_ram(label, phys_start, phys_size),
	_label(label),
	_phys_start(phys_start),
	_phys_size(phys_size) { }


/*
*	Pram_session_component --- alloc
*	
*	\param : ds_size - desired size of the returned dataspace
*	\param : chached - indicates whether the physical memory represented by the dataspace should be cached
*
*	\return : capability of a Pdataspace cast to a RAM-Dataspace
*
*	\functionality: allocates a core dataspace and uses it to create a pdataspace. the pdataspace is associated 
*					with the entrypoint of proxy
*/

Ram_dataspace_capability Pram_session_component::alloc(size_t ds_size, Cache_attribute cached)
{
	Ram_dataspace_capability ds;

	try{
		ds = _ram.alloc(ds_size, cached);
	}
	catch (Exception)
	{
		throw;
	}

	Pdataspace_component *pds;

	try{
		pds = new (&_ds_alloc) Pdataspace_component(ds_size, ds, this, &_ram);
	}
	catch (Allocator::Out_of_memory) {
		_ram.free(ds);
		PERR("No pdataspace created. Error occured!");
		throw Ram_session::Out_of_metadata();
	}

	Dataspace_capability result = _ep->manage(pds);

	return static_cap_cast<Ram_dataspace>(result);
}


/*
*	Pram_session_component --- free
*	
*	\param : ds_cap - capability of the Dataspace, that the client wants to release
*
*	\return : void
*
*	\functionality: hands the capability to the entrypoint to find the corresponding
*					Pdataspace_component object. the core dataspace is freed and the
*					pdataspace gets detroyed.
*/

void Pram_session_component::free(Ram_dataspace_capability ds_cap)
{
	Pdataspace_component *pds;

	_ep->apply(ds_cap, [&] (Pdataspace_component *c) 
	{
		pds = c;

		if(!pds) return;
		if(!pds->owner(this)) return;

		_ep->dissolve(pds);

		pds->detach_from_prm_session();

		_ram.free(static_cap_cast<Ram_dataspace>(pds->dataspace()));
	});

	destroy(&_ds_alloc, pds);
}


/*
*	Pram_session_component --- ref_account
*	
*	\param : ram_session_cap - capability of the ram-session which will be the reference account
*
*	\return : int
*
*	\functionality: checks if the target session is a PRAM-Session or a RAM-Session and registers it as reference account
*/

int Pram_session_component::ref_account(Ram_session_capability ram_session_cap)
{
	auto lambda = [&, ram_session_cap] (Pram_session_component *ref) {

		if(!ref) return _ram.ref_account(ram_session_cap);
		return _ram.ref_account(ref->_ram.cap());
	};
	return _ep->apply(ram_session_cap, lambda);
}   


/*
*	Pram_session_component --- Constructor
*	
*	\param : ram_session_cap - capability of the target RAM-Session
*
*	\return : int
*
*	\functionality: checks if the target session is a PRAM-Session or a RAM-Session and transfers the quota
*/

int Pram_session_component::transfer_quota(Ram_session_capability ram_session_cap, size_t amount)
{
	auto lambda = [&, ram_session_cap, amount] (Pram_session_component *dst) {

		if(!dst) return _ram.transfer_quota(ram_session_cap, amount);
		return _ram.transfer_quota(dst->_ram.cap(), amount);
	};
	return _ep->apply(ram_session_cap, lambda);
}


/*
*	Pram_session_component --- Constructor
*	
*	\param : -
*
*	\return : -
*
*	\functionality: returns the amount of the quota donated to the client
*/

size_t Pram_session_component::quota()
{
	return _ram.quota();
}


/*
*	Pram_session_component --- Constructor
*	
*	\param : -
*
*	\return : -
*
*	\functionality: returns the amount of the quota in use
*/

size_t Pram_session_component::used()
{
	return _ram.used();
}


/*
*	Pram_session_component --- ram_cap()
*	
*	\param : -
*
*	\return : Ram_session_capability of the internal Core-Session
*
*	\functionality: returns the RAM-Session-Capability of the internal core session
*/

Ram_session_capability Pram_session_component::ram_cap()
{
	return _ram.cap();
}
