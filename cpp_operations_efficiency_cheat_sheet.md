
# C/C++ Operations Efficiency Cheat Sheet

| Operation                       | Relative Speed | Description                                                 |
|---------------------------------|----------------|-------------------------------------------------------------|
| Integer addition                | Very Fast      | Simple arithmetic operation, usually a single CPU cycle     |
| Integer multiplication          | Very Fast      | Slightly more complex than addition but still very fast     |
| Floating-point addition         | Very Fast      | Similar to integer addition but involves floating-point unit|
| Floating-point multiplication   | Very Fast      | Slightly slower than integer multiplication                 |
| Bitwise operations (AND, OR, XOR, NOT) | Very Fast | Performed directly on CPU registers                         |
| Memory access (cached)          | Very Fast      | Accessing data in CPU cache is very quick                   |
| Integer division                | Fast           | More complex than multiplication, slower but still efficient|
| Floating-point division         | Fast           | More complex than integer division                          |
| Smart pointer dereference (`std::shared_ptr`, `std::unique_ptr`) | Fast | Slight overhead compared to raw pointer dereference due to additional checks |
| Function call (inline)          | Fast           | Inline functions eliminate the overhead of function calls   |
| Memory access (non-cached)      | Moderate       | Accessing data from RAM is slower than cache                |
| Smart pointer creation (`std::make_shared`, `std::make_unique`) | Moderate | Allocates memory and constructs the object                  |
| Function call (non-inline)      | Moderate       | Overhead due to stack operations                            |
| Virtual function call           | Moderate       | Involves indirection through a vtable                       |
| Branch (predictable)            | Moderate       | Modern CPUs predict branches effectively                    |
| Memory allocation (`new`/`delete`, `malloc`/`free`) | Moderate | Involves managing the heap, can be slow                     |
| Lock acquisition (uncontended)  | Moderate       | Fast if no contention, but still some overhead              |
| File I/O (sequential)           | Moderate       | Dependent on storage device speed, usually buffered         |
| Branch (unpredictable)          | Slow           | Mis-predicted branches cause pipeline stalls                |
| File I/O (random)               | Slow           | More overhead due to seeking                                |
| Smart pointer reference counting (`std::shared_ptr`) | Slow | Involves atomic operations for thread-safe reference counting |
| Lock acquisition (contended)    | Slow           | Can be very slow due to contention                          |
| Thread creation                 | Slow           | Involves significant OS overhead                            |
| Thread context switch           | Slow           | Involves saving/restoring CPU state                         |

### Notes:

1. **Smart Pointers**:
   - **Dereferencing (`std::shared_ptr` and `std::unique_ptr`)**: Slightly slower than raw pointers due to additional checks, but generally very efficient.
   - **Creation (`std::make_shared`, `std::make_unique`)**: Includes memory allocation and object construction.
   - **Reference Counting (`std::shared_ptr`)**: Involves atomic operations to manage the reference count, which can be slower, especially in multithreaded contexts.

2. **Memory Access**:
   - **Cached**: Extremely fast due to being within the CPU cache.
   - **Non-cached**: Slower as it involves accessing main memory (RAM).

3. **Function Calls**:
   - **Inline**: Eliminates the function call overhead by inserting the function code directly at the call site.
   - **Non-inline**: Regular function calls involve some overhead due to stack operations.
   - **Virtual**: Involves a vtable lookup, adding an extra memory access.

4. **Branching**:
   - **Predictable**: Modern CPUs handle predictable branches very efficiently.
   - **Unpredictable**: Can cause pipeline stalls due to branch mispredictions.

5. **File I/O**:
   - **Sequential**: More efficient as data is read/written in order.
   - **Random**: Involves seeking, which adds overhead.

6. **Locking and Threading**:
   - **Uncontended Locks**: Faster as there is no contention.
   - **Contended Locks**: Slower due to the need to manage contention.
   - **Thread Creation and Context Switching**: High overhead operations due to OS involvement.
