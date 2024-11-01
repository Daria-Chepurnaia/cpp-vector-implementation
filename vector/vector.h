#pragma once

#include <algorithm>
#include <cassert>

#include "raw_memory.h"

template <typename T>
class Vector {
public:
    using iterator = T*;
    using const_iterator = const T*;

    Vector() = default;
    explicit Vector(size_t size);
    Vector(const Vector& other);
    Vector(Vector&& other) noexcept;

    Vector& operator=(const Vector& rhs);
    Vector& operator=(Vector&& rhs) noexcept;

    // Swaps contents with another vector
    void Swap(Vector& other) noexcept;

    // Resizes the vector to contain `new_size` elements
    void Resize(size_t new_size);

    // Ensures that the vector has at least `new_capacity`
    // storage capacity.
    void Reserve(size_t new_capacity);

    iterator Erase(const_iterator pos);
    void PopBack();
    
    template <typename V>
    void PushBack(V&& value);
    
    template <typename... Args>
    T& EmplaceBack(Args&&... args);
 
    template <typename... Args>
    iterator Emplace(const_iterator pos, Args&&... args);
    
    template <typename V>
    iterator Insert(const_iterator pos, V&& value);

    const T& operator[](size_t index) const noexcept;
    T& operator[](size_t index) noexcept;

    size_t Size() const noexcept { return size_; }
    size_t Capacity() const noexcept { return data_.Capacity(); }
    
    iterator begin() noexcept;
    iterator end() noexcept;
    const_iterator begin() const noexcept;
    const_iterator end() const noexcept;
    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;
    
    ~Vector();

private:
    RawMemory<T> data_;
    size_t size_ = 0;
    
    // Internal helper for uninitialized memory copy or move
    void UninitializedMoveOrCopy(iterator from, int number_of_elements, iterator to);
    
    // Helper for `Emplace` method with reallocation
    template <typename... Args>
    void EmplaceWithReallocation(const_iterator pos, Args&&... args);
    
    // Helper for `Emplace` method without reallocation
    template <typename... Args>
    void EmplaceWithoutReallocation(const_iterator pos, Args&&... args);
};

// Implementation of the Vector class.
// This class template provides a dynamic array-like container with a similar 
// interface to `std::vector`. It supports a range of operations, such as:
// - Element insertion, removal, and access, with automatic resizing as needed
// - Move and copy semantics for efficient resource management
// - `PushBack` and `EmplaceBack` for adding elements at the end
// - `Insert` and `Emplace` for inserting elements at arbitrary positions
// - Iterators for range-based and STL-style iteration

// The class handles its own memory through a `RawMemory` internal storage,
// and it automatically grows when elements exceed current capacity.

// Implementation details follow:


template <typename T>
Vector<T>::Vector(Vector<T>&& other) noexcept {
    data_ = std::move(other.data_);
    size_ = std::exchange(other.size_, 0);
}

template <typename T>
Vector<T>::Vector(size_t size)
    : data_(size)        
    , size_(size)  
{
    std::uninitialized_value_construct_n(data_.GetAddress(), size);
}

template <typename T>
Vector<T>::Vector(const Vector& other)
    : data_(other.size_)        
    , size_(other.size_)             
{  
    std::uninitialized_copy_n(other.data_.GetAddress(), other.size_, data_.GetAddress());        
} 

template <typename T>
Vector<T>& Vector<T>::operator=(const Vector<T>& rhs) {
    if (this != &rhs) {
        if (rhs.size_ > data_.Capacity()) {                      
            Vector rhs_copy(rhs);
            Swap(rhs_copy);                
        } else {           
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

template <typename T>
Vector<T>& Vector<T>::operator=(Vector<T>&& rhs) noexcept {
    data_ = std::move(rhs.data_);
    size_ = std::exchange(rhs.size_, 0);
    return *this;
}

template <typename T>
void Vector<T>::Swap(Vector<T>& other) noexcept {
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
}

template <typename T>
void Vector<T>::Resize(size_t new_size) {
    if (size_ == new_size) return;
    if (new_size < size_) {
        std::destroy_n(data_.GetAddress()+new_size, size_- new_size);                        
    } else {
        Reserve(new_size);
        std::uninitialized_value_construct_n(data_.GetAddress()+size_, new_size - size_);
    }
    size_ = new_size;
}

template <typename T>
template <typename V>
void Vector<T>::PushBack(V&& value) {
    EmplaceBack(std::forward<V>(value));        
}   

template <typename T>
template <typename... Args>
T& Vector<T>::EmplaceBack(Args&&... args) {
    return *Emplace(cend(), std::forward<Args>(args)...);        
}   

template <typename T>
template <typename... Args>
typename Vector<T>::iterator Vector<T>::Emplace(const_iterator pos, Args&&... args) {
    auto distance = std::distance(cbegin(), pos);
    if (size_ == Capacity()) {            
        EmplaceWithReallocation(pos, std::forward<Args>(args)...);
    } else {
        EmplaceWithoutReallocation(pos, std::forward<Args>(args)...);
    }            
    ++size_;
    return begin()+distance;
}

template <typename T>
typename Vector<T>::iterator Vector<T>::Erase(const_iterator pos) {
    auto distance = std::distance(cbegin(), pos);
    std::move(begin()+distance+1, end(), begin()+distance);        
    PopBack();        
    return begin()+distance;
}    

template <typename T>
template <typename V>
typename Vector<T>::iterator Vector<T>::Insert(const_iterator pos, V&& value) {
    return Emplace(pos, std::forward<V>(value));
}

template <typename T>
void Vector<T>::PopBack() {
    std::destroy_n(data_.GetAddress() + size_ - 1, 1);
    --size_;
}

template <typename T>
void Vector<T>::Reserve(size_t new_capacity) {
    if (new_capacity <= data_.Capacity()) {
        return;
    }
    RawMemory<T> new_data(new_capacity);
    UninitializedMoveOrCopy(data_.GetAddress(), size_, new_data.GetAddress());
    std::destroy_n(data_.GetAddress(), size_);
    data_.Swap(new_data);
}

template <typename T>
const T& Vector<T>::operator[](size_t index) const noexcept {
    return const_cast<Vector&>(*this)[index];
}

template <typename T>
T& Vector<T>::operator[](size_t index) noexcept {
    assert(index < size_);
    return data_[index];
}

template <typename T>
typename Vector<T>::iterator Vector<T>::begin() noexcept {
    return data_.GetAddress();
}

template <typename T>
typename Vector<T>::iterator Vector<T>::end() noexcept {
    return data_.GetAddress() + size_;
}

template <typename T>
typename Vector<T>::const_iterator Vector<T>::begin() const noexcept {
    return data_.GetAddress();
}

template <typename T>
typename Vector<T>::const_iterator Vector<T>::end() const noexcept {
    return data_.GetAddress() + size_;
}

template <typename T>
typename Vector<T>::const_iterator Vector<T>::cbegin() const noexcept {
    return data_.GetAddress();
}

template <typename T>
typename Vector<T>::const_iterator Vector<T>::cend() const noexcept {
    return data_.GetAddress() + size_;
}

template <typename T>
Vector<T>::~Vector() {        
    std::destroy_n(data_.GetAddress(), size_);
}

template <typename T>
void Vector<T>::UninitializedMoveOrCopy(iterator from, int number_of_elements, iterator to) {
    if constexpr(std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {           
        std::uninitialized_move_n(from, number_of_elements, to);
    } else {
        std::uninitialized_copy_n(from, number_of_elements, to);
    }
}

template <typename T>
template <typename... Args>
void Vector<T>::EmplaceWithReallocation(const_iterator pos, Args&&... args) {
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
    
template <typename T>
template <typename... Args>
void Vector<T>::EmplaceWithoutReallocation(const_iterator pos, Args&&... args) {
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
