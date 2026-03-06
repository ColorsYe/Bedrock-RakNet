/*
 * RakSafeString.h
 * Safe string copy utilities to replace strcpy throughout the codebase.
 * Drop-in safe replacement: truncates rather than overflows.
 */

#pragma once
#include <cstring>
#include <cstddef>

namespace RakNet {

/// Safe string copy: always null-terminates, never overflows.
/// Returns dest for chaining.
inline char* SafeStrcpy(char* dest, const char* src, size_t destSize) {
    if (destSize == 0 || dest == nullptr) return dest;
    if (src == nullptr) { dest[0] = '\0'; return dest; }
    strncpy(dest, src, destSize - 1);
    dest[destSize - 1] = '\0';
    return dest;
}

/// Safe strcat: always null-terminates, never overflows.
inline char* SafeStrcat(char* dest, const char* src, size_t destSize) {
    if (destSize == 0 || dest == nullptr || src == nullptr) return dest;
    size_t destLen = strlen(dest);
    if (destLen >= destSize - 1) return dest;
    strncat(dest, src, destSize - destLen - 1);
    return dest;
}

} // namespace RakNet

// Convenience macro that auto-computes buffer size for stack arrays
#define RAKNET_SAFE_STRCPY(dest, src) RakNet::SafeStrcpy((dest), (src), sizeof(dest))
