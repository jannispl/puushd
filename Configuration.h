class Configuration;

#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

class Configuration
{
public:
	Configuration();
	~Configuration();
	
	bool load(const char *filename);
	
	std::string getString(const std::string &key, const std::string &defaultValue);
	int getInteger(const std::string &key, int defaultValue);
	
private:
	std::map<std::string, std::string> m_config;
};

#endif
