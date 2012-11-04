class Page;

#ifndef _PAGE_H
#define _PAGE_H

#include "PuushServer.h"

class Page
{
public:
	Page(PuushServer *server);

protected:
	PuushServer *getServer();

private:
	PuushServer *m_server;
};

#endif
