/*
 * A generic kernel FIFO implementation
 *
 * Copyright (C) 2009/2010 Stefani Seibold <stefani@seibold.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "pfifo.h"

/************************************************************************************************************/
/************************************************************************************************************/

#define min(a, b)  (((a) < (b)) ? (a) : (b))

/**
 * @brief  Round the given value up to nearest power of two.
 * @param  [in] x: The number to be converted.
 * @return The power of two.
 */
static unsigned int roundup_pow_of_two(unsigned int x)
{
	unsigned int b = 0;

	int i;
	for(i = 0; i < 32; i++)
		{
			b = 1UL << i;
			if(x <= b)
				break;
		}

	return b;
}

/************************************************************************************************************/
/************************************************************************************************************/

/*
 * internal helper to calculate the unused elements in a fifo
 */
static inline unsigned int pfifo_unused(struct __pfifo *fifo)
{
	return (fifo->mask + 1) - (fifo->in - fifo->out);
}

/* int __pfifo_alloc(struct __pfifo *fifo, unsigned int size, size_t esize, gfp_t gfp_mask) */
int __pfifo_alloc(struct __pfifo *fifo, unsigned int size, size_t esize)
{
	/*
	 * round up to the next power of 2, since our 'let the indices
	 * wrap' technique works only in this case.
	 */
	size = roundup_pow_of_two(size);

	fifo->in = 0;
	fifo->out = 0;
	fifo->esize = esize;

	if (size < 2)
		{
			fifo->data = NULL;
			fifo->mask = 0;
			return -1;
		}

	fifo->data = __pfifo_mem_malloc(size * esize);

	if (!fifo->data)
		{
			fifo->mask = 0;
			return PFIFO_ENOMEM;
		}
	fifo->mask = size - 1;

	return 0;
}

void __pfifo_free(struct __pfifo *fifo)
{
	/* kfree(fifo->data); */
	__pfifo_mem_free(fifo->data);
	fifo->in = 0;
	fifo->out = 0;
	fifo->esize = 0;
	fifo->data = NULL;
	fifo->mask = 0;
}

int __pfifo_init(struct __pfifo *fifo, void *buffer,
                 unsigned int size, size_t esize)
{
	size /= esize;

	size = roundup_pow_of_two(size);

	fifo->in = 0;
	fifo->out = 0;
	fifo->esize = esize;
	fifo->data = buffer;

	if (size < 2)
		{
			fifo->mask = 0;
			return -1;
		}
	fifo->mask = size - 1;

	return 0;
}

static void pfifo_copy_in(struct __pfifo *fifo, const void *src,
                          unsigned int len, unsigned int off)
{
	unsigned int size = fifo->mask + 1;
	unsigned int esize = fifo->esize;
	unsigned int l;

	off &= fifo->mask;
	if (esize != 1)
		{
			off *= esize;
			size *= esize;
			len *= esize;
		}
	l = min(len, size - off);

	memcpy(fifo->data + off, src, l);
	memcpy(fifo->data, src + l, len - l);
	/*
	 * make sure that the data in the fifo is up to date before
	 * incrementing the fifo->in index counter
	 */
	__pfifo_smp_wmb();
}

unsigned int __pfifo_in(struct __pfifo *fifo, const void *buf, unsigned int len)
{
	unsigned int l;

	l = pfifo_unused(fifo);
	if (len > l)
		len = l;

	pfifo_copy_in(fifo, buf, len, fifo->in);
	fifo->in += len;
	return len;
}

static void pfifo_copy_out(struct __pfifo *fifo, void *dst, unsigned int len, unsigned int off)
{
	unsigned int size = fifo->mask + 1;
	unsigned int esize = fifo->esize;
	unsigned int l;

	off &= fifo->mask;
	if (esize != 1)
		{
			off *= esize;
			size *= esize;
			len *= esize;
		}
	l = min(len, size - off);

	memcpy(dst, fifo->data + off, l);
	memcpy(dst + l, fifo->data, len - l);
	/*
	 * make sure that the data is copied before
	 * incrementing the fifo->out index counter
	 */
	__pfifo_smp_wmb();
}

unsigned int __pfifo_out_peek(struct __pfifo *fifo, void *buf, unsigned int len)
{
	unsigned int l;

	l = fifo->in - fifo->out;
	if (len > l)
		len = l;

	pfifo_copy_out(fifo, buf, len, fifo->out);
	return len;
}

unsigned int __pfifo_out(struct __pfifo *fifo,
                         void *buf, unsigned int len)
{
	len = __pfifo_out_peek(fifo, buf, len);
	fifo->out += len;
	return len;
}

unsigned int __pfifo_max_r(unsigned int len, size_t recsize)
{
	unsigned int max = (unsigned int)(1 << (recsize << 3)) - 1;

	if (len > max)
		return max;
	return len;
}

#define	__PFIFO_PEEK(data, out, mask) \
	((data)[(out) & (mask)])
/*
 * __pfifo_peek_n internal helper function for determinate the length of
 * the next record in the fifo
 */
static unsigned int __pfifo_peek_n(struct __pfifo *fifo, size_t recsize)
{
	unsigned int l;
	unsigned int mask = fifo->mask;
	unsigned char *data = fifo->data;

	l = __PFIFO_PEEK(data, fifo->out, mask);

	if (--recsize)
		l |= __PFIFO_PEEK(data, fifo->out + 1, mask) << 8;

	return l;
}

#define	__PFIFO_POKE(data, in, mask, val) \
	( \
	(data)[(in) & (mask)] = (unsigned char)(val) \
	)

/*
 * __pfifo_poke_n internal helper function for storeing the length of
 * the record into the fifo
 */
static void __pfifo_poke_n(struct __pfifo *fifo, unsigned int n, size_t recsize)
{
	unsigned int mask = fifo->mask;
	unsigned char *data = fifo->data;

	__PFIFO_POKE(data, fifo->in, mask, n);

	if (recsize > 1)
		__PFIFO_POKE(data, fifo->in + 1, mask, n >> 8);
}

unsigned int __pfifo_len_r(struct __pfifo *fifo, size_t recsize)
{
	return __pfifo_peek_n(fifo, recsize);
}

unsigned int __pfifo_in_r(struct __pfifo *fifo, const void *buf, unsigned int len, size_t recsize)
{
	if (len + recsize > pfifo_unused(fifo))
		return 0;

	__pfifo_poke_n(fifo, len, recsize);

	pfifo_copy_in(fifo, buf, len, fifo->in + recsize);
	fifo->in += len + recsize;
	return len;
}

static unsigned int pfifo_out_copy_r(struct __pfifo *fifo, void *buf, unsigned int len, size_t recsize, unsigned int *n)
{
	*n = __pfifo_peek_n(fifo, recsize);

	if (len > *n)
		len = *n;

	pfifo_copy_out(fifo, buf, len, fifo->out + recsize);
	return len;
}

unsigned int __pfifo_out_peek_r(struct __pfifo *fifo, void *buf, unsigned int len, size_t recsize)
{
	unsigned int n;

	if (fifo->in == fifo->out)
		return 0;

	return pfifo_out_copy_r(fifo, buf, len, recsize, &n);
}

unsigned int __pfifo_out_r(struct __pfifo *fifo, void *buf, unsigned int len, size_t recsize)
{
	unsigned int n;

	if (fifo->in == fifo->out)
		return 0;

	len = pfifo_out_copy_r(fifo, buf, len, recsize, &n);
	fifo->out += n + recsize;
	return len;
}

void __pfifo_skip_r(struct __pfifo *fifo, size_t recsize)
{
	unsigned int n;

	n = __pfifo_peek_n(fifo, recsize);
	fifo->out += n + recsize;
}

















