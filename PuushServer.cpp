#include "stdafx.h"
#include "PuushServer.h"
#include "MPFDParser/Parser.h"
#include <time.h>
#ifndef WIN32
#include <sys/stat.h>
#endif
#include "pages/ListUsersPage.h"

PuushServer::PuushServer(Configuration *config, PuushDatabase *database)
	: m_mongooseContext(NULL), m_randomDistribution(0, (10 + 26 + 26) - 1), m_randomGenerator(m_randomDevice()), m_config(config), m_database(database)
{
	m_puushUrl = config->getString("puushurl", "http://localhost:1200/");
	
	std::cout << "Using puush URL: " << m_puushUrl << "[short]" << std::endl;
}

PuushServer::~PuushServer()
{
	stop();
}

void PuushServer::start(int port)
{
	static char port_str[16];
#ifdef WIN32
	sprintf_s(port_str, "%d", port);
#else
	sprintf(port_str, "%d", port);
#endif

	const char *options[] =
	{
		"listening_ports", port_str,
		NULL
	};

	m_mongooseContext = mg_start(&PuushServer::forwardMongooseEvent, this, options);
}

void PuushServer::stop()
{
	if (m_mongooseContext != NULL)
	{
		mg_stop(m_mongooseContext);
		m_mongooseContext = NULL;
	}
}

PuushDatabase *PuushServer::getDatabase()
{
	return m_database;
}

void *PuushServer::forwardMongooseEvent(mg_event ev, mg_connection *conn)
{
	PuushServer *instance = (PuushServer *) mg_get_user_data(conn);
	return instance->handleMongooseEvent(ev, conn);
}

void *PuushServer::handleMongooseEvent(mg_event ev, mg_connection *conn)
{
	if (ev == MG_EVENT_LOG)
	{
		std::cout << "Mongoose event log: " << mg_get_log_message(conn) << std::endl;
	}
	else if (ev == MG_HTTP_ERROR)
	{
		std::cout << "Mongoose had to send HTTP error: status " << mg_get_reply_status_code(conn) << std::endl;
	}
	else if (ev == MG_NEW_REQUEST)
	{
		const mg_request_info *info = mg_get_request_info(conn);

		handleRequest(conn, info);
		return (void *) "";
	}
	
	return NULL;
}

bool PuushServer::tryServeFile(mg_connection *conn, const mg_request_info *info, const std::string &request_uri)
{
	std::string url = request_uri;
	if (url.at(0) == '/')
		url = url.substr(1);

	// see if there's another slash, if so then this is not a file request
	if (url.find('/') != std::string::npos)
		return false;

	int httpStatus;
	std::string path = m_database->lookupFile(url.c_str(), httpStatus);
	if (httpStatus == 200 && !path.empty())
	{
		mg_send_file(conn, path.c_str());
		return true;
	}

	return false;
}

void PuushServer::handleRequest(mg_connection *conn, const mg_request_info *info)
{
	std::string request_uri = info->uri;
	std::string puush_url = "http://puush.me";

	if (request_uri.length() > puush_url.length() && request_uri.substr(0, puush_url.length()) == puush_url)
	{
		request_uri = request_uri.substr(puush_url.length());
	}

	if (request_uri.at(0) == '/')
		request_uri = request_uri.substr(1);

	std::string::size_type idx = request_uri.find('/');
	std::string first_portion = idx == std::string::npos ? request_uri : request_uri.substr(0, idx);

	if (request_uri == "dl/puush-win.txt")
	{
		mg_response(conn, 200, "OK", "81", "text/plain");
	}
	else if (first_portion == "listusers")
	{
		ListUsersPage page(this);
		page.handleRequest(conn, info, request_uri);
	}
	else if (request_uri == "api/auth")
	{
		handleAuthRequest(conn, info);
	}
	else if (request_uri == "api/up")
	{
		handleFileUpload(conn, info);
	}
	else if (!tryServeFile(conn, info, request_uri))
	{
		std::cerr << "[debug] other request: " << request_uri << std::endl;

		mg_response(conn, 200, "OK", "I don't think there's anything I can do for you.", "text/html");
	}
}

void PuushServer::handleAuthRequest(mg_connection *conn, const mg_request_info *info)
{
#ifdef WIN32
	if (_stricmp(info->request_method, "POST") != 0)
#else
	if (strcasecmp(info->request_method, "POST") != 0)
#endif
	{
		mg_response(conn, 400, "Bad request", "Bad request", "text/plain");
		return;
	}

	std::string post_data;
	char buf[2048];
	int read;
	do
	{
		read = mg_read(conn, buf, sizeof(buf));
		if (read)
		{
			post_data.append(buf, read);
		}
	}
	while (read > 0);

	std::map<std::string, std::string> form_values;

    // Skip delimiters at beginning.
    std::string::size_type lastPos = post_data.find_first_not_of("&", 0);
    // Find first "non-delimiter".
    std::string::size_type pos = post_data.find_first_of("&", lastPos);

    while (std::string::npos != pos || std::string::npos != lastPos)
    {
        std::string token = post_data.substr(lastPos, pos - lastPos);
        lastPos = post_data.find_first_not_of("&", pos);
        pos = post_data.find_first_of("&", lastPos);

		std::string::size_type idx = token.find('=');
		if (idx != std::string::npos)
		{
			std::string key = token.substr(0, idx);
			std::string value = token.substr(idx + 1);

			char decoded_value[512];
			mg_url_decode(value.c_str(), value.length(), decoded_value, sizeof(decoded_value) - 1, 1);

			form_values.insert(std::pair<std::string, std::string>(key, std::string(decoded_value)));
		}
    }

	std::map<std::string, std::string>::iterator email_idx = form_values.find("e");
	std::map<std::string, std::string>::iterator password_idx = form_values.find("p");
	std::map<std::string, std::string>::iterator apikey_idx = form_values.find("k");
	if (email_idx == form_values.end() || (password_idx == form_values.end() && apikey_idx == form_values.end()))
	{
		mg_response(conn, 400, "Bad request", "Bad request", "text/plain");
		return;
	}

	std::string apikey = m_database->authenticateUser(email_idx->second.c_str(), apikey_idx != form_values.end() ? apikey_idx->second.c_str() : password_idx->second.c_str());

	if (apikey.empty())
	{
		mg_response(conn, 200, "OK", "-1", "text/plain");
		return;
	}

	// ispremium,apikey,[expireday],quotaused
	std::stringstream content;
	content << "1," << apikey << ",,0";
	mg_response(conn, 200, "OK", content.str().c_str(), "text/plain");
}

void PuushServer::handleFileUpload(mg_connection *conn, const mg_request_info *info)
{
	const char *content_type = mg_get_header(conn, "Content-Type");
	if (content_type == NULL)
	{
		mg_response(conn, 400, "Bad request", "No content-type given", "text/plain");
		return;
	}

	try
	{
#ifdef WIN32
		DWORD attributes = GetFileAttributesA("files");
		if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			CreateDirectoryA("files", NULL);
		}
#else
		struct stat st;
		if (stat("files", &st) != 0 || (st.st_mode & S_IFMT) != S_IFDIR)
		{
			mkdir("files", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		}
#endif

		MPFD::Parser *POSTParser = new MPFD::Parser();
		POSTParser->SetFilenameGenerator(&PuushServer::generateRandomFilename, this);
		POSTParser->SetMaxCollectedDataLength(20 * 1024);
		
		POSTParser->SetContentType(content_type);
		
		char input[4 * 1024];
		int read;

		do
		{
			read = mg_read(conn, input, sizeof(input));
			if (read)
			{
				POSTParser->AcceptSomeData(input, read);
			}
		}
		while (read > 0);

		// Now see what we have:
		std::map<std::string, MPFD::Field *> fields = POSTParser->GetFieldsMap();

		/*std::cout << "Have " << fields.size() << " fields" << std::endl;

		std::map<std::string, MPFD::Field *>::iterator it;
		for (it = fields.begin(); it != fields.end();it++)
		{
			if (fields[it->first]->GetType() == MPFD::Field::TextType)
			{
				std::cout << "Got text field: [" << it->first << "], value: [" << it->second->GetTextTypeContent() << "]" << std::endl;
			}
			else
			{
				std::cout << "Got file field: [" << it->first << "], Filename: [" << it->second->GetFileName() << "]" << std::endl;
				std::cout << " -> temp file saved at " << it->second->GetTempFilename() << std::endl;
			}
		}*/

		std::map<std::string, MPFD::Field *>::iterator md5hash_idx = fields.find("c");
		std::map<std::string, MPFD::Field *>::iterator file_idx = fields.find("f");
		std::map<std::string, MPFD::Field *>::iterator apikey_idx = fields.find("k");
		if (md5hash_idx == fields.end() || file_idx == fields.end() || apikey_idx == fields.end())
		{
			mg_response(conn, 400, "Bad request", "Bad request", "text/plain");
			return;
		}

		MPFD::Field *md5hash_field = md5hash_idx->second;
		MPFD::Field *file_field = file_idx->second;
		MPFD::Field *apikey_field = apikey_idx->second;
		if (md5hash_field->GetType() != MPFD::Field::TextType || file_field->GetType() != MPFD::Field::FileType || apikey_field->GetType() != MPFD::Field::TextType)
		{
			mg_response(conn, 400, "Bad request", "Bad request", "text/plain");
			return;
		}

		std::string shortName = m_database->addFile(apikey_field->GetTextTypeContent().c_str(), file_field->GetFileName().c_str(), file_field->GetTempFilename().c_str(), md5hash_field->GetTextTypeContent().c_str());
		
		if (shortName.empty())
		{
			mg_response(conn, 200, "OK", "-1", "text/plain");
			return;
		}

		std::stringstream content;
		content << "0," << m_puushUrl << shortName << ",133337,12345" << std::endl;
		mg_response(conn, 200, "OK", content.str().c_str(), "text/plain");
	}
	catch (MPFD::Exception e)
	{
		std::cout << "File upload parsing error: " << e.GetError() << std::endl;

		mg_response(conn, 500, "Internal Server Error", "Internal Server Error", "text/plain");
	}
}

std::string PuushServer::generateRandomFilename(void *userdata)
{
	PuushServer *instance = (PuushServer *) userdata;

	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	std::string str;
	str.resize(20);
	for (int i = 0; i < 20; ++i)
	{
		str[i] = alphanum[instance->m_randomDistribution(instance->m_randomGenerator)];
	}

	return "files/up_" + str;
}
