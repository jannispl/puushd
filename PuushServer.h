class PuushServer;

#ifndef _PUUSHSERVER_H
#define _PUUSHSERVER_H

#include <random>
#include "PuushDatabase.h"

class PuushServer
{
public:
	PuushServer(PuushDatabase *database);
	~PuushServer();

	void start(int port);
	void stop();

private:
	static void *forwardMongooseEvent(mg_event ev, mg_connection *conn);
	void *handleMongooseEvent(mg_event ev, mg_connection *conn);

	bool tryServeFile(mg_connection *conn, const mg_request_info *info, const std::string &request_uri);
	void handleRequest(mg_connection *conn, const mg_request_info *info);
	void handleAuthRequest(mg_connection *conn, const mg_request_info *info);
	void handleFileUpload(mg_connection *conn, const mg_request_info *info);

	static std::string generateRandomFilename(void *userdata);

	mg_context *m_mongooseContext;
	
	std::default_random_engine m_randomGenerator;
	std::uniform_int_distribution<int> m_randomDistribution;

	PuushDatabase *m_database;
};

#endif
