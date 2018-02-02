#include <string.h>
#include <arpa/inet.h>
#include <assert.h>
#include "tlv.h"

#define change_type_u16(method,x,y)		(y=(method(*((uint16_t *)(x)))))
#define change_type_u32(method,x,y)		(y=(method(*((uint32_t *)(x)))))

#define ERROR printf 
#define INFO printf 

int struct2tlv(void *out, const void *in, const struct tlv_arg_desc *desc)
{
	const struct tlv_arg_desc *arg = NULL;
	//struct tlv_template *tlv_t = NULL; 
	char *pin= (char *)in;
	char *pout = (char *)out;
	int len = 0;
	uint16_t tpara_u16;
	uint16_t tmp_16;
	uint32_t tmp_32;
	uint32_t tpara_u32;
	arg = desc;
	char tlv_struct[sizeof(struct tlv_template)]={0};
	struct tlv_template *tlv_head = NULL;


	while(arg->tag != TLV_MSG_MAX){
		tlv_head = (struct tlv_template *)(tlv_struct);
		tlv_head->tag = htons(arg->tag);
		tlv_head->length = htons(arg->size);

		switch(arg->type){
			case DATA_TYPE_U16:
				memcpy(&tmp_16, pin+arg->offset, arg->size);
				change_type_u16(htons, &tmp_16, tpara_u16);
				memcpy(pout+len, tlv_struct, sizeof(struct tlv_template));
				memcpy(pout + len + sizeof(struct tlv_template), (char *)&tpara_u16, arg->size);
				break;

			case DATA_TYPE_U32:
				memcpy(&tmp_32, pin+arg->offset, arg->size);
				change_type_u32(htonl,&tmp_32,tpara_u32);
				memcpy(pout+len, tlv_struct, sizeof(struct tlv_template));
				memcpy(pout + len + sizeof(struct tlv_template), (char *)&tpara_u32, arg->size);
				break;

			case	DATA_TYPE_U8:
				memcpy(pout+len, tlv_struct, sizeof(struct tlv_template));
				memcpy(pout + len + sizeof(struct tlv_template), pin+arg->offset, arg->size);
				break;

			case	DATA_TYPE_STRING:
				tlv_head->length = strlen(pin + arg->offset);
				memcpy(pout + len + sizeof(struct tlv_template), pin+arg->offset, tlv_head->length);
				tlv_head->length = htons(tlv_head->length);
				memcpy(pout + len, tlv_struct, sizeof(struct tlv_template));
				break;

			default:
				ERROR("unsupport this type\n");
				break;
		}

		len += sizeof(struct tlv_template) + ntohs(tlv_head->length);
		arg++;
	}

	return len;
}
void tlv_hexdump(char *d, int l, char *w)
{
	int i = 0, j = 0;
	char c;

	INFO("dumping %s:", w);
	for (i = 0; i < l; i++) {
		if (i % 16 == 0) {
			fprintf(stdout, "\t");
			for (j = i - 16; j >= 0 && j < i; j++) {
				c = *(d+j);
				fprintf(stdout, "%c", c < 0x20 ? '.': c);
			}
		}
		fprintf(stdout, "%c", (i % 16 == 0) ? '\n' : ' ');

		fprintf(stdout, "%02hhX", *(d+i));
	}

	for (j = i % 16; j > 0 && j < 16; j++) {
		fprintf(stdout, "%3s", "");
	}

	fprintf(stdout, "\t");

	for (j = l - (l % 16); j >= 0 && j < l; j++) {
		c = *(d+j);
		fprintf(stdout, "%c", c < 0x20 ? '.': c);
	}

	fprintf(stdout, "%c\n", (i % 16 == 0) ? '\0' : '\n');
}
int tlv2struct(void *out, const char *tlv_src, size_t size_tlv, const struct tlv_arg_desc *desc)
{
	const struct tlv_arg_desc *arg = NULL;
	int found;
	char *pwr = NULL; 
	uint16_t tpara_u16;
	uint32_t tpara_u32;
	/*tlv真真tlv_src真真*/
	int pos_t = 0;
	/**/
	//tlv_hexdump(tlv_src,size_tlv,"tlv:");
	int tlen = 0, ttag;
	char tlv_struct[sizeof(struct tlv_template)]={0};
	const char *tlv_value = NULL;
	uint32_t value_32;
	uint16_t value_16;

	struct tlv_template *tlv_head = NULL;
	int i = 0;

	while(pos_t < size_tlv){
		memcpy(tlv_struct, tlv_src + pos_t, sizeof(struct tlv_template));
		tlv_head = (struct tlv_template *)(tlv_struct);
		tlv_value = (tlv_src + pos_t) + sizeof(struct tlv_template);
		found = 0;
		tlen = ntohs(tlv_head->length);
		ttag =  ntohs(tlv_head->tag);

		for (arg = desc; arg->tag != TLV_MSG_MAX; arg++) {
			if (arg->tag == ttag && arg->size >= tlen) {
				found = 1;
				break;
			}
		}

		if (found == 0) {
			ERROR("wrong paramter tag or length\n");
			return -1;
		}

		pwr = (char *)out + arg->offset;


		switch(arg->type){
			case DATA_TYPE_U16:
				memcpy(&value_16,tlv_value, tlen);
				change_type_u16(ntohs,&value_16,tpara_u16);
				memcpy(pwr, (char *)&tpara_u16, tlen);
				break;

			case DATA_TYPE_U32:
				memcpy(&value_32, tlv_value, tlen);
				change_type_u32(ntohl,&value_32,tpara_u32);
				memcpy(pwr, (char *)&tpara_u32, tlen);
				break;

			case DATA_TYPE_U8:
			case DATA_TYPE_STRING:
				memcpy(pwr, tlv_value, tlen);
					pwr[tlen] = '\0';
					break;

			default:
					ERROR("unsupport this type\n");
					break;
		}

		pos_t += (tlen + sizeof(struct tlv_template));
	}

	return 0;
}
int tlv2buff(void *out, size_t out_len, const char *tlv_src, size_t size_tlv,int tlv_tag)
{
	//tlv真真tlv_src真真
	int pos_t = 0;
	//
	int tlen = 0, ttag;
	//struct tlv_template *tlv_t = NULL;
	struct tlv_template *tlv_head = NULL;
	struct tlv_template *tlv_t = NULL;
	char tlv_struct[sizeof(struct tlv_template)]={0};

	assert(out);
	assert(tlv_src);

	while(pos_t < size_tlv){
		memcpy(tlv_struct, tlv_src + pos_t, sizeof(struct tlv_template));
		tlv_head = (struct tlv_template *)(tlv_struct);
		tlv_t = (struct tlv_template *)(tlv_src + pos_t);
		tlen = ntohs(tlv_head->length);
		ttag =  ntohs(tlv_head->tag);
		if(ttag == tlv_tag && out_len >= tlen){
			//DEBUG("ttag %d, tlen %d\n", ttag, tlen);
			memcpy(out, tlv_t->value, tlen);
			return tlen;
		}
		pos_t += (tlen + sizeof(struct tlv_template));
	}

	return 0;
}


int main()
{
	return 0;
}

#if 0
struct ip_port{
	uint8_t		comp_type;
		uint32_t	ip;
		uint16_t	port;
		char		str[16];
};

struct tlv_arg_desc test_demo[] = {
	{TLV_COMPRESS_TYPE, DATA_TYPE_U8,  member_info(struct ip_port, comp_type)},
	{TLV_IP_TYPE, DATA_TYPE_U32,  member_info(struct ip_port, ip)},
	{TLV_PORT_TYPE, DATA_TYPE_U16, member_info(struct ip_port, port)},
	{TLV_RAND_TYPE, DATA_TYPE_STRING, member_info(struct ip_port, str)},
	{TLV_MSG_MAX, 0, 0, 0}
};
main()
{
	char out[1000] = {0};
		int i,len;
		struct ip_port ippt = {
			.comp_type = 2,
				.ip = ntohl(inet_addr("192.168.0.1")),
				.port = 9000,
		};
	memcpy(ippt.str, "ABCDEFGHIJKLMNOPQRSTUVWXYZ",16);
		
		struct ip_port iptmp = {0};
		
		printf("\nstruct2tlv:\n");
		
		len = struct2tlv(&out, &ippt, test_demo);
		
		for(i=0; i<len; i++){
			if(i%16 == 0){
				printf("\n");
			}
			printf("%02hhX ", out[i]);
		}
	printf("\ntlv2struct:\n");
		
		tlv2struct(&iptmp, out, len, test_demo);
		printf("ip is %lu, port is %d\n", iptmp.ip, iptmp.port);
		printf("comp_type is %hhu\n", iptmp.comp_type);
		for(i=0; i<16; i++){
			if(i%16 == 0){
				printf("\n");
			}
			printf("%02hhX ", iptmp.str[i]);
		}
}
#endif
