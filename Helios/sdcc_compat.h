#ifndef SDCC_COMPAT_H
#define SDCC_COMPAT_H

// ============================================================================
// SDCC Compatibility Header
// ============================================================================
// This header provides compatibility macros and definitions to allow
// the Helios C++ codebase to compile with SDCC's C compiler

#ifdef __SDCC

// SDCC doesn't support C++ but we can fake some basic features

// Replace C++ keywords with C equivalents
#define class struct
#define private
#define protected
#define public

// Remove C++ specific syntax
#define virtual
#define override
#define final
#define explicit
#define constexpr const
#define noexcept
#define nullptr ((void*)0)

// Enum class becomes regular enum
#define enum_class enum

// Remove reference syntax (SDCC doesn't support C++ references)
// We'll need to manually fix these in the code
// This is just to help with simple cases

// Remove template syntax (we don't use templates much anyway)
#define template

// Remove namespace syntax
#define namespace
#define using

// Remove C++11/14/17 features
#define static_assert(...)
#define decltype(x) typeof(x)

#endif // __SDCC

#endif // SDCC_COMPAT_H

