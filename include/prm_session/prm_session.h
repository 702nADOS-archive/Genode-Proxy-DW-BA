#ifndef _INCLUDE__PRM_SESSION__PRM_SESSION_H_
#define _INCLUDE__PRM_SESSION__PRM_SESSION_H_

#include <rm_session/rm_session.h>

namespace Proxy { struct Prm_session; }

struct Proxy::Prm_session : Genode::Rm_session
{
	static const char *service_name() { return "PRM"; }
};

#endif /* _INCLUDE__PRM_SESSION__PRM_SESSION_H_ */