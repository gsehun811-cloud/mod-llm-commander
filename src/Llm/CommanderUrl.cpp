/* GPL-2.0-or-later. HTTPS base URL parsing shared by both providers. */
#include "CommanderHttpsClient.h"

#include <algorithm>
#include <cctype>

namespace LlmCommander
{
bool SplitUrl(std::string const& url, std::string& host, std::string& port, std::string& path)
{
    host.clear();
    port = "443";
    path.clear();
    if (url.rfind("https://", 0) != 0 || url.find_first_of("\r\n\t @?#") != std::string::npos)
        return false;
    auto const slash = url.find('/', 8);
    std::string authority = url.substr(8, slash == std::string::npos ? slash : slash - 8);
    if (slash != std::string::npos)
        path = url.substr(slash);
    while (!path.empty() && path.back() == '/')
        path.pop_back();
    if (!authority.empty() && authority.front() == '[')
    {
        auto const close = authority.find(']');
        if (close == std::string::npos)
            return false;
        host = authority.substr(1, close - 1);
        if (close + 1 < authority.size())
        {
            if (authority[close + 1] != ':')
                return false;
            port = authority.substr(close + 2);
        }
    }
    else
    {
        auto const colon = authority.find(':');
        host = authority.substr(0, colon);
        if (colon != std::string::npos)
            port = authority.substr(colon + 1);
    }
    if (host.empty() || port.empty() || port.size() > 5 ||
        !std::all_of(port.begin(), port.end(), [](unsigned char c) { return std::isdigit(c); }))
        return false;
    unsigned long const portNumber = std::stoul(port);
    return portNumber > 0 && portNumber <= 65535;
}
}
