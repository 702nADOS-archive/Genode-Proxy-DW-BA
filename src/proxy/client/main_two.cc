#include <base/printf.h>
#include <base/env.h>
#include <ram_session/ram_session.h>
#include <prm_session/connection.h>
#include <pram_session/connection.h>
#include <rm_session/connection.h>

using namespace Genode;

int main(void)
{	
	PDBG("--- start proxy_test_client ---\n");

	Proxy::Prm_connection prm;
	PDBG("Prm_connection established -> Prm_session created");

	Proxy::Pram_connection pram;
	PDBG("Pram_connection established -> Pram_session created");

	pram.ref_account(env()->ram_session_cap());
	PDBG("Own ram_session is now ref_account of the pram_session");

	env()->ram_session()->transfer_quota(pram.ram_cap(), 32*1024);
	PDBG("Quota transfered");

	Ram_dataspace_capability ds = pram.alloc(4096);
	PDBG("Dataspace allocated");

	void *addr = prm.attach(ds);
	PDBG("Dataspace attached to Prm_session");

	prm.detach(addr);
	PDBG("Pram dataspace detached from Prm_session");

	pram.free(ds);
	PDBG("Dataspace freed");

	return 0;
}