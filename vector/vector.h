#pragma once
#include <cassert>
#include <cstdlib>
#include <new>
#include <utility>
#include <memory>
#include <algorithm>
#include <iterator>

template <typename T>
class RawMemory {
public:
    RawMemory() = default;
    
    RawMemory(const RawMemory&) = delete;
    RawMemory& operator=(const RawMemory& rhs) = delete;
    RawMemory(RawMemory&& other) noexcept {
        Swap(other);
    }
    
    RawMemory& operator=(RawMemory&& rhs) noexcept {       
        if (this != &rhs) {
            Deallocate(buffer_);            
            buffer_ = std::exchange(rhs.buffer_, nullptr);
            capacity_ = std::exchange(rhs.capacity_, 0);
        }
        return *this;
    }

    explicit RawMemory(size_t capacity)
        : buffer_(Allocate(capacity))
        , capacity_(capacity) {
    }

    ~RawMemory() {
        Deallocate(buffer_);
    }

    T* operator+(size_t offset) noexcept {
        // Разрешается получать адрес ячейки памяти, следующей за последним элементом массива
        assert(offset <= capacity_);
        return buffer_ + offset;
    }

    const T* operator+(size_t offset) const noexcept {
        return const_cast<RawMemory&>(*this) + offset;
    }

    const T& operator[](size_t index) const noexcept {
        return const_cast<RawMemory&>(*this)[index];
    }

    T& operator[](size_t index) noexcept {
        assert(index < capacity_);
        return buffer_[index];
    }

    void Swap(RawMemory& other) noexcept {
        std::swap(buffer_, other.buffer_);
        std::swap(capacity_, other.capacity_);
    }

    const T* GetAddress() const noexcept {
        return buffer_;
    }

    T* GetAddress() noexcept {
        return buffer_;
    }

    size_t Capacity() const {
        return capacity_;
    }    

private:
    // Выделяет сырую память под n элементов и возвращает указатель на неё
    static T* Allocate(size_t n) {
        return n != 0 ? static_cast<T*>(operator new(n * sizeof(T))) : nullptr;
    }

    // Освобождает сырую память, выделенную ранее по адресу buf при помощи Allocate
    static void Deallocate(T* buf) noexcept {
        operator delete(buf);
    }

    T* buffer_ = nullptr;
    size_t capacity_ = 0;
};

template <typename T>
class Vector {
public:
    using iterator = T*;
    using const_iterator = const T*;
    
    Vector() = default;

    explicit Vector(size_t size)
        : data_(size)        
        , size_(size)  //
    {
        std::uninitialized_value_construct_n(data_.GetAddress(), size);
    }
    
    Vector(const Vector& other)
        : data_(other.size_)        
        , size_(other.size_)  //            
    {  
        std::uninitialized_copy_n(other.data_.GetAddress(), other.size_, data_.GetAddress());        
    } 
    
    Vector(Vector&& other) noexcept {
        data_ = std::move(other.data_);
        size_ = std::exchange(other.size_, 0);
    }

    Vector& operator=(const Vector& rhs) {
        if (this != &rhs) {
            if (rhs.size_ > data_.Capacity()) {                      
                Vector rhs_copy(rhs);
                Swap(rhs_copy);                
            } else {
                /* Скопировать элементы из rhs, создав при необходимости новые
                   или удалив существующие */
                std::copy_n(rhs.data_.GetAddress(), std::min(size_, rhs.size_), data_.GetAddress());
                if (rhs.size_ < size_) {                    
                    std::destroy_n(data_.GetAddress()+rhs.size_, size_ - rhs.size_);
                } else {                    
                    std::uninitialized_copy_n(rhs.data_.GetAddress()+size_, rhs.size_ - size_, data_.GetAddress()+size_);
                }
                size_ = rhs.size_;
            }
        }
        return *this;
    }
    
    Vector& operator=(Vector&& rhs) noexcept {
        data_ = std::move(rhs.data_);
        size_ = std::exchange(rhs.size_, 0);
        return *this;
    }

    void Swap(Vector& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
    }
    
    void Resize(size_t new_size) {
        if (size_ == new_size) return;
        if (new_size < size_) {
            std::destroy_n(data_.GetAddress()+new_size, size_- new_size);                        
        } else {
            Reserve(new_size);
            std::uninitialized_value_construct_n(data_.GetAddress()+size_, new_size - size_);
        }
        size_ = new_size;
    }
    
    template <typename V>
    void PushBack(V&& value) {
        EmplaceBack(std::forward<V>(value));        
    }   
    
    template <typename... Args>
    T& EmplaceBack(Args&&... args) {
        return *Emplace(cend(), std::forward<Args>(args)...);        
    }   
 
    template <typename... Args>
    iterator Emplace(const_iterator pos, Args&&... args) {
        auto distance = std::distance(cbegin(), pos);
        if (size_ == Capacity()) {            
            EmplaceWithReallocation(pos, std::forward<Args>(args)...);
        } else {
            EmplaceWithoutReallocation(pos, std::forward<Args>(args)...);
        }            
        ++size_;
        return begin()+distance;
    }
    
    iterator Erase(const_iterator pos) {
        auto distance = std::distance(cbegin(), pos);
        std::move(begin()+distance+1, end(), begin()+distance);        
        PopBack();        
        return begin()+distance;
    }    
  
    template <typename V>
    iterator Insert(const_iterator pos, V&& value) {
        return Emplace(pos, std::forward<V>(value));
    }
    
    void PopBack() {
        std::destroy_n(data_.GetAddress() + size_ - 1, 1);
        --size_;
    }
    
    void Reserve(size_t new_capacity) {
        if (new_capacity <= data_.Capacity()) {
            return;
        }
        RawMemory<T> new_data(new_capacity);
        UninitializedMoveOrCopy(data_.GetAddress(), size_, new_data.GetAddress());
        std::destroy_n(data_.GetAddress(), size_);
        data_.Swap(new_data);
    }
    
    size_t Size() const noexcept {
        return size_;
    }

    size_t Capacity() const noexcept {
        return data_.Capacity();
    }

    const T& operator[](size_t index) const noexcept {
        return const_cast<Vector&>(*this)[index];
    }

    T& operator[](size_t index) noexcept {
        assert(index < size_);
        return data_[index];
    }
    
    iterator begin() noexcept {
        return data_.GetAddress();
    }
    iterator end() noexcept {
        return data_.GetAddress() + size_;
    }
    const_iterator begin() const noexcept {
        return data_.GetAddress();
    }
    const_iterator end() const noexcept {
        return data_.GetAddress() + size_;
    }
    const_iterator cbegin() const noexcept {
        return data_.GetAddress();
    }
    const_iterator cend() const noexcept {
        return data_.GetAddress() + size_;
    }
    
    ~Vector() {        
        std::destroy_n(data_.GetAddress(), size_);
    }

private:
    RawMemory<T> data_;
    size_t size_ = 0;
    
    void UninitializedMoveOrCopy(iterator from, int number_of_elements, iterator to) {
        if constexpr(std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {           
            std::uninitialized_move_n(from, number_of_elements, to);
        } else {
            std::uninitialized_copy_n(from, number_of_elements, to);
        }
    }
    
    template <typename... Args>
    void EmplaceWithReallocation(const_iterator pos, Args&&... args) {
        auto distance = std::distance(cbegin(), pos);
        RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);            
        new(new_data + distance) T(std::forward<Args>(args)...);

        try {
            UninitializedMoveOrCopy(begin(), distance, new_data.GetAddress());
        }
        catch(...) {
            std::destroy_n(new_data.GetAddress() + distance, 1);
            throw;
        }            
        try {                
            UninitializedMoveOrCopy(begin()+distance, size_ - distance, new_data.GetAddress() + distance + 1);
        }
        catch(...) {
            std::destroy_n(new_data.GetAddress(), distance + 1);
            throw;
        }
        data_.Swap(new_data);  
        std::destroy_n(new_data.GetAddress(), size_);
    }
        
    template <typename... Args>
    void EmplaceWithoutReallocation(const_iterator pos, Args&&... args) {
        auto distance = std::distance(cbegin(), pos);
        if (pos == cend()) {
            new(data_.GetAddress() + size_) T(std::forward<Args>(args)...);
        } else {
            T tmp(std::forward<Args>(args)...);                       
            std::uninitialized_move_n(data_.GetAddress() + size_ - 1, 1, data_.GetAddress() + size_);                 
            std::move_backward(data_ + distance, end() - 1, end());            
            data_[distance] = std::move(tmp);
        }
    } 

};