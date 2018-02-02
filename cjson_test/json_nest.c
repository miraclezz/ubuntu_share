#include <stdio.h>
#include <stdlib.h>

#include "cJSON.h"

int main(void)
{
	cJSON *root, *rows, *row;
	char *out;
	int i = 0;
	
	char *title[3] = {
		"hello",
		"world",
		"perfect"
	};
	
	char *inf[3] = {
		"............",
		"------------",
		"============"
	};
	
	root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "total", 3);
	
	cJSON_AddItemToObject(root, "rows", rows = cJSON_CreateArray());
	
	for(i=0; i<3; i++){
		cJSON_AddItemToArray(rows, row = cJSON_CreateObject());
		cJSON_AddItemToObject(row, "title", cJSON_CreateString(title[i]));
		cJSON_AddItemToObject(row, "symbol", cJSON_CreateString(inf[i]));
	}
	
	out = cJSON_Print(root);
	cJSON_Delete(root);
	printf("%s\n", out);
	free(out);
	
	return 0;
}