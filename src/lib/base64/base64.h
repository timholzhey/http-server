#ifndef BASE64_H_INCLUDED
#define BASE64_H_INCLUDED

#include <stddef.h>

char *base64_encode(const void *buf, size_t size);

#endif // BASE64_H_INCLUDED