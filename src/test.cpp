#include <iostream>
#include <sstream>
#include <string_view>



#include "test.h"

#ifdef __AVX2__
#include <immintrin.h>
#endif

namespace Test
{

    std::string_view get_first_word(std::string_view str)
    {
        size_t space_pos = str.find(' ');
        if (space_pos == std::string_view::npos)
        {
            return str; // No space found, return entire string
        }
        return str.substr(0, space_pos);
    }

    bool MemoryMappedFile::open(const std::string &path)
    {
        // Close any existing mapping
        close();

        // Convert string path to wide string for Windows API
        int wideLen = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, nullptr, 0);
        std::wstring widePath(wideLen, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, widePath.data(), wideLen);

        // Open file with minimal caching for sequential access
        hFile = CreateFileW(
            widePath.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_SEQUENTIAL_SCAN, // Optimize for sequential access
            nullptr);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        // Get file size
        LARGE_INTEGER fileSize;
        GetFileSizeEx(hFile, &fileSize);
        size = static_cast<size_t>(fileSize.QuadPart);

        // Create file mapping
        hMapping = CreateFileMappingW(
            hFile,
            nullptr,
            PAGE_READONLY,
            0,
            0,
            nullptr);

        if (!hMapping)
        {
            CloseHandle(hFile);
            hFile = INVALID_HANDLE_VALUE;
            return false;
        }

        // Map view of file
        data = MapViewOfFile(
            hMapping,
            FILE_MAP_READ,
            0,
            0,
            0 // Map entire file
        );

        if (!data)
        {
            CloseHandle(hMapping);
            CloseHandle(hFile);
            hMapping = nullptr;
            hFile = INVALID_HANDLE_VALUE;
            return false;
        }

        return true;
    }

    void MemoryMappedFile::close()
    {
        if (data)
            UnmapViewOfFile(data);
        if (hMapping)
            CloseHandle(hMapping);
        if (hFile != INVALID_HANDLE_VALUE)
            CloseHandle(hFile);
        data = nullptr;
        hMapping = nullptr;
        hFile = INVALID_HANDLE_VALUE;
        size = 0;
    }

    // AVX2-accelerated newline detection (assumes AVX2 is available)
    __forceinline size_t find_next_line_avx2(const char *start, const char *end)
    {
        const __m256i newline = _mm256_set1_epi8('\n');
        const __m256i carriage = _mm256_set1_epi8('\r');

        // Align pointer to 32-byte boundary for best performance
        const char *ptr = start;

        // Handle unaligned start
        while (reinterpret_cast<uintptr_t>(ptr) & 31 && ptr < end)
        {
            if (*ptr == '\n' || *ptr == '\r')
            {
                return ptr - start;
            }
            ptr++;
        }

        // Process aligned chunks
        for (; ptr + 32 <= end; ptr += 32)
        {
            __m256i chunk = _mm256_load_si256(reinterpret_cast<const __m256i *>(ptr));
            __m256i cmp_nl = _mm256_cmpeq_epi8(chunk, newline);
            __m256i cmp_cr = _mm256_cmpeq_epi8(chunk, carriage);
            __m256i cmp = _mm256_or_si256(cmp_nl, cmp_cr);

            int mask = _mm256_movemask_epi8(cmp);
            if (mask != 0)
            {
                // Count trailing zeros to find position
                unsigned long pos;
                _BitScanForward(&pos, mask);
                return (ptr - start) + pos;
            }
        }

        // Handle remaining bytes
        for (; ptr < end; ptr++)
        {
            if (*ptr == '\n' || *ptr == '\r')
            {
                return ptr - start;
            }
        }

        return end - start;
    }

    // Optimized comma search using SIMD
    __forceinline const char *find_comma_avx2(const char *start, size_t max_len)
    {
        const __m256i comma = _mm256_set1_epi8(',');
        const char *ptr = start;
        const char *end = start + max_len;

        // Align to 32-byte boundary
        while (reinterpret_cast<uintptr_t>(ptr) & 31 && ptr < end)
        {
            if (*ptr == ',')
                return ptr;
            ptr++;
        }

        // Process aligned chunks
        for (; ptr + 32 <= end; ptr += 32)
        {
            __m256i chunk = _mm256_load_si256(reinterpret_cast<const __m256i *>(ptr));
            __m256i cmp = _mm256_cmpeq_epi8(chunk, comma);
            int mask = _mm256_movemask_epi8(cmp);

            if (mask != 0)
            {
                unsigned long pos;
                _BitScanForward(&pos, mask);
                return ptr + pos;
            }
        }

        // Handle remaining bytes
        for (; ptr < end; ptr++)
        {
            if (*ptr == ',')
                return ptr;
        }

        return nullptr;
    }

    // Windows-specific high-performance timer
    class HighResolutionTimer
    {
        LARGE_INTEGER frequency;
        LARGE_INTEGER start;

    public:
        HighResolutionTimer()
        {
            QueryPerformanceFrequency(&frequency);
            QueryPerformanceCounter(&start);
        }

        double elapsed_seconds() const
        {
            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);
            return static_cast<double>(now.QuadPart - start.QuadPart) / frequency.QuadPart;
        }

        double elapsed_milliseconds() const
        {
            return elapsed_seconds() * 1000.0;
        }
    };

    bool UltraFastCSVParser::open(const std::string &filename)
    {
        return mmap.open(filename);
    }

    void UltraFastCSVParser::close()
    {
        mmap.close();
        row_offsets.clear();
        index_built = false;
    }


    // Parse a specific row at given offset
    ParsedRow parse_row_at_offset(const MemoryMappedFile &mmap, size_t offset, size_t line_number = 0)
    {

        ParsedRow result;
        result.file_offset = offset;
        result.line_number = line_number;

        if (!mmap.valid() || offset >= mmap.get_size())
        {
            return result; // Empty result
        }

        const char *file_start = mmap.begin();
        const char *file_end = mmap.end();
        const char *row_start = file_start + offset;

        // Ensure we're at the start of a line (skip any remaining newline chars)
        while (row_start < file_end && (*row_start == '\r' || *row_start == '\n'))
        {
            row_start++;
            offset++;
        }

        if (row_start >= file_end)
        {
            return result;
        }

        // Find the end of this row (next newline)
        size_t line_len = find_next_line_avx2(row_start, file_end);
        const char *row_end = row_start + line_len;

        // Set full line view
        result.full_line = std::string_view(row_start, row_end - row_start);

        // Parse columns
        const char *col_start = row_start;
        const char *line_end = row_end;

        const char *comma = find_comma_avx2(col_start, line_end - col_start);

        col_start = comma + 1; // Skip comma
        comma = find_comma_avx2(col_start, line_end - col_start);
        // Add column from col_start to comma
        result.FEN = std::string_view(col_start, comma - col_start);

        col_start = comma + 1; // Skip comma
        comma = find_comma_avx2(col_start, line_end - col_start);

        result.first_UCI = get_first_word(std::string_view(col_start, comma - col_start));
        return result;
    }

    std::vector<size_t> build_row_offset_index(const MemoryMappedFile &mmap, size_t max_rows = 0)
    {
        std::vector<size_t> offsets;

        if (!mmap.valid())
        {
            return offsets;
        }

        const char *ptr = mmap.begin();
        const char *end = mmap.end();

        if (ptr < end)
        {
            offsets.push_back(0);
        }

        while (ptr < end)
        {
            size_t line_len = find_next_line_avx2(ptr, end);
            ptr += line_len;

            bool found_newline = false;
            while (ptr < end && (*ptr == '\r' || *ptr == '\n'))
            {
                ptr++;
                found_newline = true;
            }

            if (found_newline && ptr < end)
            {
                offsets.push_back(ptr - mmap.begin());

                if (max_rows > 0 && offsets.size() >= max_rows)
                {
                    break;
                }
            }
        }

        return offsets;
    }



    void UltraFastCSVParser::build_index(size_t max_rows = 0)
    {
        if (!mmap.valid())
            return;

        std::cout << "Building row offset index..." << std::endl;
        auto start = __rdtsc();

        row_offsets = build_row_offset_index(mmap, max_rows);
        index_built = true;

        auto end = __rdtsc();
        double cycles_per_row = 0;

        std::cout << "Index built: " << row_offsets.size() << " rows" << std::endl;
    }

    ParsedRow UltraFastCSVParser::get_row(size_t row_index)
    {
        if (!index_built || row_index >= row_offsets.size())
        {
            return ParsedRow();
        }

        return parse_row_at_offset(mmap, row_offsets[row_index], row_index);
    }

    ParsedRow UltraFastCSVParser::get_row_at_offset(size_t offset, size_t hint_line_number = 0)
    {
        return parse_row_at_offset(mmap, offset, hint_line_number);
    }

    size_t UltraFastCSVParser::row_count() const
    {
        return row_offsets.size();
    }

    bool UltraFastCSVParser::is_open() const
    {
        return mmap.valid();
    }

    int LichessDbPuzzle::open_and_build_index(std::string db_filename)
    {
        std::cout << "Ultra-Fast CSV Parser for Windows (AVX2)" << std::endl;
        std::cout << "========================================" << std::endl;

        if (!parser.open(db_filename))
        {
            std::cerr << "Failed to open file" << std::endl;
            return 1;
        }

        parser.build_index();
        return 0;
    }

    int LichessDbPuzzle::pass_FEN_and_first_UCI(std::function<void(std::string_view, std::string_view, size_t)> processor)
    {

        for (size_t row_id = 0; row_id < parser.row_count(); row_id++)
        {

            ParsedRow row = parser.get_row(row_id);

            processor(row.FEN, row.first_UCI, row_id);
        }

        return 0;
    }

    LichessPuzzle LichessDbPuzzle::get_full(size_t index)
    {

        ParsedRow row = parser.get_row(index);

        std::string_view input = row.full_line;

        std::string s{input}; // Need copy for stringstream
        std::istringstream iss(s);
        std::string tags;

        // Extract next three tokens
        std::string id, FEN, moves;
        std::getline(iss, id, ',');
        std::getline(iss, FEN, ',');
        std::getline(iss, moves, ',');

        std::string rating1, rating2, rating3, rating4;
        std::getline(iss, rating1, ',');
        std::getline(iss, rating2, ',');
        std::getline(iss, rating3, ',');
        std::getline(iss, rating4, ',');
        std::getline(iss, tags, ',');

        std::string link = std::string{"https://lichess.org/training/"} + id;

       
        //link = link.substr(0, 28);


        return LichessPuzzle
        {
            .FEN = FEN,
            .moves = moves,
            .id = id,
            .link = link,
            .full = std::string{input}
        };
    }
}
