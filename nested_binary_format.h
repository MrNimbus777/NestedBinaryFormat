#ifndef NESTED_BINARY_FORMAT_H
#define NESTED_BINARY_FORMAT_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define NBF_FILE_EXTENSION ".nbf"

#define NBF_TYPE_FAMILY_EMBPTY                                                                    \
    X(EMPTY, void*)

#define NBF_TYPE_FAMILY_STRUCT                                                                    \
    X(NODE, nbf_node_t)                                                                           \
    X(LIST, nbf_list_t)

#define NBF_TYPE_FAMILY_CHAR                                                                      \
    X(RAW,    nbf_raw_t)                                                                          \
    X(STRING, char*)

#define NBF_TYPE_FAMILY_INT                                                                       \
    X(INT8,    int8_t)                                                                            \
    X(INT16,   int16_t)                                                                           \
    X(INT32,   int32_t)                                                                           \
    X(INT64,   int64_t)                                                                           \
    X(UINT8,   uint8_t)                                                                           \
    X(UINT16,  uint16_t)                                                                          \
    X(UINT32,  uint32_t)                                                                          \
    X(UINT64,  uint64_t)

#define NBF_TYPE_FAMILY_FLOAT                                                                     \
    X(FLOAT32, float)                                                                             \
    X(FLOAT64, double)                                                                            \
        
#define NBF_TYPE_FAMILY_NUMERIC                                                                   \
    NBF_TYPE_FAMILY_INT                                                                           \
    NBF_TYPE_FAMILY_FLOAT

#define NBF_TYPE_FAMILY                                                                           \
    NBF_TYPE_FAMILY_EMBPTY                                                                        \
    NBF_TYPE_FAMILY_STRUCT                                                                        \
    NBF_TYPE_FAMILY_CHAR                                                                          \
    NBF_TYPE_FAMILY_NUMERIC

typedef enum NBF_TYPES {

#define X(x1, x2) NBF_TYPES_##x1,
NBF_TYPE_FAMILY
#undef X
    
    NBF_TYPES_COUNT
} NBF_TYPES;


typedef unsigned char byte;

typedef struct nbf_raw_t {
    byte*    data;
    uint32_t size;
} nbf_raw_t;
#define AS_BYTES(value) ((byte*)&(typeof(value)){ value })
static void printf_bytes(byte* data, size_t size){
    if(size > 0){
        printf("%02X", data[0]);
        for(size_t i = 1; i < size; i++) {
            printf(" %02X", data[i]);
        }
    }
}

static byte* string_as_bytes(char* in, byte* out, size_t n){
    for(char* c = in; *c != 0; c++){
        out[c-in] = *AS_BYTES(*c);
    }
    return out;
}

typedef struct nbf_typeless_value_t nbf_typeless_value_t;
typedef struct nbf_value_t          nbf_value_t;
typedef struct nbf_field_t          nbf_field_t;

typedef struct nbf_error_value_t {
    enum ERROR_CODES {
        NULL_PTR,
        DECODE_ERROR,
        ENCODE_ERROR
    } error_code;
} nbf_error_value_t;

typedef struct nbf_node_t {
    nbf_field_t* fields;
    uint16_t     size;
} nbf_node_t; 

typedef struct nbf_list_t {
    nbf_typeless_value_t* values;
    uint16_t              size;
    NBF_TYPES             type;
} nbf_list_t;

typedef enum nbf_ownership_t {
    NBF_OWNERDHIP_UNDEFINED,
    NBF_OWNERDHIP_HEAP
} nbf_ownership_t;

struct nbf_typeless_value_t {
    union {
        #define X(x1, x2) x2 x1;
        NBF_TYPE_FAMILY
        #undef X
    };
    nbf_ownership_t __ownership;
}; 

struct nbf_value_t {
    nbf_typeless_value_t typeless_value;
    NBF_TYPES            type;
};
struct nbf_field_t {
    char*       name;
    nbf_value_t value;
};

nbf_value_t nbf_decode(byte** buffer);
byte* nbf_encode(nbf_value_t* value, byte* buffer);
size_t nbf_sizeof(nbf_value_t* value);
void nbf_free(nbf_value_t* value);
void nbf_print(nbf_value_t* value);


// CONSTRUCTORS

#define NBF_EMPTY() ((nbf_value_t){                                                               \
    .type = NBF_TYPES_EMPTY                                                                       \
})

#define NBF_FIELD(name_, value_) ((nbf_field_t){                                                  \
    .name = (name_),                                                                              \
    .value = (nbf_value_t)(value_),                                                               \
})
#define NBF_NODE(...) ((nbf_value_t){                                                             \
    .type = NBF_TYPES_NODE,                                                                       \
    .typeless_value.NODE = (nbf_node_t){                                                          \
        .fields = (nbf_field_t[]){ __VA_ARGS__ },                                                 \
        .size = sizeof((nbf_field_t[]){ __VA_ARGS__ }) / sizeof(nbf_field_t)                      \
    },                                                                                             \
    .typeless_value.__ownership = NBF_OWNERDHIP_UNDEFINED                                         \
})

static inline nbf_typeless_value_t* nbf_value_to_typeless_value(nbf_value_t* values, size_t n){
    nbf_typeless_value_t* as_typeless = (nbf_typeless_value_t*) values;
    for(size_t i = 1; i < n; i++) {
        as_typeless[i] = *(nbf_typeless_value_t*)(values+i);
    }
    return as_typeless;
}
#define NBF_LIST(first_element, ...) ((nbf_value_t){                                              \
    .type = NBF_TYPES_LIST,                                                                       \
    .typeless_value.LIST = (nbf_list_t){                                                          \
        .values = nbf_value_to_typeless_value(                                                    \
            (nbf_value_t[]){ first_element, __VA_ARGS__ },                                        \
            1 + sizeof((nbf_value_t[]){ __VA_ARGS__ }) / sizeof(nbf_value_t)                      \
        ),                                                                                        \
        .size = 1 + sizeof((nbf_value_t[]){ __VA_ARGS__ }) / sizeof(nbf_value_t),                 \
        .type = first_element.type                                                                \
    },                                                                                            \
    .typeless_value.__ownership = NBF_OWNERDHIP_UNDEFINED                                         \
})

#define NBF_RAW(...) ((nbf_value_t){                                                              \
    .type = NBF_TYPES_RAW,                                                                        \
    .typeless_value.RAW = (nbf_raw_t) {                                                           \
        .data = (byte[]){ __VA_ARGS__ },                                                          \
        .size = sizeof((byte[]){ __VA_ARGS__ }) / sizeof(byte)                                    \
    },                                                                                            \
    .typeless_value.__ownership = NBF_OWNERDHIP_UNDEFINED                                         \
})
#define NBF_BYTES_TO_RAW(bytes_ptr, size) ((nbf_value_t){                                         \
    .type = NBF_TYPES_RAW,                                                                        \
    .typeless_value.RAW = (nbf_raw_t) {                                                           \
        .data = bytes_ptr,                                                                        \
        .size = size                                                                              \
    },                                                                                            \
    .typeless_value.__ownership = NBF_OWNERDHIP_UNDEFINED                                         \
})
#define NBF_STRING(v) ((nbf_value_t){                                                             \
    .type = NBF_TYPES_STRING,                                                                     \
    .typeless_value.STRING = (v),                                                                 \
    .typeless_value.__ownership = NBF_OWNERDHIP_UNDEFINED                                         \
})

#define NBF_INT8(v) ((nbf_value_t){                                                               \
    .type = NBF_TYPES_INT8,                                                                       \
    .typeless_value.INT8 = (v),                                                                   \
    .typeless_value.__ownership = NBF_OWNERDHIP_UNDEFINED                                         \
})
#define NBF_INT16(v) ((nbf_value_t){                                                              \
    .type = NBF_TYPES_INT16,                                                                      \
    .typeless_value.INT16 = (v),                                                                  \
    .typeless_value.__ownership = NBF_OWNERDHIP_UNDEFINED                                         \
})
#define NBF_INT32(v) ((nbf_value_t){                                                              \
    .type = NBF_TYPES_INT32,                                                                      \
    .typeless_value.INT32 = (v),                                                                  \
    .typeless_value.__ownership = NBF_OWNERDHIP_UNDEFINED                                         \
})
#define NBF_INT64(v) ((nbf_value_t){                                                              \
    .type = NBF_TYPES_INT64,                                                                      \
    .typeless_value.INT64 = (v),                                                                  \
    .typeless_value.__ownership = NBF_OWNERDHIP_UNDEFINED                                         \
})
#define NBF_UINT8(v) ((nbf_value_t){                                                              \
    .type = NBF_TYPES_UINT8,                                                                      \
    .typeless_value.UINT8 = (v),                                                                  \
    .typeless_value.__ownership = NBF_OWNERDHIP_UNDEFINED                                         \
})
#define NBF_UINT16(v) ((nbf_value_t){                                                             \
    .type = NBF_TYPES_UINT16,                                                                     \
    .typeless_value.UINT16 = (v),                                                                 \
    .typeless_value.__ownership = NBF_OWNERDHIP_UNDEFINED                                         \
})
#define NBF_UINT32(v) ((nbf_value_t){                                                             \
    .type = NBF_TYPES_UINT32,                                                                     \
    .typeless_value.UINT32 = (v),                                                                 \
    .typeless_value.__ownership = NBF_OWNERDHIP_UNDEFINED                                         \
})
#define NBF_UINT64(v) ((nbf_value_t){                                                             \
    .type = NBF_TYPES_UINT64,                                                                     \
    .typeless_value.UINT64 = (v),                                                                 \
    .typeless_value.__ownership = NBF_OWNERDHIP_UNDEFINED                                         \
})

#define NBF_FLOAT32(v) ((nbf_value_t){                                                            \
    .type = NBF_TYPES_FLOAT32,                                                                    \
    .typeless_value.FLOAT32 = (v),                                                                \
    .typeless_value.__ownership = NBF_OWNERDHIP_UNDEFINED                                         \
})
#define NBF_FLOAT64(v) ((nbf_value_t){                                                            \
    .type = NBF_TYPES_FLOAT64,                                                                    \
    .typeless_value.FLOAT64 = (v),                                                                \
    .typeless_value.__ownership = NBF_OWNERDHIP_UNDEFINED                                         \
})


#ifdef NBF_STRIP_PREFIXES
    typedef struct nbf_typeless_value_t typeless_value_t;
    typedef struct nbf_value_t          value_t;
    typedef struct nbf_node_t           node_t;
    typedef struct nbf_field_t          field_t;
    typedef struct nbf_list_t           list_t;
    typedef struct nbf_raw_t            raw_t;
    #define EMPTY()                       NBF_EMPTY()
    #define FIELD(name_, value_)          NBF_FIELD(name_, value_)
    #define NODE(...)                     NBF_NODE(__VA_ARGS__)
    #define LIST(first_element, ...)      NBF_LIST(first_element, __VA_ARGS__)
    #define RAW(...)                      NBF_RAW(__VA_ARGS__)
    #define BYTES_TO_RAW(bytes_ptr, size) NBF_BYTES_TO_RAW(bytes_ptr, size)
    #define STRING(v)                     NBF_STRING(v)
    #define INT8(v)                       NBF_INT8(v)
    #define INT16(v)                      NBF_INT16(v)
    #define INT32(v)                      NBF_INT32(v)
    #define INT64(v)                      NBF_INT64(v)
    #define UINT8(v)                      NBF_UINT8(v)
    #define UINT16(v)                     NBF_UINT16(v)
    #define UINT32(v)                     NBF_UINT32(v)
    #define UINT64(v)                     NBF_UINT64(v)
    #define FLOAT32(v)                    NBF_FLOAT32(v)
    #define FLOAT64(v)                    NBF_FLOAT64(v)
#endif // NBF_STRIP_PREFIXES

#ifdef NBF_IMPLEMENTATION
// DECLARE DECODE FUNCTIONS AND FUNCTION TABLE

typedef nbf_value_t(*nbf_decode_type_f)(byte**);
#define X(x1, x2) nbf_value_t nbf_decode_##x1(byte**);
NBF_TYPE_FAMILY
#undef X
static const nbf_decode_type_f NBF_DECODE_FUNCTION_TABLE[NBF_TYPES_COUNT] = {
    #define X(x1, x2) nbf_decode_##x1,
    NBF_TYPE_FAMILY
    #undef X
};
nbf_value_t nbf_decode(byte** buffer){
    return NBF_DECODE_FUNCTION_TABLE[*((*buffer)++)](buffer);
}


// DECLARE ENCODE FUNCTIONS AND FUNCTION TABLE

typedef byte*(*nbf_encode_type_f)(nbf_typeless_value_t*, byte*);
#define X(x1, x2) byte* nbf_encode_##x1(nbf_typeless_value_t*, byte*);
NBF_TYPE_FAMILY
#undef X
static const nbf_encode_type_f NBF_ENCODE_FUNCTION_TABLE[NBF_TYPES_COUNT] = {
    #define X(x1, x2) nbf_encode_##x1,
    NBF_TYPE_FAMILY
    #undef X
};
byte* nbf_encode(nbf_value_t* value, byte* buffer){
    return NBF_ENCODE_FUNCTION_TABLE[value->type](&value->typeless_value, buffer);
}


// DECLARE SIZEOF FUNCTIONS AND FUNCTION TABLE

typedef size_t(*nbf_sizeof_type_f)(nbf_typeless_value_t*);
#define X(x1, x2) size_t nbf_sizeof_##x1(nbf_typeless_value_t*);
NBF_TYPE_FAMILY
#undef X
static const nbf_sizeof_type_f NBF_SIZEOF_FUNCTION_TABLE[NBF_TYPES_COUNT] = {
    #define X(x1, x2) nbf_sizeof_##x1,
    NBF_TYPE_FAMILY
    #undef X
};
size_t nbf_sizeof(nbf_value_t* value){
    return NBF_SIZEOF_FUNCTION_TABLE[value->type](&value->typeless_value);
}


// DECLARE FREE FUNCTIONS AND FUNCTION TABLE

typedef void(*nbf_free_type_f)(nbf_typeless_value_t*);
#define X(x1, x2) void nbf_free_##x1(nbf_typeless_value_t*);
NBF_TYPE_FAMILY_STRUCT
NBF_TYPE_FAMILY_CHAR
#undef X
static void nbf_free_ignored_(nbf_typeless_value_t* ignored){(void)ignored;}
static const nbf_free_type_f NBF_FREE_FUNCTION_TABLE[NBF_TYPES_COUNT] = {
    #define X(x1, x2) nbf_free_ignored_,
    NBF_TYPE_FAMILY_EMBPTY
    #undef X
    #define X(x1, x2) nbf_free_##x1,
    NBF_TYPE_FAMILY_STRUCT
    NBF_TYPE_FAMILY_CHAR
    #undef X
    #define X(x1, x2) nbf_free_ignored_,
    NBF_TYPE_FAMILY_NUMERIC
    #undef X
};
void nbf_free(nbf_value_t* value){
    return NBF_FREE_FUNCTION_TABLE[value->type](&value->typeless_value);
}


// DECLARE PRINT FUNCTIONS AND FUNCTION TABLE

typedef void(*nbf_print_type_f)(nbf_typeless_value_t*);
#define X(x1, x2) void nbf_print_##x1(nbf_typeless_value_t*);
NBF_TYPE_FAMILY
#undef X
static const nbf_print_type_f NBF_PRINT_FUNCTION_TABLE[NBF_TYPES_COUNT] = {
    #define X(x1, x2) nbf_print_##x1,
    NBF_TYPE_FAMILY
    #undef X
};
void nbf_print(nbf_value_t* value){
    return NBF_PRINT_FUNCTION_TABLE[value->type](&value->typeless_value);
}



static void* nbf_memcpy(byte* dest, const byte* source, size_t n) {
    void* start = dest;
    while(n--){
        *(dest++) = *(source++);
    }
    return start;
}

static inline void nbf_write_64(byte* buf, uint64_t v) {
    buf[0] = (v >> 56) & 0xFF;
    buf[1] = (v >> 48) & 0xFF;
    buf[2] = (v >> 40) & 0xFF;
    buf[3] = (v >> 32) & 0xFF;
    buf[4] = (v >> 24) & 0xFF;
    buf[5] = (v >> 16) & 0xFF;
    buf[6] = (v >>  8) & 0xFF;
    buf[7] = (v >>  0) & 0xFF;
}
static inline void nbf_write_32(byte* buf, uint32_t v) { 
    buf[0] = (v >> 24) & 0xFF;
    buf[1] = (v >> 16) & 0xFF;
    buf[2] = (v >>  8) & 0xFF;
    buf[3] = (v >>  0) & 0xFF;
}
static inline void nbf_write_16(byte* buf, uint16_t v) {
    buf[0] = (v >> 8) & 0xFF;
    buf[1] = (v >> 0) & 0xFF;
}
static inline void nbf_write_8(byte* buf, uint8_t v) {
    buf[0] = v;
}

static inline uint64_t nbf_read_64(const byte* buf) {
    return ((uint64_t)buf[0] << 56) |
           ((uint64_t)buf[1] << 48) |
           ((uint64_t)buf[2] << 40) |
           ((uint64_t)buf[3] << 32) |
           ((uint64_t)buf[4] << 24) |
           ((uint64_t)buf[5] << 16) |
           ((uint64_t)buf[6] <<  8) |
           ((uint64_t)buf[7] <<  0);
}
static inline uint32_t nbf_read_32(const byte* buf) {
    return ((uint32_t)buf[0] << 24) |
           ((uint32_t)buf[1] << 16) |
           ((uint32_t)buf[2] <<  8) |
           ((uint32_t)buf[3] <<  0);
}
static inline uint16_t nbf_read_16(const byte* buf) {
    return ((uint16_t)buf[0] << 8) |
           ((uint16_t)buf[1] << 0);
}
static inline uint8_t nbf_read_8(const byte* buf) {
    return buf[0];
}



nbf_value_t nbf_decode_EMPTY(byte** buffer){
    *buffer += 0;
    return NBF_EMPTY();
}

nbf_value_t nbf_decode_NODE(byte** buffer){
    uint16_t size = nbf_read_16(*buffer);
    *buffer += sizeof(uint16_t);

    nbf_field_t* fields = malloc(size*sizeof(nbf_field_t));

    for(uint16_t i = 0; i < size; i++){    
        uint16_t name_len = nbf_read_16(*buffer);
        *buffer += sizeof(uint16_t);
        
        char* name = nbf_memcpy(malloc((name_len+1)*sizeof(char)), *buffer, name_len);
        name[name_len] = 0;
        *buffer += name_len;

        fields[i] = (nbf_field_t) {
            .name = name,
            .value = nbf_decode(buffer)
        };
    }

    return (nbf_value_t){
        .type = NBF_TYPES_NODE,
        .typeless_value.NODE = {
            .fields = fields,
            .size = size
        },
        .typeless_value.__ownership = NBF_OWNERDHIP_HEAP
    };
}

nbf_value_t nbf_decode_LIST(byte** buffer){
    uint16_t size = nbf_read_16(*buffer);
    *buffer += sizeof(uint16_t);

    uint8_t type = nbf_read_8(*buffer);
    *buffer += sizeof(uint8_t);

    nbf_typeless_value_t* values = malloc(size*sizeof(nbf_typeless_value_t));

    for(uint16_t i = 0; i < size; i++){
        values[i] = NBF_DECODE_FUNCTION_TABLE[type](buffer).typeless_value;
    }

    return (nbf_value_t){
        .type = NBF_TYPES_LIST,
        .typeless_value.LIST = {
            .values = values,
            .size = size,
            .type = type
        },
        .typeless_value.__ownership = NBF_OWNERDHIP_HEAP
    };
}

nbf_value_t nbf_decode_RAW(byte** buffer){
    uint32_t size = sizeof(byte)*nbf_read_32(*buffer);
    byte* bytes = nbf_memcpy(malloc(size), (*buffer)+2, size);
    (*buffer)+=sizeof(uint16_t)+size;
    return (nbf_value_t){
        .type = NBF_TYPES_RAW,
        .typeless_value = (nbf_typeless_value_t) {
            .RAW = (nbf_raw_t) {
                .data = bytes,
                .size = size
            },
            .__ownership = NBF_OWNERDHIP_HEAP
        }
    };
}

nbf_value_t nbf_decode_STRING(byte** buffer){
    uint32_t size = sizeof(char)*nbf_read_32(*buffer);
    char* str = nbf_memcpy(malloc(size+sizeof(char)), (*buffer)+sizeof(uint32_t), size);
    str[size] = 0;
    (*buffer)+=sizeof(uint32_t)+size;
    return (nbf_value_t){
        .type = NBF_TYPES_STRING,
        .typeless_value = (nbf_typeless_value_t) {
            .STRING = str,
            .__ownership = NBF_OWNERDHIP_HEAP
        }
    };
}

nbf_value_t nbf_decode_INT8(byte** buffer){
    int8_t val = *((int8_t*)(*buffer));
    (*buffer) += sizeof(int8_t);
    return NBF_INT8(val);
}
nbf_value_t nbf_decode_INT16(byte** buffer){
    int16_t val = *((int16_t*)(*buffer));
    (*buffer) += sizeof(int16_t);
    return NBF_INT16(val);
}
nbf_value_t nbf_decode_INT32(byte** buffer){
    int32_t val = *((int32_t*)(*buffer));
    (*buffer) += sizeof(int32_t);
    return NBF_INT32(val);
}
nbf_value_t nbf_decode_INT64(byte** buffer){
    int64_t val = *((int64_t*)(*buffer));
    (*buffer) += sizeof(int64_t);
    return NBF_INT64(val);
}
nbf_value_t nbf_decode_UINT8(byte** buffer){
    uint8_t val = *((uint8_t*)(*buffer));
    (*buffer) += sizeof(uint8_t);
    return NBF_UINT8(val);
}
nbf_value_t nbf_decode_UINT16(byte** buffer){
    uint16_t val = *((uint16_t*)(*buffer));
    (*buffer) += sizeof(uint16_t);
    return NBF_UINT16(val);
}
nbf_value_t nbf_decode_UINT32(byte** buffer){
    uint32_t val = *((uint32_t*)(*buffer));
    (*buffer) += sizeof(uint32_t);
    return NBF_UINT32(val);
}
nbf_value_t nbf_decode_UINT64(byte** buffer){
    uint64_t val = *((uint64_t*)(*buffer));
    (*buffer) += sizeof(uint64_t);
    return NBF_UINT64(val);
}

nbf_value_t nbf_decode_FLOAT32(byte** buffer){
    float val = *((float*)(*buffer));
    (*buffer) += sizeof(float);
    return NBF_FLOAT32(val);
}

nbf_value_t nbf_decode_FLOAT64(byte** buffer){
    double val = *((double*)(*buffer));
    (*buffer) += sizeof(double);
    return NBF_FLOAT64(val);
}



void nbf_free_NODE(nbf_typeless_value_t* value){
    if(value->__ownership == NBF_OWNERDHIP_UNDEFINED) return;
    nbf_node_t node = value->NODE;
    for(uint16_t i = 0; i < node.size; i++) {
        nbf_free(&node.fields[i].value);
    }
    free(node.fields);
}

void nbf_free_LIST(nbf_typeless_value_t* value){
    if(value->__ownership == NBF_OWNERDHIP_UNDEFINED) return;
    nbf_list_t list = value->LIST;
    for(uint16_t i = 0; i < list.size; i++) {
        NBF_FREE_FUNCTION_TABLE[list.type](list.values+i);
    }
    free(list.values);
}

void nbf_free_RAW(nbf_typeless_value_t* value){
    if(value->__ownership == NBF_OWNERDHIP_UNDEFINED) return;
    free(value->RAW.data);
}

void nbf_free_STRING(nbf_typeless_value_t* value){
    if(value->__ownership == NBF_OWNERDHIP_UNDEFINED) return;
    free(value->STRING);
}




byte* nbf_encode_EMPTY(nbf_typeless_value_t* value, byte* buffer){
    (void)value;
    *buffer = 0;
    return buffer;
}

byte* nbf_encode_NODE(nbf_typeless_value_t* value, byte* buffer){
    *buffer = NBF_TYPES_NODE;
    nbf_node_t node = value->NODE;
    nbf_write_16(buffer+1, node.size);   // fields count 
    size_t padding = sizeof(uint16_t)+1; // first 3 bytes occupied 
    for(uint16_t i = 0; i < node.size; i++) {
        nbf_field_t field = node.fields[i];

        char* str = field.name;
        uint16_t n = 0;
        n = 0;
        padding += sizeof(uint16_t);
        for(char* c = str; *c != '\0'; c++) {            
            buffer[padding+n] = *(byte*)c;
            n++;
        }
        nbf_write_16(buffer+padding-sizeof(uint16_t), n);
        padding += n;

        nbf_encode(&field.value, buffer+padding);

        padding += nbf_sizeof(&field.value);
    }
    return buffer;
}

byte* nbf_encode_LIST(nbf_typeless_value_t* value, byte* buffer){
    *buffer = NBF_TYPES_LIST;
    nbf_list_t list = value->LIST;
    size_t padding = sizeof(uint16_t)+1; 
    nbf_write_16(buffer+1, list.size);   // elements count
    nbf_write_8(buffer+3, list.type);    // elements type
    for(uint16_t i = 0; i < list.size; i++) {
        byte tmp = buffer[padding];
        NBF_ENCODE_FUNCTION_TABLE[list.type](list.values+i, buffer+padding);
        buffer[padding] = tmp;
        padding += NBF_SIZEOF_FUNCTION_TABLE[list.type](list.values+i)-1;
    }
    return buffer;
}

byte* nbf_encode_RAW(nbf_typeless_value_t* value, byte* buffer){
    *buffer = NBF_TYPES_RAW;
    nbf_raw_t raw = value->RAW;
    for(size_t i = 0; i < raw.size; i++) {
        buffer[sizeof(uint32_t)+i+1] = raw.data[i];
    }
    nbf_write_32(buffer+1, raw.size);   // bytes count
    return buffer;
}

byte* nbf_encode_STRING(nbf_typeless_value_t* value, byte* buffer){
    *buffer = NBF_TYPES_STRING;
    char* str = value->STRING;
    uint32_t n = 0;
    for(char* c = str; *c != '\0'; c++) {
        buffer[sizeof(uint32_t)+(n++)+1] = *(byte*)c;
    }
    nbf_write_32(buffer+1, n);   // string len
    return buffer;
}

byte* nbf_encode_INT8(nbf_typeless_value_t* value, byte* buffer){
    *buffer = NBF_TYPES_INT8;
    nbf_write_8(buffer+1, value->INT8);
    return buffer;
}
byte* nbf_encode_INT16(nbf_typeless_value_t* value, byte* buffer){
    *buffer = NBF_TYPES_INT16;
    nbf_write_16(buffer+1, value->INT16);
    return buffer;
}
byte* nbf_encode_INT32(nbf_typeless_value_t* value, byte* buffer){
    *buffer = NBF_TYPES_INT32;
    nbf_write_32(buffer+1, value->INT32);
    return buffer;
}
byte* nbf_encode_INT64(nbf_typeless_value_t* value, byte* buffer){
    *buffer = NBF_TYPES_INT64;
    nbf_write_64(buffer+1, value->INT64);
    return buffer;
}
byte* nbf_encode_UINT8(nbf_typeless_value_t* value, byte* buffer){
    *buffer = NBF_TYPES_UINT8;
    nbf_write_8(buffer+1, value->UINT8);
    return buffer;
}
byte* nbf_encode_UINT16(nbf_typeless_value_t* value, byte* buffer){
    *buffer = NBF_TYPES_UINT16;
    nbf_write_16(buffer+1, value->UINT16);
    return buffer;
}
byte* nbf_encode_UINT32(nbf_typeless_value_t* value, byte* buffer){
    *buffer = NBF_TYPES_UINT32;
    nbf_write_32(buffer+1, value->UINT32);
    return buffer;
}
byte* nbf_encode_UINT64(nbf_typeless_value_t* value, byte* buffer){
    *buffer = NBF_TYPES_UINT64;
    nbf_write_64(buffer+1, value->UINT64);
    return buffer;
}

byte* nbf_encode_FLOAT32(nbf_typeless_value_t* value, byte* buffer){
    *buffer = NBF_TYPES_FLOAT32;
    nbf_write_32(buffer + 1, *(uint32_t*)&value->FLOAT32);
    return buffer;
}

byte* nbf_encode_FLOAT64(nbf_typeless_value_t* value, byte* buffer){
    *buffer = NBF_TYPES_FLOAT64;
    nbf_write_64(buffer + 1, *(uint64_t*)&value->FLOAT64);
    return buffer;
}



size_t nbf_sizeof_EMPTY(nbf_typeless_value_t* value){
    (void)value;
    return 1; // header only which is 0x00
}

size_t nbf_sizeof_NODE(nbf_typeless_value_t* value){
    nbf_node_t node = value->NODE;
    size_t size = 1 +               // header
                  sizeof(uint16_t); // fields count
    for(size_t i = 0; i < node.size; i++){
        nbf_field_t field = node.fields[i];
        
        char* str = field.name;
        uint16_t n = 0;
        for(char* c = str; *c != '\0'; c++) n++;

        size += NBF_SIZEOF_FUNCTION_TABLE[field.value.type](&field.value.typeless_value) +
                2 + // 2 bytes for name len
                n;  // name content
    }
    return size;
}

size_t nbf_sizeof_LIST(nbf_typeless_value_t* value){
    nbf_list_t list = value->LIST;
    size_t size = 1 +               // header
                  1 +               // type
                  sizeof(uint16_t); // fields count
    for(size_t i = 0; i < list.size; i++){
        size += NBF_SIZEOF_FUNCTION_TABLE[list.type](list.values+i) - 
                1; // substracting the header as it is already hold in the list.type.
    }
    return size;
}

size_t nbf_sizeof_RAW(nbf_typeless_value_t* value){
    return sizeof(byte)*value->RAW.size + // raw content
           sizeof(uint32_t) +             // raw len
           1;                             // header
}

size_t nbf_sizeof_STRING(nbf_typeless_value_t* value){
    char* str = value->STRING;
    size_t n = 0;
    for(char* c = str; *c != '\0'; c++) n++;
    return sizeof(char)*n +   // string content
           sizeof(uint32_t) + // string len
           1;                 // header
}
size_t nbf_sizeof_INT8(nbf_typeless_value_t* value){
    static size_t size = sizeof(int8_t) +   // value
                         1;                 // header
    return size;
}
size_t nbf_sizeof_INT16(nbf_typeless_value_t* value){
    static size_t size = sizeof(int16_t) +   // value
                         1;                 // header
    return size;
}
size_t nbf_sizeof_INT32(nbf_typeless_value_t* value){
    static size_t size = sizeof(int32_t) +   // value
                         1;                 // header
    return size;
}
size_t nbf_sizeof_INT64(nbf_typeless_value_t* value){
    static size_t size = sizeof(int64_t) +   // value
                         1;                 // header
    return size;
}
size_t nbf_sizeof_UINT8(nbf_typeless_value_t* value){
    static size_t size = sizeof(uint8_t) +   // value
                         1;                 // header
    return size;
}
size_t nbf_sizeof_UINT16(nbf_typeless_value_t* value){
    static size_t size = sizeof(uint16_t) +   // value
                         1;                 // header
    return size;
}
size_t nbf_sizeof_UINT32(nbf_typeless_value_t* value){
    static size_t size = sizeof(uint32_t) +   // value
                         1;                 // header
    return size;
}
size_t nbf_sizeof_UINT64(nbf_typeless_value_t* value){
    static size_t size = sizeof(uint64_t) +   // value
                         1;                 // header
    return size;
}

size_t nbf_sizeof_FLOAT32(nbf_typeless_value_t* value){
    static size_t size = sizeof(float) +   // value
                         1;                 // header
    return size;
}

size_t nbf_sizeof_FLOAT64(nbf_typeless_value_t* value){
    static size_t size = sizeof(double) +   // value
                         1;                 // header
    return size;
}



void nbf_print_EMPTY(nbf_typeless_value_t* value){
    (void)value;
    printf("null");
}

void nbf_print_NODE(nbf_typeless_value_t* value){
    nbf_node_t node = value->NODE;
    printf("{");
    if(node.size > 0){
        nbf_field_t field = node.fields[0];
        printf("\"%s\": ", field.name); nbf_print(&field.value);
        for(size_t i = 1; i < node.size; i++) {
            nbf_field_t field = node.fields[i];
            printf(", \"%s\": ", field.name); nbf_print(&field.value);
        }
    }
    printf("}");
}

void nbf_print_LIST(nbf_typeless_value_t* value){
    nbf_list_t list = value->LIST;
    printf("[");
    if(list.size > 0){
        NBF_PRINT_FUNCTION_TABLE[list.type](list.values);
        for(size_t i = 1; i < list.size; i++) {
            printf(", "); NBF_PRINT_FUNCTION_TABLE[list.type](list.values+i);
        }
    }
    printf("]");
}

void nbf_print_RAW(nbf_typeless_value_t* value){
    nbf_raw_t raw = value->RAW;
    printf_bytes(raw.data, raw.size);
}

void nbf_print_STRING(nbf_typeless_value_t* value){
    char* str = value->STRING;
    printf("\"%s\"", str);
}

void nbf_print_INT8(nbf_typeless_value_t* value){
    printf("%d", value->INT8);
}
void nbf_print_INT16(nbf_typeless_value_t* value){
    printf("%d", value->INT16);
}
void nbf_print_INT32(nbf_typeless_value_t* value){
    printf("%d", value->INT32);
}
void nbf_print_INT64(nbf_typeless_value_t* value){
    printf("%lld", value->INT64);
}
void nbf_print_UINT8(nbf_typeless_value_t* value){
    printf("%u", value->INT8);
}
void nbf_print_UINT16(nbf_typeless_value_t* value){
    printf("%u", value->INT16);
}
void nbf_print_UINT32(nbf_typeless_value_t* value){
    printf("%u", value->INT32);
}
void nbf_print_UINT64(nbf_typeless_value_t* value){
    printf("%llu", value->INT64);
}

void nbf_print_FLOAT32(nbf_typeless_value_t* value){
    printf("%f", value->FLOAT32);
}
void nbf_print_FLOAT64(nbf_typeless_value_t* value){
    printf("%lf", value->FLOAT64);
}
#endif

#endif // NESTED_BINARY_FORMAT_H