# nadekOSengoku

- This project is a small, yet fully functional operating system kernel designed to support multithreading with time sharing.

## Basic functionality

- Custom contiguous memory allocator
- Multithreading support
- User/kernel space
- C/C++ API

## Ubuntu 20.04 dependencies

- build-essential
- qemu-system-misc
- gcc-riscv64-linux-gnu
- binutils-riscv64-linux-gnu
- gdb-multiarch
- g++-riscv64-linux-gnu

## Build

Build with:
```bash
make qemu
```
Debugging with gdb:

```bash
make qemu-gdb
```
