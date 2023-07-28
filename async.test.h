/************************************************
 * UNIT_ASYNC_TEST1
 * Description:
 * 	Testing of UNIT_ASYNC
 ***********************************************/
#include "async.h"
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define AT1_MAX_BUF 64

struct at1_can_frame {
	uint32_t id;
	int8_t	 len;
	uint8_t	 data[8];
};

void at1_can_frame_init(struct at1_can_frame *self) { self->len = -1; }

struct at1 {
	char	buf[AT1_MAX_BUF + 1];
	uint8_t buf_len;

	char	hex_str[9];
	uint8_t hex_str_len;

	struct at1_can_frame can;
};

void at1_init(struct at1 *self)
{
	self->buf_len = 0;
	at1_can_frame_init(&self->can);
}

void at1_skip_until_eol(const char **str)
{
	while (**str && **str != '\n')
		(*str)++;
}

bool at1_tokenize_hex(struct at1 *self, const char **str)
{
	self->hex_str_len = 0;

	/* Skip spaces (new lines not allowed). */
	while (isspace(**str)) {
		if (**str == '\n')
			return false;
		(*str)++;
	}

	/* Read hex digits. */
	while (isxdigit(**str)) {
		if (self->hex_str_len > 8)
			return false;

		self->hex_str[self->hex_str_len++] = **str;

		(*str)++;
	}

	if (self->hex_str_len <= 0)
		return false;

	self->hex_str[self->hex_str_len] = '\0';

	return true;
}

bool at1_parse_line(struct at1 *self, const char **str)
{
	/* If parsed unsuccesfully. */
	if (!at1_tokenize_hex(self, str)) {
		at1_skip_until_eol(str);
		return false;
	}

	/* Set CAN id. */
	self->can.id = strtol(self->hex_str, NULL, 16);

	/* Set CAN data. */
	for (self->can.len = 0; self->can.len < 8; self->can.len++) {
		if (!at1_tokenize_hex(self, str)) {
			at1_skip_until_eol(str);
			return true;
		}

		self->can.data[self->can.len] = strtol(self->hex_str, NULL, 16);
	}

	/* Skip the rest of line. */
	at1_skip_until_eol(str);
	return true;
}

void at1_putc(struct at1 *self, const char c)
{
	if (self->buf_len <= AT1_MAX_BUF)
		self->buf[self->buf_len] = c;

	if (c == '\n') {
		if (self->buf_len <= AT1_MAX_BUF) {
			const char *str		 = self->buf;
			self->buf[self->buf_len] = '\0';
			at1_parse_line(self, &str);
		}
		self->buf_len = 0;
	} else {
		self->buf_len++;
	}
}

bool at1_get_can_frame(struct at1 *self, struct at1_can_frame *frame)
{
	if (self->can.len <= -1)
		return false;

	*frame = self->can;
	at1_can_frame_init(&self->can);
	return true;
}

/************************************************
 * UNIT_ASYNC_TEST2
 * Description:
 * 	Serial to CAN converter testing
 ***********************************************/
#include "async.h"
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

enum at2_e { AT2_RET_YIELD, AT2_RET_GETC, AT2_RET_FAIL, AT2_RET_OK };

#define AT2_GETC                                                               \
	do {                                                                   \
		self->chr = -1;                                                \
		ASYNC_AWAIT(self->chr > -1, AT2_RET_GETC);                     \
	} while (0)

struct at2_can_frame {
	uint32_t id;
	int8_t	 len;
	uint8_t	 data[8];
};

void at2_can_frame_init(struct at2_can_frame *self) { self->len = -1; }

struct at2 {
	async hex_state;
	async prs_state;

	bool  has_frame;
	short chr;

	char	hex_str[9];
	uint8_t hex_str_len;

	struct at2_can_frame can;
};

void at2_init(struct at2 *self)
{
	async_init(&self->hex_state);
	async_init(&self->prs_state);

	self->has_frame = false;

	at2_can_frame_init(&self->can);
}

async at2_skip_until_eol(struct at2 *self)
{
	ASYNC_DISPATCH(self->hex_state);

	while (self->chr && self->chr != '\n')
		AT2_GETC;

	ASYNC_RETURN(AT2_RET_OK);
}

async at2_tokenize_hex(struct at2 *self)
{
	ASYNC_DISPATCH(self->hex_state);

	self->hex_str_len = 0;

	/* Skip spaces (new lines not allowed). */
	while (isspace(self->chr)) {
		if (self->chr == '\n')
			ASYNC_RETURN(AT2_RET_FAIL);
		AT2_GETC;
	}

	/* Read hex digits. */
	while (isxdigit(self->chr)) {
		if (self->hex_str_len >= 8)
			ASYNC_RETURN(AT2_RET_FAIL);

		self->hex_str[self->hex_str_len++] = self->chr;

		AT2_GETC;
	}

	if (self->hex_str_len <= 0)
		ASYNC_RETURN(AT2_RET_FAIL);

	self->hex_str[self->hex_str_len] = '\0';

	ASYNC_RETURN(AT2_RET_OK);
}

async at2_parse_line(struct at2 *self)
{
	ASYNC_DISPATCH(self->prs_state);

	/* If parsed unsuccesfully. */
	ASYNC_CALL(at2_tokenize_hex(self), self->hex_state);
	if (ASYNC_CALL_RESULT == AT2_RET_FAIL) {
		ASYNC_CALL(at2_skip_until_eol(self), self->hex_state);
		ASYNC_RETURN(AT2_RET_FAIL);
	}

	/* Set CAN id. */
	self->can.id = strtol(self->hex_str, NULL, 16);

	/* Set CAN data. */
	for (self->can.len = 0; self->can.len < 8; self->can.len++) {
		ASYNC_CALL(at2_tokenize_hex(self), self->hex_state);
		if (ASYNC_CALL_RESULT == AT2_RET_FAIL) {
			ASYNC_CALL(at2_skip_until_eol(self), self->hex_state);
			ASYNC_RETURN(AT2_RET_OK);
		}

		self->can.data[self->can.len] = strtol(self->hex_str, NULL, 16);
	}

	/* Skip the rest of line. */
	ASYNC_CALL(at2_skip_until_eol(self), self->hex_state);
	ASYNC_RETURN(AT2_RET_OK);
}

void at2_putc(struct at2 *self, const char c)
{
	async ret;
	self->chr = c;
	do {
		ret = at2_parse_line(self);
	} while (ret == AT2_RET_YIELD);
	self->has_frame = (ret == AT2_RET_OK);
}

bool at2_get_can_frame(struct at2 *self, struct at2_can_frame *frame)
{
	if (!self->has_frame)
		return false;

	self->has_frame = false;

	*frame = self->can;
	at2_can_frame_init(&self->can);
	return true;
}

/************************************************
 * UNIT_ASYNC_TEST3
 * Description:
 * 	Most basic test.
 ***********************************************/
#include "async.h"
#include <stdio.h>

async at3()
{
	/* Every asynchronous macro destroy local variables.
	 * So we need to keep them non-local, or pass as *self parameter. */
	static async state;
	static int i;
	
	/* Dispatcher will bring us back to the last yielded state. */
	ASYNC_DISPATCH(state);
	
	for (i = 0; i < 5; i++) {
		printf("Value of i variable: %i\n", i);

		/* Return from function with return value of 1.
		 * When function gets called again, dispatcher will
		 * continue executing code after this yield.*/
		ASYNC_YIELD(1);
	}
	
	/* Return 0, so we know function has ended. */
	ASYNC_RETURN(0);
}
