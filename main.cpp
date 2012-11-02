#include "stdafx.h"
#include "PuushServer.h"
#include "PuushDatabase.h"

int main(int argc, char* argv[])
{
	system("title ");

	PuushDatabase db;
	if (!db.load("test.db"))
	{
		printf("failed to load test.db\n");
		getchar();
		return 1;
	}

	PuushServer server(&db);
	server.start(1200);

	getchar();

	server.stop();

	return 0;
}
