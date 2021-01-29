#pragma once

#include <chrono>
#include <string>
#include <string_view>

// Prints the time since construction at time of destruction.
class Timer {
  public:
    Timer();
    Timer(std::string_view msg);
    ~Timer();

    void printDuration() const;

  private:
      std::chrono::steady_clock::time_point start;
      std::string_view msg;
};

#ifdef DEBUG
#define TIMER(x) Timer timer(x)
#else
#define TIMER(x)
#endif

/**
 * @brief Write data buffer to file.
 */
void writeDataToFile(const uint8_t* data, size_t sz, const char* file);

/**
 * @brief A buffer in the form of a regular file.
 *
 * The buffer is created by opening a file "tmp_0", "tmp_1" etc., which it then
 * truncates to the desired size and then mmaps.
 */
struct FdBuffer {
    size_t size = 0;
    uint8_t* data = nullptr;
    int fd = -1;
    std::string filePath;

    FdBuffer(size_t sz);
    ~FdBuffer();
};
