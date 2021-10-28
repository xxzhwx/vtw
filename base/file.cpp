#include "base/file.h"
#include <stdio.h>
#include <ctype.h>
#include <vector>

#ifdef _MSC_VER
#include <Windows.h>
#include <Fileapi.h>
#else
#include <sys/stat.h>
#endif // _MSC_VER

namespace vtw
{
namespace file
{

bool Exists(const std::string &path)
{
#ifdef _MSC_VER
    DWORD attr = ::GetFileAttributes(path.c_str());

    if (attr == INVALID_FILE_ATTRIBUTES)
        return false;
    return true;
#else
    if (!access(path, F_OK))
        return true;
    return false;
#endif
}

bool CreateDirectory(const std::string &path)
{
    if (path.empty())
        return false;

    size_t prePos = std::string::npos;
    std::vector<size_t> sepPoses;
    for (size_t pos = 0; pos < path.size(); ++pos)
    {
        if (path[pos] != '/' && path[pos] != '\\')
            continue;

        if ((prePos != std::string::npos) && (prePos + 1 == pos))
            return false;

        prePos = pos;
        sepPoses.push_back(pos);
    }

    if (sepPoses.empty() || *sepPoses.rbegin() != path.size() - 1)
        sepPoses.push_back(path.size());

    for (size_t i = 0; i < sepPoses.size(); ++i)
    {
        const std::string toPath = path.substr(0, sepPoses[i] + 1);
        if (Exists(toPath))
            continue;

#ifdef _MSC_VER
        if (!::CreateDirectoryA(toPath.c_str(), NULL))
            return false;
#else
        if (mkdir(toPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) // permission: rwx-rwx-rx
            return false;
#endif
    }

    return true;
}

std::string GetParentDirectory(const std::string &path)
{
    if (path.empty())
        return "";

    if (path.length() == 1) // "/"
        return "";

    // to skip the last separator
    size_t off = std::string::npos;
    if (path.back() == '\\' || path.back() == '/')
        off = path.length() - 1;

    size_t sepPos = path.rfind('\\', off);
    if (sepPos == std::string::npos)
    {
        sepPos = path.rfind('/', off);
        if (sepPos == std::string::npos)
            return "";
    }

    return path.substr(0, sepPos);
}

bool DeleteFile(const std::string &filename)
{
    if (Exists(filename))
    {
        if (remove(filename.c_str()) == 0)
            return true;
        else
            return false;
    }

    return true;
}

} // namespace file
} // namespace vtw
