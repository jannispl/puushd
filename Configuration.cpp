#include "stdafx.h"
#include "Configuration.h"
#include <fstream>
#include <algorithm>
#include <locale>
#include <cctype>

// damn, I'm so lazy
// trim from start
static inline void ltrim(std::string &s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
}

// trim from end
static inline void rtrim(std::string &s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

// trim from both ends
static inline void trim(std::string &s)
{
	ltrim(s);
	rtrim(s);
}

Configuration::Configuration()
{
}

Configuration::~Configuration()
{
}

bool Configuration::load(const char *filename)
{
	std::ifstream is;
	is.open(filename, std::ios_base::in);
	if (is.fail())
		return false;
	
	std::string line;
	while (!is.eof())
	{
		std::getline(is, line);
		trim(line);
		if (line.empty() || line.at(0) == '#')
			continue;
		
		std::string::size_type idx = line.find('=');
		if (idx != std::string::npos)
		{
			std::string key = line.substr(0, idx);
			trim(key);
			
			std::string value = line.substr(idx + 1);
			ltrim(value);
			
			m_config[key] = value;
		}
	}
	
	is.close();
	
	return true;
}

std::string Configuration::getString(const std::string &key, const std::string &defaultValue)
{
	std::map<std::string, std::string>::iterator it = m_config.find(key);
	if (it == m_config.end())
		return defaultValue;
	
	return it->second;
}

int Configuration::getInteger(const std::string &key, int defaultValue)
{
	std::map<std::string, std::string>::iterator it = m_config.find(key);
	if (it == m_config.end())
		return defaultValue;
	
	return atoi(it->second.c_str());
}
