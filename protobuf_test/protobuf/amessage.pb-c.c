/* Generated by the protocol buffer compiler.  DO NOT EDIT! */

/* Do not generate deprecated warnings for self */
#ifndef PROTOBUF_C_NO_DEPRECATED
#define PROTOBUF_C_NO_DEPRECATED
#endif

#include "amessage.pb-c.h"
void   amessage__init
                     (AMessage         *message)
{
  static AMessage init_value = AMESSAGE__INIT;
  *message = init_value;
}
size_t amessage__get_packed_size
                     (const AMessage *message)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &amessage__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t amessage__pack
                     (const AMessage *message,
                      uint8_t       *out)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &amessage__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t amessage__pack_to_buffer
                     (const AMessage *message,
                      ProtobufCBuffer *buffer)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &amessage__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
AMessage *
       amessage__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (AMessage *)
     protobuf_c_message_unpack (&amessage__descriptor,
                                allocator, len, data);
}
void   amessage__free_unpacked
                     (AMessage *message,
                      ProtobufCAllocator *allocator)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &amessage__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
static const ProtobufCFieldDescriptor amessage__field_descriptors[2] =
{
  {
    "a",
    1,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_INT32,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(AMessage, a),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "b",
    2,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_INT32,
    PROTOBUF_C_OFFSETOF(AMessage, has_b),
    PROTOBUF_C_OFFSETOF(AMessage, b),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned amessage__field_indices_by_name[] = {
  0,   /* field[0] = a */
  1,   /* field[1] = b */
};
static const ProtobufCIntRange amessage__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor amessage__descriptor =
{
  PROTOBUF_C_MESSAGE_DESCRIPTOR_MAGIC,
  "AMessage",
  "AMessage",
  "AMessage",
  "",
  sizeof(AMessage),
  2,
  amessage__field_descriptors,
  amessage__field_indices_by_name,
  1,  amessage__number_ranges,
  (ProtobufCMessageInit) amessage__init,
  NULL,NULL,NULL    /* reserved[123] */
};
