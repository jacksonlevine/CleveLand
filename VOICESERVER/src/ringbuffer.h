#include <vector>
#include <atomic>

struct RingBuffer {
    std::atomic<int> readHead;
    std::atomic<int> writeHead;
    std::atomic<int> count;
    static const size_t maxBuffers = 128;
    static const size_t bufferSize = 480;
    std::vector<std::vector<float>> buffers;
    RingBuffer& operator=(RingBuffer&& other) noexcept;
    RingBuffer(const RingBuffer& other);
    RingBuffer();
};

void writeToRingBuffer(RingBuffer &rbuf, float *data);
void readFromRingBuffer(RingBuffer &rbuf, float *out);

#ifdef RINGBUFFER_IMPLEMENTATION

RingBuffer::RingBuffer() {
    this->readHead = 0;
    this->writeHead = 0;
    this->count = 0;
    this->buffers.resize(this->maxBuffers);
    for(auto &buf : this->buffers) {
        buf.resize(this->bufferSize);
    }
}

RingBuffer& RingBuffer::operator=(RingBuffer&& other) noexcept {
    if (this != &other) {
        readHead.store(other.readHead.load());
        writeHead.store(other.writeHead.load());
        count.store(other.count.load());
        buffers = std::move(other.buffers);
    }
    return *this;
}

RingBuffer::RingBuffer(const RingBuffer& other)
        : readHead(other.readHead.load()),
          writeHead(other.writeHead.load()),
          count(other.count.load()),
          buffers(other.buffers) {
}

void writeToRingBuffer(RingBuffer &rbuf, float *data) {
    if(rbuf.count < rbuf.maxBuffers) {
        int currentWriteHead = rbuf.writeHead.load();
        std::copy(data, data + rbuf.bufferSize, rbuf.buffers[currentWriteHead].data());

        int oldValue, newValue;
        
        do {
            oldValue = rbuf.count.load();
            newValue = oldValue + 1;
        } while (!rbuf.count.compare_exchange_weak(oldValue, newValue));


        do {
            oldValue = rbuf.writeHead.load();
            newValue = (oldValue + 1) % rbuf.maxBuffers;
        } while (!rbuf.writeHead.compare_exchange_weak(oldValue, newValue));
    }
}

void readFromRingBuffer(RingBuffer &rbuf, float *out) {
    if(rbuf.count > 0) {
        int currentReadHead = rbuf.readHead.load();
        std::copy(rbuf.buffers[currentReadHead].data(), rbuf.buffers[currentReadHead].data()+rbuf.bufferSize, out);
        
        int oldValue, newValue;
        
        do {
            oldValue = rbuf.count.load();
            newValue = oldValue - 1;
        } while (!rbuf.count.compare_exchange_weak(oldValue, newValue));

        do {
            oldValue = rbuf.readHead.load();
            newValue = (oldValue + 1) % rbuf.maxBuffers;
        } while (!rbuf.readHead.compare_exchange_weak(oldValue, newValue));
    }
}

#endif