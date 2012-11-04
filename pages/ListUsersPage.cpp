#include "stdafx.h"
#include "ListUsersPage.h"

ListUsersPage::ListUsersPage(PuushServer *server)
	: Page(server)
{
}

void ListUsersPage::sendScripts(std::stringstream &content)
{
	content << "<script type=\"text/javascript\">";
	content << "function doDelete(id) {"
			"if (confirm('Are you sure to delete this user?')) {"
				"window.location.href = '/listusers/delete?' + id;"
			"}"
			"return false;"
		"}";
	content << "</script>";
}

void ListUsersPage::doDelete(mg_connection *conn, int id)
{
	bool success = getServer()->getDatabase()->deleteUser(id);

	if (!success)
	{
		mg_response(conn, 200, "OK", "No user was deleted", "text/html");
		return;
	}

	std::stringstream content;

	content << "User was deleted.<br />";
	content << "<a href=\"/listusers\">Back</a>";

	mg_response(conn, 200, "OK", content.str().c_str(), "text/html");
}

void ListUsersPage::handleRequest(mg_connection *connection, const mg_request_info *info, const std::string &uri)
{
	std::string::size_type idx = uri.find('/');
	if (idx != std::string::npos)
	{
		std::string action = uri.substr(idx + 1);
		if (!action.empty() && action == "delete")
		{
			std::string action = info->query_string;
			if (!action.empty())
			{
				doDelete(connection, atoi(action.c_str()));
			}
			else
			{
				mg_response(connection, 200, "OK", "ID argument missing", "text/html");
			}
			return;
		}
	}

	std::stringstream content;

	std::list<PuushDatabase::User> users = getServer()->getDatabase()->getUsers();

	if (users.size() == 0)
	{
		content << "<strong>No users in database</strong>";
	}
	else
	{
		sendScripts(content);

		content << "<table border=\"1\">";
		content << "<tr>";
		content << "<td><strong>ID</strong></td>";
		content << "<td><strong>Username</strong></td>";
		content << "<td><strong>API key</strong></td>";
		content << "<td><strong>Action</strong></td>";
		content << "</tr>";

		for (std::list<PuushDatabase::User>::iterator i = users.begin(); i != users.end(); ++i)
		{
			content << "<tr>";

			content << "<td>" << i->id << "</td>";
			content << "<td>" << i->username << "</td>";
			content << "<td>" << i->apiKey << "</td>";
			content << "<td><a href=\"#\" onclick=\"doDelete(" << i->id << ");\">Delete</a></td>";

			content << "</tr>";
		}
		
		content << "</table>";
	}

	mg_response(connection, 200, "OK", content.str().c_str(), "text/html");
}
