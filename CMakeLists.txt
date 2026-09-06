cmake_minimum_required(VERSION 3.20)

project(MarketMicrostructureSimulator
    VERSION 1.0
    LANGUAGES CXX
)

# Use the C++20 standard.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable useful compiler warnings.
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# Tell the compiler where our header files are located.
include_directories(include)

# Build the OrderBook library and MatchingEngine library.
add_library(order_book
    src/order_book/OrderBook.cpp
    src/matching_engine/MatchingEngine.cpp
)

# Build the OrderBook test executable.
add_executable(test_order_book
    tests/test_order_book.cpp
)

# Link the OrderBook implementation with the test.
target_link_libraries(test_order_book
    PRIVATE order_book
)