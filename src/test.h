#pragma once

#include <windows.h>
#include <memoryapi.h>
#include <string>
#include <string_view>
#include <functional>
#include <iostream>
#include <intrin.h>



namespace Test {


    class MemoryMappedFile
    {
    public:
        MemoryMappedFile() = default;

        MemoryMappedFile(const std::string &path)
        {
            open(path);
        }


        bool open(const std::string &path);
        void close();

        ~MemoryMappedFile()
        {
            close();
        }


        // Prevent copying
        MemoryMappedFile(const MemoryMappedFile &) = delete;
        MemoryMappedFile &operator=(const MemoryMappedFile &) = delete;

        // Allow moving
        MemoryMappedFile(MemoryMappedFile &&other) noexcept
            : hFile(other.hFile), hMapping(other.hMapping),
              data(other.data), size(other.size)
        {
            other.hFile = INVALID_HANDLE_VALUE;
            other.hMapping = nullptr;
            other.data = nullptr;
            other.size = 0;
        }

        const char *begin() const { return static_cast<const char *>(data); }
        const char *end() const { return static_cast<const char *>(data) + size; }
        bool valid() const { return data != nullptr; }
        size_t get_size() const { return size; }
        const char *data_ptr() const { return static_cast<const char *>(data); }


        private:

        HANDLE hFile = INVALID_HANDLE_VALUE;
        HANDLE hMapping = nullptr;
        void *data = nullptr;
        size_t size = 0;
    };

    // Structure to hold parsed row information
    struct ParsedRow
    {
        std::string_view full_line;
        std::string_view FEN;
        size_t line_number;
        size_t file_offset;
    };



    class UltraFastCSVParser
    {
        MemoryMappedFile mmap;
        std::vector<size_t> row_offsets;
        bool index_built = false;

    public:

        bool open(const std::string &filename);
        void close();
        void build_index(size_t max_rows);
        ParsedRow get_row(size_t row_index);

        ParsedRow get_row_at_offset(size_t offset, size_t hint_line_number);

        size_t row_count() const;
        bool is_open() const;
    };

    class LichessDbPuzzle
    {

    private:
        UltraFastCSVParser parser;

    public:
        int open_and_build_index(std::string db_filename);
        int pass_FEN(std::function<void(std::string_view, size_t)> processor);

        std::string get_full(size_t index);
    };

}