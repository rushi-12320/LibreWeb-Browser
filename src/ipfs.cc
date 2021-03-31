#include "ipfs.h"
#include <sstream>

/**
 * \brief IPFS Contructor, connect to IPFS
 */
IPFS::IPFS(const std::string &host, int port) : client(host, port, "6s") {}

/**
 * \brief Get the number of IPFS peers
 * \return number of peers
 */
std::size_t IPFS::getNrPeers()
{
    try
    {
        ipfs::Json peers;
        client.SwarmPeers(&peers);
        return peers["Peers"].size();
    }
    catch (const std::runtime_error &error)
    {
        // ignore connection issues
    }
    return 0;
}

/**
 * \brief Get the number of IPFS peers
 * \return Map with bandwidth information (with keys: 'in' and 'out')
 */
std::map<std::string, float> IPFS::getBandwidthRates()
{
    std::map<std::string, float> bandwidthRates;
    try
    {
        ipfs::Json bandwidth_info;
        client.StatsBw(&bandwidth_info);
        float in = bandwidth_info["RateIn"];
        float out = bandwidth_info["RateOut"];
        bandwidthRates.insert(std::pair<std::string, float>("in", in));
        bandwidthRates.insert(std::pair<std::string, float>("out", out));
    }
    catch (const std::runtime_error &error)
    {
        // ignore connection issues
    }
    return bandwidthRates;
}

/**
 * \brief Fetch file from IFPS network (create a new client object each time - which is thread-safe), static method
 * \param path File path
 * \throw std::runtime_error when there is a connection-time/something goes wrong while trying to get the file
 * \return content as string
 */
std::string const IPFS::fetch(const std::string &path)
{
    ipfs::Client client("localhost", 5001, "6s");
    std::stringstream contents;
    client.FilesGet(path, &contents);
    return contents.str();
}

/**
 * \brief Publish file to IPFS network (not thread-safe)
 * \param filename Filename that gets stored in IPFS
 * \param content Content that needs to be written to the IPFS network
 * \return IPFS content-addressed identifier (CID) hash
 */
std::string const IPFS::publish(const std::string &filename, const std::string &content)
{
    try
    {
        ipfs::Json result;
        // Publish a single file
        ipfs::http::FileUpload file = {filename, ipfs::http::FileUpload::Type::kFileContents, content};
        client.FilesAdd({file}, &result);
        if (result.is_array())
        {
            for (const auto &files : result.items())
            {
                return files.value()["hash"];
            }
        }
        // something is wrong, fallback
        return "";
    }
    catch (const std::runtime_error &error)
    {
        // ignore connection issues
    }
    return ""; // empty string, something went wrong
}
