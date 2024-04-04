#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <vector>
#include <atomic>

struct RingBuffer {
    std::atomic<int> readHead;
    std::atomic<int> writeHead;
    std::atomic<int> count;
    size_t maxBuffers = 128;
    size_t channels = 1;
    size_t bufferSize = 480;
    std::vector<std::vector<float>> buffers;
    RingBuffer& operator=(RingBuffer&& other) noexcept;
    RingBuffer(const RingBuffer& other);
    RingBuffer();
    RingBuffer(size_t bufferSize);
    void write(float *data, size_t amount);
    void readOneBuffer(float *out);
};

#endif