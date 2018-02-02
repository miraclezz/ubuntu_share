#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"

void create_single_number(void){
	cJSON *root;
	char *out;
	int value = 24;
	
	root = cJSON_CreateObject();
	
	cJSON_AddItemToObject(root, "value", cJSON_CreateNumber(value));
	
	out = cJSON_Print(root);
	cJSON_Delete(root);
	printf("%s\n",out);
	free(out);
}

int main(void)
{
	create_single_number();
	return 0;
}