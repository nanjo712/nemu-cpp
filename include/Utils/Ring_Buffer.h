#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>

template <typename T, size_t N>
class RingBuffer
{
   public:
    RingBuffer() : head(0), tail(0) {}

    template <typename Ty>
        requires std::is_same_v<T, Ty>
    void push(Ty&& item)
    {
        buffer[head] = std::forward<Ty>(item);
        head = (head + 1) % N;
        if (head == tail)
        {
            tail = (tail + 1) % N;
        }
    }

    T pop()
    {
        if (head == tail)
        {
            return T();
        }
        T item = std::move(buffer[tail]);
        tail = (tail + 1) % N;
        return item;
    }

    bool empty() const { return head == tail; }

    bool full() const { return (head + 1) % N == tail; }

   private:
    std::array<T, N> buffer;
    size_t head;
    size_t tail;
};

#endif  // RING_BUFFER_H_