#include "circ_buf.h"
#include <string.h>

// Lightweight circular buffer implementation inspired by kfifo.
// The buffer capacity is rounded down to the nearest power-of-two and stored
// in `mask` for efficient modulo operations. This file intentionally keeps
// a small API surface for performance on the STM32 USB and stepper codepaths.

// Round down to nearest power-of-two
static unsigned int rounddown_pow_of_two(unsigned int n) {
	n |= n >> 1; n |= n >> 2; n |= n >> 4; n |= n >> 8; n |= n >> 16;
	return (n + 1) >> 1;
}

void circ_buf_init(circ_buf_t *self, void *buf, unsigned int bufsize, unsigned int item_size) {
	self->data = (unsigned char*)buf;
	self->esize = item_size;
	self->mask = rounddown_pow_of_two(bufsize) - 1;
	self->in = self->out = 0;
}

// item count in buf (uint8_t units)
unsigned int circ_buf_size_uint8(circ_buf_t *self) {
	return self->in - self->out;
}

// max item count in buf (uint8_t units)
unsigned int circ_buf_capacity_uint8(circ_buf_t *self) {
	return self->mask + 1;
}

// avail item count (uint8_t units)
unsigned int circ_buf_avail_uint8(circ_buf_t *self) {
	return circ_buf_capacity_uint8(self) - circ_buf_size_uint8(self);
}

// item count in buf (item units)
unsigned int circ_buf_size(circ_buf_t *self) {
	return circ_buf_size_uint8(self) / self->esize;
}

// max item count in buf (item units)
unsigned int circ_buf_capacity(circ_buf_t *self) {
	return circ_buf_capacity_uint8(self) / self->esize;
}

// avail item count (item units)
unsigned int circ_buf_avail(circ_buf_t *self) {
	return circ_buf_avail_uint8(self) / self->esize;
}

int circ_buf_full(circ_buf_t *self) {
	return circ_buf_size_uint8(self) > self->mask;
}

int circ_buf_empty(circ_buf_t *self) {
	return self->in == self->out;
}

#define mymin(a, b) ((a) < (b) ? (a) : (b))

static void circ_buf_copy_in(circ_buf_t *self, const void *src, unsigned int len, unsigned int off) {
	unsigned int size = self->mask + 1;
	unsigned int l;

	off &= self->mask;
	l = mymin(len, size - off);

	memcpy(self->data + off, src, l);
	memcpy(self->data, (const unsigned char*)src + l, len - l);
}

unsigned int circ_buf_push(circ_buf_t *self, const void *buf) {
	return circ_buf_push_many(self, buf, 1);
}

unsigned int circ_buf_push_many(circ_buf_t *self, const void *buf, unsigned int item_count) {
	unsigned int avail = circ_buf_avail_uint8(self);
	item_count = item_count * self->esize;
	if (item_count > avail)
		item_count = avail;

	circ_buf_copy_in(self, buf, item_count, self->in);

	self->in += item_count;
	return item_count / self->esize;
}

unsigned int circ_buf_push_many_uint8(circ_buf_t *self, const void *buf, unsigned int item_count) {
	unsigned int avail = circ_buf_avail_uint8(self);
	if (item_count > avail)
		item_count = avail;

	circ_buf_copy_in(self, buf, item_count, self->in);

	self->in += item_count;
	return item_count;
}

static void circ_buf_copy_out(circ_buf_t *self, void *dst, unsigned int len, unsigned int off) {
	unsigned int size = self->mask + 1;
	unsigned int l;

	off &= self->mask;

	l = mymin(len, size - off);

	memcpy(dst, self->data + off, l);
	memcpy((unsigned char*)dst + l, self->data, len - l);
}

unsigned int circ_buf_out_peek(circ_buf_t *self, void *buf, unsigned int len) {
	unsigned int l;
	l = self->in - self->out;
	if (len > l)
		len = l;
	circ_buf_copy_out(self, buf, len, self->out);
	return len;
}

unsigned int circ_buf_pop(circ_buf_t *self, void *buf) {
	return circ_buf_pop_many(self, buf, 1);
}

unsigned int circ_buf_pop_many(circ_buf_t *self, void *buf, unsigned int item_count) {
	item_count = circ_buf_out_peek(self, buf, item_count * self->esize);
	self->out += item_count;
	return item_count / self->esize;
}
