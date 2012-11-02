// This file is distributed under GPLv3 licence
// Author: Gorelov Grigory (gorelov@grigory.info)
//
// Contacts and other info are on the WEB page:  grigory.info/MPFDParser

#include "Field.h"
#include "Parser.h"

MPFD::Field::Field() {
    type = 0;
    FieldContent = NULL;

    FieldContentLength = 0;

}

MPFD::Field::~Field() {

    if (FieldContent) {
	delete FieldContent;
    }

    if (type == FileType) {
	if (file.is_open()) {
	    file.close();
	    remove((TempDir + "/" + TempFile).c_str());
	}

    }

}

void MPFD::Field::SetType(int type) {
    if ((type == TextType) || (type == FileType)) {
	this->type = type;
    } else {
	throw MPFD::Exception("Trying to set type of field, but type is incorrect.");
    }

}

int MPFD::Field::GetType() {
    if (type > 0) {
	return type;
    } else {
	throw MPFD::Exception("Trying to get type of field, but no type was set.");
    }
}

void MPFD::Field::AcceptSomeData(char *data, long length) {
    if (type == TextType) {
	if (FieldContent == NULL) {
	    FieldContent = new char[length + 1];
	} else {
	    FieldContent = (char*) realloc(FieldContent, FieldContentLength + length + 1);
	}

	memcpy(FieldContent + FieldContentLength, data, length);
	FieldContent[FieldContentLength + length] = 0;
    } else if (type == FileType) {
	if (WhereToStoreUploadedFiles == Parser::StoreUploadedFilesInFilesystem) {
		if (!file.is_open()) {
		    std::ifstream testfile;
		    std::string tempfile;
		    do {
				if (testfile.is_open()) {
					testfile.close();
				}

				if (FilenameGenerator == NULL)
				{
					char rnd[21];
					static const char alphanum[] =
						"0123456789"
						"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
						"abcdefghijklmnopqrstuvwxyz";
					for (int i = 0; i < 20; ++i) {
						rnd[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
					}
					rnd[20] = 0;
					TempFile = std::string("upload_") + rnd;
				}
				else
				{
					typedef std::string (* FilenameGenerator_t)(void *);
					TempFile = ((FilenameGenerator_t) FilenameGenerator)(FilenameGeneratorUserData);
				}

				if (TempDir.empty())
					tempfile = TempFile;
				else
					tempfile = TempDir + "/" + TempFile;

				testfile.open(tempfile.c_str(), std::ios::in);
		    } while (testfile.is_open());

		    file.open(tempfile.c_str(), std::ios::out | std::ios::binary | std::ios_base::trunc);
		}

		if (file.is_open()) {
		    file.write(data, length);
		    file.flush();
		} else {
		    throw Exception(std::string("Cannot write to file ") + TempDir + "/" + TempFile);
		}
	} else { // If files are stored in memory
	    if (FieldContent == NULL) {
		FieldContent = new char[length];
	    } else {
		FieldContent = (char*) realloc(FieldContent, FieldContentLength + length);
	    }
	    memcpy(FieldContent + FieldContentLength, data, length);
	    FieldContentLength += length;
	}
    } else {
	throw MPFD::Exception("Trying to AcceptSomeData but no type was set.");
    }
}

void MPFD::Field::SetTempDir(std::string dir) {
    TempDir = dir;
}

void MPFD::Field::SetFilenameGenerator(void *func, void *userdata)
{
	FilenameGenerator = func;
	FilenameGeneratorUserData = userdata;
}

unsigned long MPFD::Field::GetFileContentSize() {
    if (type == 0) {
	throw MPFD::Exception("Trying to get file content size, but no type was set.");
    } else {
	if (type == FileType) {
	    if (WhereToStoreUploadedFiles == Parser::StoreUploadedFilesInMemory) {
		return FieldContentLength;
	    } else {
		throw MPFD::Exception("Trying to get file content size, but uploaded files are stored in filesystem.");
	    }
	} else {
	    throw MPFD::Exception("Trying to get file content size, but the type is not file.");
	}
    }
}

char * MPFD::Field::GetFileContent() {
    if (type == 0) {
	throw MPFD::Exception("Trying to get file content, but no type was set.");
    } else {
	if (type == FileType) {
	    if (WhereToStoreUploadedFiles == Parser::StoreUploadedFilesInMemory) {
		return FieldContent;
	    } else {
		throw MPFD::Exception("Trying to get file content, but uploaded files are stored in filesystem.");
	    }
	} else {
	    throw MPFD::Exception("Trying to get file content, but the type is not file.");
	}
    }
}

std::string MPFD::Field::GetTextTypeContent() {
    if (type == 0) {
	throw MPFD::Exception("Trying to get text content of the field, but no type was set.");
    } else {
	if (type != TextType) {
	    throw MPFD::Exception("Trying to get content of the field, but the type is not text.");
	} else {
	    if (FieldContent == NULL) {
		return std::string();
	    } else {
		return std::string(FieldContent);
	    }
	}
    }
}

std::string MPFD::Field::GetTempFileName() {
    if (type == 0) {
	throw MPFD::Exception("Trying to get file temp name, but no type was set.");
    } else {
	if (type == FileType) {
	    if (WhereToStoreUploadedFiles == Parser::StoreUploadedFilesInFilesystem) {
			return std::string(TempDir.empty() ? TempFile : TempDir + "/" + TempFile);
	    } else {
		throw MPFD::Exception("Trying to get file temp name, but uplaoded files are stored in memory.");
	    }
	} else {
	    throw MPFD::Exception("Trying to get file temp name, but the type is not file.");
	}
    }
}

std::string MPFD::Field::GetFileName() {
    if (type == 0) {
	throw MPFD::Exception("Trying to get file name, but no type was set.");
    } else {
	if (type == FileType) {
	    return FileName;
	} else {
	    throw MPFD::Exception("Trying to get file name, but the type is not file.");
	}
    }
}

void MPFD::Field::SetFileName(std::string name) {
    FileName = name;

}

void MPFD::Field::SetUploadedFilesStorage(int where) {
    WhereToStoreUploadedFiles = where;
}

void MPFD::Field::SetFileContentType(std::string type) {
    FileContentType = type;
}

std::string MPFD::Field::GetFileMimeType() {
    if (type == 0) {
	throw MPFD::Exception("Trying to get mime type of file, but no type was set.");
    } else {
	if (type != FileType) {
	    throw MPFD::Exception("Trying to get mime type of the field, but the type is not File.");
	} else {
	    return std::string(FileContentType);
	}
    }
}