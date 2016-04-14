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

	Dataspace_capability ds = env()->ram_session()->alloc(4096);
	PDBG("Pdataspace allocated");

	void *addr = env()->rm_session()->attach(ds);
	PDBG("Pdataspace attached to virtual memory");

	PINF("This Component won't be started as a fault occurs in the Fiasco.OC kernel");

	return 0;
}
