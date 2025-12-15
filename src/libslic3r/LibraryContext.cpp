#include "LibraryContext.hpp"
#include <boost/filesystem/path.hpp>

namespace Slic3r {

LibraryContext::LibraryContext(
    std::string data_dir,
    std::string resources_dir,
    std::string temporary_dir
)
    : m_data_dir(std::move(data_dir))
    , m_resources_dir(std::move(resources_dir))
    , m_temporary_dir(std::move(temporary_dir))
{
}

std::string LibraryContext::custom_shapes_dir() const
{
    return (boost::filesystem::path(m_data_dir) / "shapes").string();
}

std::string LibraryContext::profiles_dir() const
{
    return (boost::filesystem::path(m_resources_dir) / "profiles").string();
}

std::string LibraryContext::data_path(const std::string& relative) const
{
    return (boost::filesystem::path(m_data_dir) / relative).string();
}

std::string LibraryContext::resource_path(const std::string& relative) const
{
    return (boost::filesystem::path(m_resources_dir) / relative).string();
}

std::string LibraryContext::temp_path(const std::string& relative) const
{
    return (boost::filesystem::path(m_temporary_dir) / relative).string();
}

} // namespace Slic3r
