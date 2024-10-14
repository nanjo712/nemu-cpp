#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#include <array>
#include <cstddef>
#include <print>
#include <type_traits>
#include <utility>

template <typename T, size_t N>
class RingBuffer
{
   public:
    RingBuffer() : head(0), tail(0) {}

    template <typename Ty>
        requires std::is_same_v<std::remove_reference_t<Ty>, T>
    Ty& push(Ty&& item)
    {
        Ty& ret = buffer[head] = std::forward<Ty>(item);
        head = (head + 1) % N;
        if (head == tail)
        {
            tail = (tail + 1) % N;
        }
        return ret;
    }

    bool empty() const { return head == tail; }

    bool full() const { return (head + 1) % N == tail; }

    void print()
    {
        for (size_t i = tail; i != head; i = (i + 1) % N)
        {
            std::print("{}\n", buffer[i]);
        }
    }

   private:
    std::array<T, N> buffer;
    size_t head;
    size_t tail;
};

#endif  // RING_BUFFER_H_