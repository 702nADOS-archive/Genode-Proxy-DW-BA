#ifndef _PROXY_INCLUDE__PRM_ROOT_H_
#define _PROXY_INCLUDE__PRM_ROOT_H_

#include <base/printf.h>
#include <root/component.h>
#include <util/arg_string.h>
#include "prm_session_component.h"

namespace Proxy {

	class Prm_root : public Genode::Root_component<Prm_session_component>
	{
		protected:

			Prm_session_component *_create_session(const char *args)
			{
				Genode::addr_t start	= Genode::Arg_string::find_arg(args, "start").ulong_value(~0UL);
				Genode::size_t size 	= Genode::Arg_string::find_arg(args, "size").ulong_value(0);

				PDBG("Creating Prm_session_component...");
				return new (md_alloc()) Prm_session_component(ep(), md_alloc(), start, size);
			}

		public:

			Prm_root(Genode::Rpc_entrypoint *session_ep, Genode::Rpc_entrypoint *ds_ep,
						Genode::Allocator *md_alloc)
			: Genode::Root_component<Prm_session_component>(session_ep, md_alloc)
			{
				PDBG("Creating Prm_root_component...");
			}
	};
}

#endif /* _PROXY_INCLUDE__PRM_ROOT_H_ */
