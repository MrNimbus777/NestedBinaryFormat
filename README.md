# Nested Binary Format (NBF)

NBF is a lightweight library for serializing hierarchical typed data into a compact binary representation.

NBF provides:

- nested object structures
- typed homogeneous arrays
- raw binary blobs
- strings
- signed and unsigned integers
- floating-point values
- recursive encoding and decoding
- explicit memory ownership

---

# Installation

Include the header in one translation unit with `NBF_IMPLEMENTATION` defined:

```c
#define NBF_IMPLEMENTATION
#include "nested_binary_format.h"
```

In other files:

```c
#include "nested_binary_format.h"
```

You may also want to strip the names, then the inclusion should look like this:
```c
#define NBF_IMPLEMENTATION
#define NBF_STRIP_PREFIXES
#include "nested_binary_format.h"
```

---

# Basic Example

```c
#define NBF_STRIP_PREFIXES
#define NBF_IMPLEMENTATION
#include "nested_binary_format.h"

int main(){
    value_t value = STACK_NODE(
        FIELD("printf", STRING("HELLO WORLD"))
    );
    printf("To encode: "); nbf_print(&value); printf("\n");
    
    size_t buffer_size = nbf_sizeof(&value);
    byte buffer[buffer_size];

    nbf_encode(&value, buffer);

    printf("Encoded bytes: "); printf_bytes(buffer, buffer_size); printf("\n");

    byte* ptr = buffer;
    value_t decoded = nbf_decode(&ptr);

    printf("Decoded: "); nbf_print(&decoded); printf("\n");

    nbf_free(&decoded);
    return 0;
}
```

Output:

```txt
To encode: {"printf": "HELLO WORLD"}
Encoded bytes: 01 00 01 00 06 70 72 69 6E 74 66 04 00 00 00 0B 48 45 4C 4C 4F 20 57 4F 52 4C 44
Decoded: {"printf": "HELLO WORLD"}
```

## Future plans:
- To add files related functionality (read, write)
- To implement partial decoding
- To integrate a compression algorithm for the binary data
- To translate this lib into other programming languages (e.g. C++, Java, Go, Rust, JavaScript, Python)
- To conquer the world.
