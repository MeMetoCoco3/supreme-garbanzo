#include <string_view>
#include <optional>
#include <string>



std::string remove_double_slash(const char* path);
bool is_correct_extension(const char* file_path, std::initializer_list<const char*> extensions);
const char* get_file_extension(const char* filename);

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


