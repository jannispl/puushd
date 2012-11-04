class ListUsersPage;

#ifndef _LISTUSERSPAGE_H
#define _LISTUSERSPAGE_H

#include "../Page.h"

class ListUsersPage : Page
{
public:
	ListUsersPage(PuushServer *server);

	void handleRequest(mg_connection *connection, const mg_request_info *info, const std::string &uri);

private:
	void sendScripts(std::stringstream &content);
	void doDelete(mg_connection *conn, int id);
};

#endif
