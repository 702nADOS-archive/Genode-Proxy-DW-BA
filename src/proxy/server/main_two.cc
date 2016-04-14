/* Genode includes */
#include <base/printf.h>
#include <base/sleep.h>
#include <base/child.h>
#include <util/arg_string.h>
#include <cap_session/connection.h>
#include <ram_session/connection.h>
#include <rm_session/connection.h>
#include <rom_session/connection.h>
#include <pd_session/connection.h>
#include <cpu_session/connection.h>
#include <pram_session/connection.h>
#include <prm_session/connection.h>

namespace Fiasco{
	#include <l4/sys/kdebug.h>
}

/* Proxy includes*/
#include "include/prm_root.h"
#include "include/pram_root.h"

using namespace Genode;

int main(void)
{
	PDBG("--- start proxy_server ---\n");

	PDBG("--- announcing proxy services --- ");

	Cap_connection cap;

	static Sliced_heap sliced_heap(env()->ram_session(),
									env()->rm_session());

	enum { STACK_SIZE = 16*1024 };
	static Rpc_entrypoint ep(&cap, STACK_SIZE, "proxy_ep");

	static Proxy::Prm_root prm_root(&ep, &ep, &sliced_heap);
	PDBG("PRM_root created and back in server-main!");

	static Proxy::Pram_root pram_root(&ep, &ep, &sliced_heap);
	PDBG("PRAM_root created and back in server-main!");

	env()->parent()->announce(ep.manage(&prm_root));
	PDBG("PRM service announced!");

	env()->parent()->announce(ep.manage(&pram_root));
	PDBG("PRAM service announced!");

	PDBG("--- proxy_server is going to sleep ---");
	sleep_forever();
	PDBG("Server still awake - This output should not be reached!");

	return 0;
}

