#ifndef _PROXY__INCLUDE__PRAM_ROOT_H_
#define _PROXY__INCLUDE__PRAM_ROOT_H_

#include <root/component.h>
#include <base/printf.h>
#include <util/arg_string.h>
#include "pram_session_component.h"

namespace Proxy {

	class Pram_root : public Genode::Root_component<Pram_session_component>
	{
		protected:

			Pram_session_component *_create_session(const char *args)
			{
				enum { MAX_LABEL_LEN = 64 };
				char label[MAX_LABEL_LEN];
				Genode::Arg_string::find_arg(args, "label").string(label, ~0UL, "");

				unsigned long phys_start 	= Genode::Arg_string::find_arg(args, "size").ulong_value(~0UL);
				unsigned long phys_size		= Genode::Arg_string::find_arg(args, "start").ulong_value(~0UL);

				PDBG("Creating Pram_session_component...");
				return new (md_alloc()) Pram_session_component(ep(), md_alloc(), (char *)label,
																phys_start, phys_size);
			}

		public:

			Pram_root(Genode::Rpc_entrypoint *session_ep, Genode::Rpc_entrypoint *ds_ep,
						Genode::Allocator *md_alloc) 
			: Genode::Root_component<Pram_session_component>(session_ep, md_alloc)
			{
				PDBG("Creating Pram_root_component...");
			}
	};
}

#endif /* _PROXY__INCLUDE__PRAM_ROOT_H_ */
