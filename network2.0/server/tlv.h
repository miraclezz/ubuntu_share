#ifndef __TLV_H__
#define __TLV_H__

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

#if 1
#define sizeof_member(TYPE, MEMBER) (sizeof(((TYPE *)0)->MEMBER))
#define member_info(t, m) offsetof(t, m), sizeof_member(t, m)
#endif

/*the struct of tlv*/
typedef enum msg_type_s {
	TLV_ID_TYPE = 0,
	TLV_NAME_TYPE,		
	TLV_MAJOR_TYPE,
	TLV_MSG_MAX
} msg_type_t;


struct tlv_template{
	uint16_t	tag;		/*����tlv��tag*/
	uint16_t	length;	/*����tlv��value����*/
	char		value[0];	/*���������������ղ�����ֵ*/
};

typedef enum data_type_s{
	DATA_TYPE_NONE = 0,
	DATA_TYPE_U8,
	DATA_TYPE_U16,
	DATA_TYPE_U32,
	DATA_TYPE_STRING,
	DATA_TYPE_MAX,
}data_type_t;

struct tlv_arg_desc {
	const int tag; /* tlv tag */
	data_type_t type; 		  /* �������� */
	size_t offset;       /* �ڽṹ���ڵ�ƫ�� */
	size_t size; 		  /* ����ռ�ÿռ�Ĵ�С */
};

int struct2tlv(void *out, const void *in, const struct tlv_arg_desc *desc);
int tlv2struct(void *out, const char *tlv_src, size_t size_tlv, const struct tlv_arg_desc *desc);
int tlv2buff(void *out, size_t out_len, const char *tlv_src, size_t size_tlv,int tlv_tag);

#endif
