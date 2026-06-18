
#include <filesystem>
#include <fstream>
#include <iostream>
#include "file_reader.h"
#include "vlogger.h"



// Returns a pointer to the file extension of the filename
const char* get_file_extension(const char* filename) {
    const char* dot = nullptr;
    for (const char* p = filename; *p != '\0'; p++) {
        if (*p == '.') dot = p;
    }
    return (dot && dot[1] != '\0') ? dot : "";
}

bool is_correct_extension(const char* file_path, std::initializer_list<const char*> extensions) {
    bool result = false;
    const char* val_extension = get_file_extension(file_path);
    if(strcmp(val_extension, "")) {
        for(const auto& extension: extensions)
        {
            if(strcmp(val_extension, extension) == 0) 
            {
                result = true;
                break;
            }
        }
    }else
        V_LOG_WARN("File %s does not have a extension", file_path);

    return result;
};

std::string remove_double_slash(const char* path)
{
    std::string s(path);
    size_t pos = s.find('\\');
    if(pos != std::string::npos)
        s.erase(pos, 1);
    return s;
}

FileReader::FileReader(std::string contents) : m_contents(std::move(contents)) {};

std::string_view FileReader::contents() const noexcept {
    return std::string_view{m_contents};
}

std::optional<FileReader> FileReader::open(std::string_view file_name) {
    std::filesystem::path path{file_name};
    // ifstream closes file on destruction.
    std::ifstream file{path};
    if(!file) return std::nullopt;


    std::stringstream buffer;
    buffer << file.rdbuf();
    return FileReader{std::move(buffer).str()};
}

