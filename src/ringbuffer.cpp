#include "ringbuffer.h"

RingBuffer::RingBuffer() {
    this->readHead = 0;
    this->writeHead = 0;
    this->count = 0;
    this->buffers.resize(this->maxBuffers);
    for(auto &buf : this->buffers) {
        buf.resize(this->bufferSize*channels);
    }
    maxBuffers = 128;
    bufferSize = 256;
}

RingBuffer::RingBuffer(size_t bufferSize) : bufferSize(bufferSize) 
{
    this->readHead = 0;
    this->writeHead = 0;
    this->count = 0;
    this->buffers.resize(this->maxBuffers);
    for(auto &buf : this->buffers) {
        buf.resize(this->bufferSize*channels);
    }
    maxBuffers = 128;
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

//TODO: MAKE THIS TAKE A SIZE_T OF HOW MUCH IS BEING WRITTEN, 
//AND IT WILL FILL CEIL(SIZE/BUFFERSIZE) BUFFERS WITH PADDING 0's ON THE END IF NECESSARY
//EDIT: Did this
void RingBuffer::write(float *data, size_t amount) {
    size_t amountLeft = amount;
    size_t offset = 0;

    while(amountLeft > 0) {
        size_t amountPulled = std::min(bufferSize*channels, amountLeft);
        amountLeft -= amountPulled;

        size_t paddingToAdd = bufferSize*channels - amountPulled;

        if(count < maxBuffers) {
            int currentWriteHead = writeHead.load();
            std::copy(data + offset, data + offset + amountPulled, buffers[currentWriteHead].data());

            if(paddingToAdd > 0) {
                std::fill(buffers[currentWriteHead].data() + amountPulled, buffers[currentWriteHead].data() + bufferSize*channels, 0.0f);
            }

            int oldValue, newValue;
            
            do {
                oldValue = count.load();
                newValue = oldValue + 1;
            } while (!count.compare_exchange_weak(oldValue, newValue));


            do {
                oldValue = writeHead.load();
                newValue = (oldValue + 1) % maxBuffers;
            } while (!writeHead.compare_exchange_weak(oldValue, newValue));
            offset += amountPulled;
        }
    }
}

//TODO: MAKE THIS TAKE A SIZE_T OF HOW MUCH TO READ?
//Or maybe it can just always output 1 buffer-full (This seems fine)
//EDIT renamed to readonebuffer
void RingBuffer::readOneBuffer(float *out) {
    if(count > 0) {
        int currentReadHead = readHead.load();
        std::copy(buffers[currentReadHead].data(), buffers[currentReadHead].data()+bufferSize*channels, out);
        
        int oldValue, newValue;
        
        do {
            oldValue = count.load();
            newValue = oldValue - 1;
        } while (!count.compare_exchange_weak(oldValue, newValue));

        do {
            oldValue = readHead.load();
            newValue = (oldValue + 1) % maxBuffers;
        } while (!readHead.compare_exchange_weak(oldValue, newValue));
    }
}