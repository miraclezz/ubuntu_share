/* Generated by the protocol buffer compiler.  DO NOT EDIT! */

#ifndef PROTOBUF_C_test_2eperson_2eproto__INCLUDED
#define PROTOBUF_C_test_2eperson_2eproto__INCLUDED

#include <google/protobuf-c/protobuf-c.h>

PROTOBUF_C_BEGIN_DECLS


typedef struct _Person Person;


/* --- enums --- */


/* --- messages --- */

struct  _Person
{
  ProtobufCMessage base;
  int32_t id;
  char *name;
  char *major;
};
#define PERSON__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&person__descriptor) \
    , 0, NULL, NULL }


/* Person methods */
void   person__init
                     (Person         *message);
size_t person__get_packed_size
                     (const Person   *message);
size_t person__pack
                     (const Person   *message,
                      uint8_t             *out);
size_t person__pack_to_buffer
                     (const Person   *message,
                      ProtobufCBuffer     *buffer);
Person *
       person__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   person__free_unpacked
                     (Person *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*Person_Closure)
                 (const Person *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor person__descriptor;

PROTOBUF_C_END_DECLS


#endif  /* PROTOBUF_test_2eperson_2eproto__INCLUDED */