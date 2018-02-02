#include <stdio.h>
#include <stdlib.h>

#include "cJSON.h"

int main(void)
{
	cJSON *root;
	char *out;
	int list[4] = {5, 6, 7, 8};
	
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "lists", cJSON_CreateIntArray(list, 2));
	
	out = cJSON_Print(root);
	cJSON_Delete(root);
	printf("%s\n", out);
	free(out);
	
	return 0;
}