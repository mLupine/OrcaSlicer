#ifndef slic3r_LibraryContext_hpp_
#define slic3r_LibraryContext_hpp_

#include <string>

namespace Slic3r {

class LibraryContext {
public:
    LibraryContext(
        std::string data_dir,
        std::string resources_dir,
        std::string temporary_dir
    );

    LibraryContext(const LibraryContext&) = default;
    LibraryContext(LibraryContext&&) = default;
    LibraryContext& operator=(const LibraryContext&) = default;
    LibraryContext& operator=(LibraryContext&&) = default;

    const std::string& data_dir() const { return m_data_dir; }
    const std::string& resources_dir() const { return m_resources_dir; }
    const std::string& temporary_dir() const { return m_temporary_dir; }

    std::string custom_shapes_dir() const;
    std::string profiles_dir() const;

    std::string data_path(const std::string& relative) const;
    std::string resource_path(const std::string& relative) const;
    std::string temp_path(const std::string& relative) const;

private:
    std::string m_data_dir;
    std::string m_resources_dir;
    std::string m_temporary_dir;
};

} // namespace Slic3r

#endif // slic3r_LibraryContext_hpp_
