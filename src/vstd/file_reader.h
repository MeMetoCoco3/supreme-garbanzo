
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>
#include <optional>
#include <string>

// Code from @ChShers
// This struct is meant to be used on Small files
struct FileReader {
    // string_view is a reference to a piece of text data
        std::string_view contents() const noexcept; // says that no exceptions will be thrown used with standard library can improve performance if used on a move constructor
        static std::optional<FileReader> open(std::string_view path);
    private: 
        // Explicit means that we require a std::string, not something that can be converted to std::string
        explicit FileReader(std::string contents);
        std::string m_contents;
};

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

