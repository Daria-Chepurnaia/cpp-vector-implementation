# Vector Implementation in C++

## Overview

This repository contains the implementation of a dynamic array class `Vector` in C++. It mirrors the functionality of `std::vector`, providing an educational exploration of memory management, exception safety, and efficient resource handling. This implementation supports dynamic resizing, element insertion, and other common vector operations, adhering to modern C++ best practices.

## Features

- **Dynamic Memory Management**: Efficient allocation and deallocation using raw memory management techniques.
- **Exception Safety**: Robust handling of exceptions during memory allocation and element construction.
- **Element Insertion and Removal**: Methods for adding and removing elements, both at the end and at arbitrary positions.
- **Move Semantics**: Optimized operations using move constructors and move assignment operators.
- **Template Support**: A generic implementation that can handle elements of any type.

## Classes

### Vector

The `Vector` class is the main container, providing an interface similar to `std::vector`. It supports the following functionalities:

- **Constructor**: Default, size-based, copy, and move constructors.
- **Element Access**: `operator[]` for non-const and const access.
- **Capacity Management**: Methods to check size and capacity, and to reserve memory.
- **Modifiers**: Methods for adding (`PushBack`, `EmplaceBack`), inserting (`Insert`, `Emplace`), and removing (`PopBack`, `Erase`) elements.

### RawMemory

A helper class responsible for managing raw memory allocation and deallocation. It ensures that the `Vector` class can handle memory efficiently and safely.

## Usage

To use the `Vector` class, include the header file and create an instance of the vector with the desired element type:

```cpp
#include "vector.h"

int main() {
    Vector<int> vec;
    vec.PushBack(10);
    vec.PushBack(20);
    vec.PushBack(30);
    
    vec.EmplaceBack(40);
    
    for (size_t i = 0; i < vec.Size(); ++i) {
        std::cout << vec[i] << " ";
    }
    
    return 0;
}
```

## Building

To compile and run this example code, use a C++ compiler that supports C++17 or later. The main function contains an autotest for testing this class. Here is an example of how to compile it using g++:

```sh
g++ -std=c++17 -o vector_example main.cpp
./vector_example
```



## License

This project is licensed under the MIT License. See the [LICENSE](https://github.com/Daria-Chepurnaia/cpp-vector-implementation/blob/main/LICENSE.txt) file for details.

