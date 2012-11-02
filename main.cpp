#include "stdafx.h"
#include "PuushServer.h"
#include "PuushDatabase.h"
#include "Configuration.h"

int main(int argc, char* argv[])
{	
	Configuration config;
	if (!config.load("puushd.conf"))
	{
		std::cerr << "Unable to load puushd.conf" << std::endl;
		return 1;
	}
	
	std::string databaseFile = config.getString("dbfile", "puushd.db");
	
	PuushDatabase db;
	if (!db.load("test.db"))
	{
		std::cerr << "Unable to load database (" << databaseFile << ")" << std::endl;
		return 1;
	}
	
#ifdef WIN32
	if (argc >= 4 && _stricmp(argv[1], "adduser") == 0)
#else
	if (argc >= 4 && strcasecmp(argv[1], "adduser") == 0)
#endif
	{
		std::cout << "Adding user '" << argv[2] << "'/'" << argv[3] << "'..." << std::endl;
		std::string apiKey = db.addUser(argv[2], argv[3]);
		std::cout << "Done. API key: " << apiKey << std::endl;
		return 1;
	}
	
	std::cout << "Loaded database from " << databaseFile << std::endl;
	
	int port = config.getInteger("port", 1200);
	
	std::cout << "Starting server on port " << port << "..." << std::endl;
	
	PuushServer server(&config, &db);
	server.start(port);

	getchar();

	std::cout << "Stopping server..." << std::endl;
	
	server.stop();

	return 0;
}
