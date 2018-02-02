#include <stdio.h>
#include <stdlib.h>

#include "cJSON.h"

int main(void)
{
	cJSON *root;
	char *out;
	char *name = "Frank";
	
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "name", cJSON_CreateString(name));
	
	out = cJSON_Print(root);
	cJSON_Delete(root);
	printf("%s\n", out);
	free(out);
}