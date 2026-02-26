#pragma once

namespace Util {

    class FileAppender
    {
    private:
        HANDLE hFile;
        bool _isOpen;
        std::string filename;

        void throwLastError(const std::string &operation) const;

    public:
        // Constructor with options
        explicit FileAppender(const std::string &filename, bool createPath = false)
            : hFile(INVALID_HANDLE_VALUE), _isOpen(false), filename(filename)
        {

            if (createPath)
            {
                // Create directory structure if needed
                size_t pos = filename.find_last_of("\\/");
                if (pos != std::string::npos)
                {
                    std::string path = filename.substr(0, pos);
                    CreateDirectoryA(path.c_str(), nullptr); // Ignores if exists
                }
            }

            open();
        }

        ~FileAppender()
        {
            if (_isOpen)
            {
                CloseHandle(hFile);
            }
        }

        // Move operations
        FileAppender(FileAppender &&other) noexcept
            : hFile(other.hFile), _isOpen(other._isOpen), filename(std::move(other.filename))
        {
            other.hFile = INVALID_HANDLE_VALUE;
            other._isOpen = false;
        }

        FileAppender &operator=(FileAppender &&other) noexcept;

        // Prevent copying
        FileAppender(const FileAppender &) = delete;
        FileAppender &operator=(const FileAppender &) = delete;

        void open();
        void close();

        // Clear the file contents (truncate to 0)
        void clear();
        // Write methods with return values
        size_t write(const std::string &data);
        size_t writeLine(const std::string &data);
        size_t writeBinary(const void *data, size_t size);

        // Write with timestamp
        size_t writeWithTimestamp(const std::string &data);

        // Write multiple items
        template <typename... Args>
        size_t writeAll(Args &&...args);
        

        bool flush();

        // Get file info
        size_t size() const;

        bool isOpen() const;
        const std::string &getFilename() const;

        // Seek to end (useful for ensuring we're at the end)
        bool seekToEnd();
        
    };
}