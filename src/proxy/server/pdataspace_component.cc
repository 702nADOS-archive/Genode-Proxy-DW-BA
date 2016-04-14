/* Genode includes */
#include <base/printf.h>

/* proxy includes */
#include "include/pdataspace_component.h"
#include "include/prm_session_component.h"

using namespace Genode;
using namespace Proxy;

/*
*	Pdataspace_fault_handler --- Constructor
*	
*	\param : receiver - Signal_receiver of the respective pdataspace
*	\param : pdataspace - pointer to the pdataspace it the Pdataspace_fault_handler object is responsible for
*
*	\return : -
*
*	\functionality: Creates a Pdataspace_fault_handler object.
*/

Pdataspace_fault_handler::Pdataspace_fault_handler(Signal_receiver *receiver,
													Pdataspace_component *pdataspace)
: Thread("Pdataspace_fault_handler"), _receiver(receiver),
	_pdataspace(pdataspace) { }


/*
*	Pdataspace_fault_handler --- fault_handle
*	
*	\param : -
*
*	\return : void
*
*	\functionality: The dataspace gets reattached to the Rm_session (managed dataspace).
*/

void Pdataspace_fault_handler::fault_handle()
{
	_pdataspace->reattach();
}


/*
*	Pdataspace_fault_handler --- entry
*	
*	\param : -
*
*	\return : void
*
*	\functionality: Waits for _reciever to receive a signal indicating that a page_fault occured.
					Then starts a method handling the fault.
					This method gets executed when the Pdatspace_fault_handler
					(Thread) gets started.
*/

void Pdataspace_fault_handler::entry()
{
	while(true)
	{
		Signal signal = _receiver->wait_for_signal();

		for (unsigned i = 0; i < signal.num(); i++)
			fault_handle();
	}
}

/********************************************************************************************************************/

/*
*	Pdataspace_component --- Constructor
*	
*	\param : sub_start 	- start address of managed dataspace
*	\param : sub_size	- size of managed dataspace
*	\param : ds 		- dataspace backing the managed dataspace
*	\param : owner 		- pointer to the owner of the Pdatspace (Pram_session_component which created it)
*
*	\return : -
*
*	\functionality: Creates a Pdataspace object.
*/

Pdataspace_component::Pdataspace_component(Genode::size_t sub_size,
											Genode::Dataspace_capability ds,
											Pdataspace_owner *owner,
											Genode::Ram_session *owner_ram)
			: 	_rm(START_OF_SUB_RM, sub_size),
				_managed_dataspace(_rm.dataspace()),
				_dataspace(ds), _ds_client(_dataspace),
				_dataspace_attached(false), _snapshotable(false),
				_owner(owner), _owner_ram(owner_ram),
				_handler(&_receiver, this)
			{ 
				_attach_dataspace();
				_rm.fault_handler(_receiver.manage(&_context));
				PINF("Pdataspace component created!"); 
			}


/*
*	Pdataspace_component --- attach_dataspace
*	
*	\param : -
*
*	\return : void
*
*	\functionality: The dataspace backing the managed dataspace of the Pdataspace
					gets attached.
*/

void Pdataspace_component::_attach_dataspace()
{
	if (!_dataspace_attached) _rm.attach_at(_dataspace, START_OF_SUB_RM);
	_dataspace_attached = true;
}


/*
*	Pdataspace_component --- detach_dataspace
*	
*	\param : -
*
*	\return : void
*
*	\functionality: The dataspace backing the managed dataspace of the Pdataspace
					gets detached.
*/

void Pdataspace_component::_detach_dataspace()
{
	if (_dataspace_attached) _rm.detach(START_OF_SUB_RM);
	_dataspace_attached = false;
}


/*
*	Pdataspace_component --- _create_new_dataspace
*	
*	\param : -
*
*	\return : bool
*
*	\functionality: Creates new Dataspace as and adjusts the Dataspace_client
*/

bool Pdataspace_component::_create_new_dataspace()
{
	if(_dataspace_attached)
	{
		return false;
	}
	else
	{
		_dataspace = _owner_ram->alloc(_ds_client.size());
		_ds_client = Dataspace_client(_dataspace);
		return true;
	}
}


/*
*	Pdataspace_component --- _create_new_dataspace
*	
*	\param : -
*
*	\return : bool
*
*	\functionality: Copies the content of src to dst. the length of the content has to be defined in size
*/

bool Pdataspace_component::_copy_dataspace_content(Dataspace_capability src, Dataspace_capability dst, size_t size)
{
	void* dst_addr;
	void* src_addr;
	try
	{
		dst_addr = env()->rm_session()->attach(dst);
		src_addr = env()->rm_session()->attach(src);
	}
	catch (Exception)
	{
		PERR("_copy_dataspace_content failed");
		return false;
	}

	memcpy(dst_addr, src_addr, size);
	
	env()->rm_session()->detach(dst_addr);
	env()->rm_session()->detach(src_addr);

	return true;
}


/*
*	Pdataspace_component --- attached_to
*	
*	\param : -
*
*	\return : void
*
*	\functionality:	Adds a Prm_attachment to _attachments.
*					This method gets called when the Pdataspace is attached to a Prm_session.
*/

void Pdataspace_component::attached_to(Prm_attachment *prm_att)
{
	Lock::Guard lock_guard(_lock_att);
	_attachments.insert(prm_att);
}


/*
*	Pdataspace_component --- detached_from
*	
*	\param : prm_att : Pointer to the Prm_attachment which is removed from the list
*
*	\return : void
*
*	\functionality:	Removes a Prm_attachment from _attachments.
*					This method gets called when the Pdataspace is detached from a Prm_session.
*/

void Pdataspace_component::detached_from(Prm_attachment *prm_att)
{
	Lock::Guard lock_guard(_lock_att);
	_attachments.remove(prm_att);
}


/*
*	Pdataspace_component --- detach_from_prm_session
*	
*	\param : -
*
*	\return : void
*
*	\functionality: detaches the Pdataspace from every PRM-Session it is attached to
*/

void Pdataspace_component::detach_from_prm_session()
{
	_lock_att.lock();

	while(Prm_attachment *pa = _attachments.first())
	{
		_lock_att.unlock();
		pa->session_comp()->detach((void *)pa->addr());
		_lock_att.lock();
	}
	_lock_att.unlock();
}


/*
*	Pdataspace_component --- is_shared
*	
*	\param : -
*
*	\return : true - if Pdataspace is attached to more than just one Prm_session / false - otherwise
*
*	\functionality: The method checks the _attachments-list. If the list holds multiple elements
*					the Pdataspace is considered to be shared.
*/

bool Pdataspace_component::is_shared()
{
	Lock::Guard lock_guard(_lock_att);

	Prm_attachment *pa = _attachments.first();
	if(!pa) return false;
	
	pa = pa->next();
	if(!pa) return false;

	return true;
}


/*
*	Pdataspace_component --- create_snapshotable
*	
*	\param : -
*
*	\return : void
*
*	\functionality: Detaches the backing dataspace from the managed dataspace and
					puts the capability/reference to _snapshotable_dataspace.
*/

void Pdataspace_component::create_snapshotable()
{
	Lock::Guard lock_guard(_lock_snap);
	_detach_dataspace();
	_snapshotable_dataspace = _dataspace;
	_snapshotable = true;
}


/*
*	Pdataspace_component --- free_snapshotable
*	
*	\param : -
*
*	\return : void
*
*	\functionality: This method frees the dataspace which is stored for snapshotting purposes.
*/

void Pdataspace_component::free_snapshotable()
{
	Lock::Guard lock_guard(_lock_snap);
	if(_snapshotable)
	{
		_owner_ram->free(static_cap_cast<Ram_dataspace>(_snapshotable_dataspace));
		_snapshotable = false;
	}
	else { return; }
}


/*
*	Pdataspace_component --- reattach()
*	
*	\param : -
*
*	\return : void
*
*	\functionality: Attaches a newly created dataspace to the managed dataspace.
*					This method is called by the Pdataspace_fault_handler.
*/

void Pdataspace_component::reattach()
{
	Lock::Guard lock_guard(_lock_snap);
	if(!_dataspace_attached)
	{
		_create_new_dataspace();
		if(_copy_dataspace_content(_dataspace, _snapshotable_dataspace, _ds_client.size()))
			_attach_dataspace();
	}
	else { return; }
}


/*
*	Pdataspace_component --- Destructor
*	
*	\param : -
*
*	\return : -
*
*	\functionality: -
*/

Pdataspace_component::~Pdataspace_component()
{
	detach_from_prm_session();
	_receiver.dissolve(&_context);
}
