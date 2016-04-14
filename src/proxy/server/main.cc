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


class Test_child : public Child_policy
{
	private:

		enum { CHILD_STACK_SIZE = 8*1024 };

		Rpc_entrypoint _ep;

		Child _child;

		Parent_service _log_service;
		Parent_service _rm_service;

	public:

		Test_child(Dataspace_capability elf_ds,
							Pd_session_capability pd,
							Ram_session_capability ram,
							Cpu_session_capability cpu,
							Rm_session_capability rm,
							Cap_session *cap)
		:
			_ep(cap, CHILD_STACK_SIZE, "child", false),
			_child(elf_ds, pd, ram, cpu, rm, &_ep, this),
			_log_service("LOG"),
			_rm_service("RM")
			{
				_ep.activate();
			}


		/****************************
		 ** Child-policy interface **
		 ****************************/

		const char *name() const { return "proxy_test_client"; }

		Service *resolve_session_request(const char *service_name, const char *args)
		{
			return !strcmp(service_name, "LOG") ? &_log_service
			     : !strcmp(service_name, "RM")  ? &_rm_service
			     : 0;
		}

		void filter_session_args(const char *service, char *args, size_t args_len)
		{
			Arg_string::set_arg(args, args_len, "label", "child");
		}
};



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

	PDBG("--- starting test child ---");

	// creating sessions for child's environment 

	static Rom_connection rom("proxy_test_client");
	static Pd_connection pd;
	static Proxy::Pram_connection pram;
	static Cpu_connection cpu;
	static Proxy::Prm_connection prm;
	static Cap_connection child_cap;

	// declaring ref account for child's ram session and transferring quota 

	enum { CHILD_QUOTA = 1024*1024 };
	pram.ref_account(env()->ram_session_cap());
	env()->ram_session()->transfer_quota(pram.ram_cap(), CHILD_QUOTA);

	// creating test child 

	Test_child proxy_test_child(rom.dataspace(), pd.cap(), pram.cap(), cpu.cap(), prm.cap(), &child_cap);

	PDBG("--- proxy_server is going to sleep ---");
	sleep_forever();
	PDBG("Server still awake - This output should not be reached!");

	return 0;
}
