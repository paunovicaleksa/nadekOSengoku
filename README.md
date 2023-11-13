# nadekOSengoku

# nadekOSengoku

## Overview

This project is a small, yet fully functional operating system kernel designed to support multithreading with time sharing.

## Basic Functionality

- Custom contiguous memory allocator
- Custom Slab allocator for kernel memory
- Multithreading support
- User/kernel space
- C/C++ API

## Ubuntu 20.04 Dependencies

- build-essential
- qemu-system-misc
- gcc-riscv64-linux-gnu
- binutils-riscv64-linux-gnu
- gdb-multiarch
- g++-riscv64-linux-gnu

## Build

To build and run the project, use the following command:

```bash
make qemu
```
Debug with gdb:
```bash
make qemu-gdb
```
