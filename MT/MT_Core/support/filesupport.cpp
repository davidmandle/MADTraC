#include "filesupport.h"

#include <sys/stat.h>

#include "stringsupport.h"

#ifndef _WIN32
#else
  #include <direct.h>
  #define mkdir(a,b) _mkdir(a)
#endif

bool MT_CatTextFile(const char* filename)
{
    std::string file_text = MT_TextFileToString(filename);

    if(file_text != std::string(MT_FILE_READ_ERROR_STRING))
    {
        std::cout << file_text;
        return true;
    }
    else
    {
        return false;
    }
}

std::string MT_TextFileToString(const char* filename)
{
    std::vector<std::string> lines = MT_TextFileToStringVector(filename);

    if(lines.size() == 0)
    {
        return std::string(MT_FILE_READ_ERROR_STRING);
    }
    else
    {
        return MT_StringVectorToString(lines);
    }
    
}

std::vector<std::string> MT_TextFileToStringVector(const char* filename)
{
    std::vector<std::string> lines;
    std::string line;
    lines.resize(0);

    std::ifstream readfile(filename);

    if(readfile.is_open())
    {
        while(!readfile.eof())
        {
            getline(readfile, line);
            lines.push_back(line);
        }
        readfile.close();
    }

    return lines;

}


std::string MT_GetFileExtension(const std::string& path)
{
    std::string::size_type idx = path.rfind('.');
    std::string::size_type idx2 = path.rfind(MT_PathSeparator);
    std::string return_value = "";

    if(idx != std::string::npos
       && (idx2 == std::string::npos || (idx2 < idx)))
    {
        return_value = path.substr(idx+1);
    }

    return return_value;
}

bool MT_PathHasFileExtension(const char* path, const char* extension)
{
    std::string ext(extension);
    std::string actual = MT_GetFileExtension(std::string(path));

    return (actual == ext);
}

bool MT_FileIsAvailable(const char* name, const char* method)
{
  FILE* tmp_file = fopen(name, method);
  if(tmp_file)
  {
    fclose(tmp_file);
    return true;
  }
  else
  {
    return false;
  }
}

void MT_mkdir(const char* dir_name, unsigned int mode)
{
    struct stat st;
    if(stat(dir_name, &st) == 0)
    {
        return;
    }
    else
    {
        mkdir(dir_name, mode);
    }
}
