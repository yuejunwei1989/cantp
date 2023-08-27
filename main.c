#include <stdio.h>
#include "config.h"

int main()
{
	printf("Version major: %d\n", cantp_VERSION_MAJOR);
	printf("Version minor: %d\n", cantp_VERSION_MINOR);
	printf("Version minor: %d\n", cantp_VERSION_PATCH);

	return 0;
}
