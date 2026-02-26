#include <windows.h>
#include <string>
#include <stdexcept>
#include <vector>
#include <system_error>
#include <chrono>
#include <iomanip>
#include <sstream>

#include "file_io.h"

namespace Util
{

    void FileAppender::throwLastError(const std::string &operation) const
    {
        DWORD error = GetLastError();
        throw std::system_error(static_cast<int>(error),
                                std::system_category(),
                                "Failed to " + operation + " file: " + filename);
    }

    FileAppender &FileAppender::operator=(FileAppender &&other) noexcept
    {
        if (this != &other)
        {
            if (_isOpen)
                CloseHandle(hFile);
            hFile = other.hFile;
            _isOpen = other._isOpen;
            filename = std::move(other.filename);
            other.hFile = INVALID_HANDLE_VALUE;
            other._isOpen = false;
        }
        return *this;
    }

    void FileAppender::open()
    {
        if (_isOpen)
            return;

        hFile = CreateFileA(
            filename.c_str(),
            FILE_APPEND_DATA,
            FILE_SHARE_READ,
            nullptr,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, // Optional: immediate write
            nullptr);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            throwLastError("open");
        }

        _isOpen = true;
    }

    void FileAppender::close()
    {
        if (_isOpen)
        {
            flush();
            CloseHandle(hFile);
            hFile = INVALID_HANDLE_VALUE;
            _isOpen = false;
        }
    }

    void FileAppender::clear()
    {
        if (!_isOpen)
            return;

        // Close the file
        close();

        // Reopen with truncation flag
        hFile = CreateFileA(
            filename.c_str(),
            FILE_APPEND_DATA,
            FILE_SHARE_READ,
            nullptr,
            CREATE_ALWAYS, // Always create new file (truncates existing)
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            throwLastError("clear");
        }

        _isOpen = true;
    }

    // Write methods with return values
    size_t FileAppender::write(const std::string &data)
    {
        return writeBinary(data.c_str(), data.size());
    }

    size_t FileAppender::writeLine(const std::string &data)
    {
        return write(data + "\r\n");
    }

    size_t FileAppender::writeBinary(const void *data, size_t size)
    {
        if (!_isOpen || !data || size == 0)
            return 0;

        DWORD bytesWritten;
        BOOL result = WriteFile(
            hFile,
            data,
            static_cast<DWORD>(size),
            &bytesWritten,
            nullptr);

        if (!result)
        {
            throwLastError("write");
        }

        return bytesWritten;
    }

    // Write with timestamp
    size_t FileAppender::writeWithTimestamp(const std::string &data)
    {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "] " << data;
        return writeLine(ss.str());
    }

    // Write multiple items
    template <typename... Args>
    size_t FileAppender::writeAll(Args &&...args)
    {
        return (write(std::forward<Args>(args)) + ...);
    }

    bool FileAppender::flush()
    {
        if (!_isOpen)
            return false;
        return FlushFileBuffers(hFile) != 0;
    }

    // Get file info
    size_t FileAppender::size() const
    {
        if (!_isOpen)
            return 0;

        LARGE_INTEGER size;
        if (GetFileSizeEx(hFile, &size))
        {
            return static_cast<size_t>(size.QuadPart);
        }
        return 0;
    }

    bool FileAppender::isOpen() const { return _isOpen; }
    const std::string & FileAppender::getFilename() const { return filename; }

    // Seek to end (useful for ensuring we're at the end)
    bool FileAppender::seekToEnd()
    {
        if (!_isOpen)
            return false;

        LARGE_INTEGER distance = {0};
        return SetFilePointerEx(hFile, distance, nullptr, FILE_END) != 0;
    }
}