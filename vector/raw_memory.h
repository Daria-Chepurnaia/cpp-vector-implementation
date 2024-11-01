#pragma once 

template <typename T>
class RawMemory {
public:
    RawMemory() = default;
    explicit RawMemory(size_t capacity);
    RawMemory(RawMemory&& other) noexcept;
    RawMemory& operator=(RawMemory&& rhs) noexcept;

    RawMemory(const RawMemory&) = delete;
    RawMemory& operator=(const RawMemory& rhs) = delete;

    T* operator+(size_t offset) noexcept;
    const T* operator+(size_t offset) const noexcept;
    const T& operator[](size_t index) const noexcept;
    T& operator[](size_t index) noexcept;

    void Swap(RawMemory& other) noexcept;
    const T* GetAddress() const noexcept;
    T* GetAddress() noexcept;
    size_t Capacity() const;
    ~RawMemory();

private:
    static T* Allocate(size_t n);
    static void Deallocate(T* buf) noexcept;

    T* buffer_ = nullptr;
    size_t capacity_ = 0;
};


// Implementation of RawMemory class template methods


template <typename T>
RawMemory<T>::RawMemory(size_t capacity)
    : buffer_(Allocate(capacity)), capacity_(capacity) {}

template <typename T>
RawMemory<T>::RawMemory(RawMemory&& other) noexcept {
    Swap(other);
}

template <typename T>
RawMemory<T>& RawMemory<T>::operator=(RawMemory&& rhs) noexcept {
    if (this != &rhs) {
        Deallocate(buffer_);
        buffer_ = std::exchange(rhs.buffer_, nullptr);
        capacity_ = std::exchange(rhs.capacity_, 0);
    }
    return *this;
}

template <typename T>
T* RawMemory<T>::operator+(size_t offset) noexcept {
    assert(offset <= capacity_);
    return buffer_ + offset;
}

template <typename T>
const T* RawMemory<T>::operator+(size_t offset) const noexcept {
    return const_cast<RawMemory&>(*this) + offset;
}

template <typename T>
const T& RawMemory<T>::operator[](size_t index) const noexcept {
    return const_cast<RawMemory&>(*this)[index];
}

template <typename T>
T& RawMemory<T>::operator[](size_t index) noexcept {
    assert(index < capacity_);
    return buffer_[index];
}

template <typename T>
void RawMemory<T>::Swap(RawMemory& other) noexcept {
    std::swap(buffer_, other.buffer_);
    std::swap(capacity_, other.capacity_);
}

template <typename T>
const T* RawMemory<T>::GetAddress() const noexcept {
    return buffer_;
}

template <typename T>
T* RawMemory<T>::GetAddress() noexcept {
    return buffer_;
}

template <typename T>
size_t RawMemory<T>::Capacity() const {
    return capacity_;
}

template <typename T>
RawMemory<T>::~RawMemory() {
    Deallocate(buffer_);
}

template <typename T>
T* RawMemory<T>::Allocate(size_t n) {
    return n != 0 ? static_cast<T*>(operator new(n * sizeof(T))) : nullptr;
}

template <typename T>
void RawMemory<T>::Deallocate(T* buf) noexcept {
    operator delete(buf);
}
