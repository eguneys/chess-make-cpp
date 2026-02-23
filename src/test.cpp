#include <windows.h>
#include <memoryapi.h>
#include <string>
#include <string_view>
#include <functional>
#include <iostream>
#include <intrin.h>


#include "test.h"

#ifdef __AVX2__
#include <immintrin.h>
#endif


namespace Test {

    class MemoryMappedFile
    {
        HANDLE hFile = INVALID_HANDLE_VALUE;
        HANDLE hMapping = nullptr;
        void *data = nullptr;
        size_t size = 0;

    public:
        MemoryMappedFile(const std::string &path)
        {
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
                return;
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
                return;
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
            }
        }

        ~MemoryMappedFile()
        {
            if (data)
                UnmapViewOfFile(data);
            if (hMapping)
                CloseHandle(hMapping);
            if (hFile != INVALID_HANDLE_VALUE)
                CloseHandle(hFile);
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
    };

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

    // Main parsing function with Windows optimizations
    void ultra_fast_csv_parse_windows(const std::string &filename,
                                      std::function<void(std::string_view)> processor,
                                      bool show_stats = false)
    {
        HighResolutionTimer timer;

        MemoryMappedFile file(filename);
        if (!file.valid())
        {
            DWORD error = GetLastError();
            std::cerr << "Failed to memory-map file. Error: " << error << std::endl;
            return;
        }

        const char *ptr = file.begin();
        const char *end = file.end();
        const char *line_start = ptr;

        size_t row_count = 0;
        size_t total_bytes = file.get_size();

        // Process the file
        while (ptr < end)
        {
            // Find end of current line using AVX2
            size_t line_len = find_next_line_avx2(ptr, end);
            const char *line_end = ptr + line_len;

            if (line_end > end)
                break;

            
            // Extract requested columns
            const char *col_start = ptr;
            size_t current_col = 0;
            size_t next_col_idx = 0;

            size_t target = 1;

            // Skip to target column
            while (current_col < target && col_start < line_end)
            {
                const char *comma = find_comma_avx2(col_start, line_end - col_start);
                if (!comma)
                {
                    col_start = line_end;
                    break;
                }
                col_start = comma + 1;
                current_col++;
            }

            if (col_start >= line_end)
                break;

            // Extract column value
            const char *next_comma = find_comma_avx2(col_start, line_end - col_start);
            if (next_comma)
            {
                processor(std::string_view(col_start, next_comma - col_start));
                col_start = next_comma + 1;
            }
            else
            {
                processor(std::string_view(col_start, line_end - col_start));
                col_start = line_end;
            }

            // Move to next line, handling Windows line endings (\r\n)
            ptr = line_end;
            if (ptr < end && *ptr == '\r')
                ptr++;
            if (ptr < end && *ptr == '\n')
                ptr++;

            row_count++;

            // Progress indicator for large files
            if (show_stats && (row_count % 1000000 == 0))
            {
                double percent = (static_cast<double>(ptr - file.begin()) / total_bytes) * 100.0;
                printf("\rProcessed %zu rows (%.1f%%)", row_count, percent);
            }
        }

        if (show_stats)
        {
            double elapsed = timer.elapsed_seconds();
            double mb_per_sec = (total_bytes / (1024.0 * 1024.0)) / elapsed;
            printf("\n\nPerformance Stats:\n");
            printf("  Rows: %zu\n", row_count);
            printf("  File size: %.2f MB\n", total_bytes / (1024.0 * 1024.0));
            printf("  Time: %.3f seconds\n", elapsed);
            printf("  Speed: %.2f MB/s\n", mb_per_sec);
            printf("  Throughput: %.0f rows/second\n", row_count / elapsed);
        }
    }

    // Example usage with parallel processing for multiple columns
    template <typename Func>
    void ultra_fast_csv_parse_multiple_columns(const std::string &filename,
                                               const std::vector<size_t> &column_indices,
                                               Func &&processor)
    {
        MemoryMappedFile file(filename);
        if (!file.valid())
        {
            std::cerr << "Failed to memory-map file" << std::endl;
            return;
        }

        const char *ptr = file.begin();
        const char *end = file.end();

        while (ptr < end)
        {
            // Find line end
            size_t line_len = find_next_line_avx2(ptr, end);
            const char *line_end = ptr + line_len;

            if (line_end > end)
                break;

            // Extract requested columns
            std::vector<std::string_view> values(column_indices.size());
            const char *col_start = ptr;
            size_t current_col = 0;
            size_t next_col_idx = 0;

            while (next_col_idx < column_indices.size())
            {
                size_t target = column_indices[next_col_idx];

                // Skip to target column
                while (current_col < target && col_start < line_end)
                {
                    const char *comma = find_comma_avx2(col_start, line_end - col_start);
                    if (!comma)
                    {
                        col_start = line_end;
                        break;
                    }
                    col_start = comma + 1;
                    current_col++;
                }

                if (col_start >= line_end)
                    break;

                // Extract column value
                const char *next_comma = find_comma_avx2(col_start, line_end - col_start);
                if (next_comma)
                {
                    values[next_col_idx] = std::string_view(col_start, next_comma - col_start);
                    col_start = next_comma + 1;
                }
                else
                {
                    values[next_col_idx] = std::string_view(col_start, line_end - col_start);
                    col_start = line_end;
                }

                current_col++;
                next_col_idx++;
            }

            // Process the extracted values
            processor(values, ptr - file.begin());

            // Move to next line
            ptr = line_end;
            if (ptr < end && *ptr == '\r')
                ptr++;
            if (ptr < end && *ptr == '\n')
                ptr++;
        }
    }

    // Example usage
    int test(std::function<void(std::string_view)> processor)
    {
        std::cout << "Ultra-Fast CSV Parser for Windows (AVX2)" << std::endl;
        std::cout << "========================================" << std::endl;

        // Example 1: Process first column with statistics
        size_t total_chars = 0;
        ultra_fast_csv_parse_windows("../data/lichess_db_puzzle.csv", [&total_chars, &processor](std::string_view value)
                                     {
                                         total_chars += value.size();
                                         // Your actual processing here

                                         processor(value);
                                     },
                                     true // Show statistics
        );

        std::cout << "\nTotal characters processed: " << total_chars << std::endl;

        return 0;

        /*
        // Example 2: Process multiple columns
        std::cout << "\nProcessing multiple columns..." << std::endl;
        ultra_fast_csv_parse_multiple_columns("../data/lichess_db_puzzle.csv",
                                              {0, 2, 4}, // Columns to extract
                                              [](const std::vector<std::string_view> &values, size_t offset)
                                              {
                                                  // Process row with multiple columns
                                                  if (values.size() == 3)
                                                  {
                                                      // Do something with values[0], values[1], values[2]
                                                  }
                                              });

        return 0;
        */
    }
}