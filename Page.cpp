#include "stdafx.h"
#include "Page.h"

Page::Page(PuushServer *server)
	: m_server(server)
{
}

PuushServer *Page::getServer()
{
	return m_server;
}
