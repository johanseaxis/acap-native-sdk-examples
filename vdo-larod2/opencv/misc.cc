#include "misc.hh"

#include <cstdio>
#include <fcntl.h>
#include <functional>
#include <iostream>
#include <memory>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "log.h"

using namespace std;

static constexpr long int MILLION = 1e6;
static constexpr long int BILLION = 1e9;

Timer::Timer() : msg("Timer:") {
    start = std::chrono::steady_clock::now();
}

Timer::Timer(std::string_view msg) : msg(msg) {
    start = std::chrono::steady_clock::now();
}

Timer::~Timer()
{
    printDuration();
}

void Timer::printDuration() const
{
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> duration = end - start;
    auto ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
    if (ns < MILLION) {
        cout << msg << ": " << ns << " ns\n";
    } else if (ns >= MILLION && ns <= BILLION) {
        auto ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(duration)
                .count();
        cout << msg << ": " << ms << " ms\n";
    } else {
        size_t sec = static_cast<size_t>(
            std::chrono::duration_cast<std::chrono::seconds>(duration).count());

        size_t min = sec / 60;
        size_t hours = min / 60;
        min %= 60;
        sec = sec - 60 * 60 * hours - 60 * min;
        cout << msg << ": " << hours << " h " << min << " m " << sec
             << " s\n";
    }
}

void writeDataToFile(const uint8_t* data, size_t sz, const char* file) {
    unique_ptr<FILE, function<void(FILE*)>> fp{fopen(file, "w"),
                                               [](FILE* f) { fclose(f); }};
    if (!fp) {
        logError("Could not open %s", file);
        return;
    }

    size_t bytesWritten = 0;
    while (bytesWritten < sz) {
        size_t bytes =
            fwrite(data + bytesWritten, 1, sz - bytesWritten, fp.get());
        bytesWritten += bytes;
    }

    logInfo("Wrote %zu bytes to %s", bytesWritten, file);
}

FdBuffer::FdBuffer(size_t sz) : size(sz) {
    static size_t id = 0;

    filePath = "tmp_" + to_string(id++);
    static constexpr int MODE = S_IRWXG | S_IRWXU | S_IRWXO;
    fd = open(filePath.c_str(), O_CREAT | O_TRUNC | O_RDWR, MODE);
    if (fd < 0) {
        throw runtime_error("Could not open file for fd buffer");
    }

    if (ftruncate(fd, static_cast<off_t>(size)) != 0) {
        close(fd);
        throw runtime_error("Could not truncate file for fd buffer");
    }

    data = static_cast<uint8_t*>(
        mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (data == MAP_FAILED) {
        close(fd);
        throw runtime_error("Could not mmap file for fd buffer");
    }
}

FdBuffer::~FdBuffer() {
    if (close(fd)) {
        logWarning("Failed to close fd");
    }
    if (msync(data, size, MS_SYNC)) {
        logWarning("Failed to sync mmap");
    }
    if (munmap(data, size)) {
        logWarning("Failed to unmap mmap");
    }
}
