class PuushDatabase;

#ifndef _PUUSHDATABASE_H
#define _PUUSHDATABASE_H

#include <list>
#include <vector>
#include <random>

#ifndef WIN32
namespace std { typedef minstd_rand0 default_random_engine; }
#endif

class PuushDatabase
{
public:
	PuushDatabase();
	~PuushDatabase();

	bool load(const char *filename);
	void close();

	std::string addUser(const char *username, const char *password);
	std::string authenticateUser(const char *username, const char *passwordOrApiKey);
	std::string addFile(const char *apiKey, const char *filename, const char *path, const char *md5hash);
	std::string lookupFile(const char *shortName, int &httpStatus);

private:
	struct QueryField
	{
		bool isNull;
		std::string value;
	};

	struct QueryResult
	{
		std::list<std::vector<QueryField> > rows;
		std::vector<std::string> columns;
	};

	std::string execute(const char *query, QueryResult *destResult);
	static int queryCallback(void *userdata, int argc, char **argv, char **columns);

	std::string generateApiKey();
	std::string generateShortName();

	sqlite3 *m_database;

	std::random_device m_randomDevice;
	std::default_random_engine m_randomGenerator;
#if defined(WIN32) || (defined(GCC_VERSION) && GCC_VERSION >= 40500)
	std::uniform_int_distribution<int> m_randomDistribution;
#else
	std::uniform_int<int> m_randomDistribution;
#endif
};

#endif
