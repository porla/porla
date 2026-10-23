#include "secretkey.hpp"

#include <cerrno>
#include <cstring>
#include <fstream>
#include <stdexcept>

#include <boost/log/trivial.hpp>
#include <fcntl.h>
#include <sodium.h>
#include <unistd.h>

using porla::Utils::SecretKey;

namespace fs = std::filesystem;

namespace
{
    static std::string kAllowedChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    std::string New(int length)
    {
        std::string key;
        key.resize(length);

        for (char& c : key)
        {
            c = kAllowedChars[randombytes_uniform(kAllowedChars.size())];
        }

        return key;
    }

    std::string Read(const fs::path& file)
    {
        if (std::error_code ec; !fs::is_regular_file(file, ec))
        {
            throw std::runtime_error("Secret key file " + file.string() + " is not a regular file");
        }

        std::ifstream stream(file, std::ios::binary);

        if (!stream)
        {
            throw std::runtime_error("Failed to read secret key file " + file.string());
        }

        std::string key(
            (std::istreambuf_iterator<char>(stream)),
            std::istreambuf_iterator<char>());

        while (!key.empty() && std::isspace(static_cast<unsigned char>(key.back())))
        {
            key.pop_back();
        }

        if (key.empty())
        {
            throw std::runtime_error("Secret key file " + file.string() + " is empty - delete it to generate a new key");
        }

        std::error_code ec;
        const auto perms = fs::status(file, ec).permissions();

        if (!ec && (perms & (fs::perms::group_all | fs::perms::others_all)) != fs::perms::none)
        {
            BOOST_LOG_TRIVIAL(warning) << "Secret key file " << file << " is accessible by other users - chmod 600 it";
        }

        return key;
    }
}

std::string SecretKey::Load(const fs::path& file, int length)
{
    // O_EXCL creates the file with 0600 from the start (no umask or chmod window), and fails if
    // another process created it first - then read that one.
    const int fd = open(file.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);

    if (fd < 0)
    {
        if (errno != EEXIST)
        {
            throw std::runtime_error("Failed to create secret key file " + file.string() + ": " + std::strerror(errno));
        }

        BOOST_LOG_TRIVIAL(debug) << "Using secret key from " << file;

        return Read(file);
    }

    const auto key = New(length);

    const bool written = write(fd, key.data(), key.size()) == static_cast<ssize_t>(key.size())
        && fsync(fd) == 0;

    const int write_errno = errno;

    close(fd);

    if (!written)
    {
        fs::remove(file);
        throw std::runtime_error(
            "Failed to write secret key file " + file.string() + ": " + std::strerror(write_errno));
    }

    BOOST_LOG_TRIVIAL(info) << "Generated secret key at " << file;

    return key;
}
