#include "async.test.h"

int main()
{
	const char *str1 = "155 1 2 3\n34 5 5 2 3   \n  \n\n  00001337 01 02 03 "
			  "04 05 06 07 08 09 10\nabc\n";
	const char *str2 = str1;

	struct at1_can_frame frame1;
	struct at2_can_frame frame2;

	struct at1 test1;
	struct at2 test2;
	at1_init(&test1);
	at2_init(&test2);

	printf("TESTING UNIT_ASYNC_TEST1\n");
	while (*str1) {
		int i;

		at1_putc(&test1, *str1++);

		if (!at1_get_can_frame(&test1, &frame1))
			continue;

		printf("w---> %08X, %i: ", frame1.id, frame1.len);
		for (i = 0; i < frame1.len; i++)
			printf("%02X ", frame1.data[i]);
		printf("\n");
	}

	printf("\nTESTING UNIT_ASYNC_TEST2\n");
	while (*str2) {
		int i;

		at2_putc(&test2, *str2++);

		if (!at2_get_can_frame(&test2, &frame2))
			continue;

		printf("w---> %08X, %i: ", frame2.id, frame2.len);
		for (i = 0; i < frame2.len; i++)
			printf("%02X ", frame2.data[i]);
		printf("\n");
	}
	
	while (at3());

	return 0;
}
