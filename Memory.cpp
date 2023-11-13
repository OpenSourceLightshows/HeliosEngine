#include "Memory.h"

#include <stdlib.h>
#include <stdio.h>

// for C++11 need the following:
void *operator new  (size_t size) { return malloc(size); }
void *operator new[](size_t size) { return malloc(size); }
void  operator delete  (void *ptr) { free(ptr); }
void  operator delete[](void *ptr) { free(ptr); }
void *operator new  (size_t size, void *ptr) noexcept { return ptr; }
void *operator new[](size_t size, void *ptr) noexcept { return ptr; }
void  operator delete  (void *ptr, size_t size) noexcept { free(ptr); }
void  operator delete[](void *ptr, size_t size) noexcept { free(ptr); }
//void *operator new  (size_t size, std::align_val_t al) { return vmalloc(size); }
//void *operator new[](size_t size, std::align_val_t al) { return vmalloc(size); }
//void  operator delete  (void *ptr, std::align_val_t al) noexcept { free(ptr); }
//void  operator delete[](void *ptr, std::align_val_t al) noexcept { free(ptr); }
//void  operator delete  (void *ptr, size_t size, std::align_val_t al) noexcept { free(ptr); }
//void  operator delete[](void *ptr, size_t size, std::align_val_t al) noexcept { free(ptr); }

// needed for C++ virtual functions
extern "C" void __cxa_pure_virtual(void) {}
extern "C" void __cxa_deleted_virtual(void) {}

