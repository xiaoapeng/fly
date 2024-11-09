/**
 * @file pfifo.h
 * @brief 拷贝来自linux kfifo内核的实现，
 	通过修改改进为便于移植的版本
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2023-11-22
 * 
 * @copyright GPL 3.0
 * 

 */



#ifndef _PFIFO_H
#define _PFIFO_H

#include <stdio.h>
#include <string.h>

/* ######################################################################## */
/* ################################ CONFIG ################################ */
#define 	PFIFO_USE_PLATFORM_LINUX			1
#define 	PFIFO_USE_PLATFORM_ONSYS			0

/* ######################################################################## */
/* ################################ CHECK ################################# */
#if PFIFO_USE_PLATFORM_LINUX && PFIFO_USE_PLATFORM_ONSYS
#error  "Define multiple platforms"
#endif
/* ######################################################################## */


#include <stdatomic.h>
#define __pfifo_smp_wmb() 			atomic_thread_fence(memory_order_release)



#if PFIFO_USE_PLATFORM_LINUX
#include <stdlib.h>


#ifndef __pfifo_mem_malloc
#define __pfifo_mem_malloc(_n) 		malloc(_n)
#endif

#ifndef __pfifo_mem_free
#define __pfifo_mem_free(_p) 		free(_p)
#endif

#else

#ifndef __pfifo_mem_malloc
#define __pfifo_mem_malloc(_n) 		NULL
#endif

#ifndef __pfifo_mem_free
#define __pfifo_mem_free(_p)
#endif

#endif
#define PFIFO_ARRAY_SIZE(arr)		(sizeof(arr) / sizeof((arr)[0]))



struct __pfifo {
	unsigned int			in;
	unsigned int			out;
	unsigned int			mask;
	unsigned int			esize;
	void					*data;
};

#define __STRUCT_PFIFO_COMMON(datatype, recsize, ptrtype) \
	union { \
		struct __pfifo			pfifo; \
		datatype				*type; \
		const datatype			*const_type; \
		char					(*rectype)[recsize]; \
		ptrtype					*ptr; \
		ptrtype const			*ptr_const; \
	}

#define __STRUCT_PFIFO(type, size, recsize, ptrtype) \
{ \
	__STRUCT_PFIFO_COMMON(type, recsize, ptrtype); \
	type		buf[((size < 2) || (size & (size - 1))) ? -1 : size]; \
}

#define STRUCT_PFIFO(type, size) \
	struct __STRUCT_PFIFO(type, size, 0, type)

#define __STRUCT_PFIFO_PTR(type, recsize, ptrtype) \
{ \
	__STRUCT_PFIFO_COMMON(type, recsize, ptrtype); \
	type		buf[0]; \
}

#define STRUCT_PFIFO_PTR(type) \
	struct __STRUCT_PFIFO_PTR(type, 0, type)

/*
 * define compatibility "struct pfifo" for dynamic allocated fifos
 */
struct pfifo __STRUCT_PFIFO_PTR(unsigned char, 0, void);

#define STRUCT_PFIFO_REC_1(size) \
	struct __STRUCT_PFIFO(unsigned char, size, 1, void)

#define STRUCT_PFIFO_REC_2(size) \
	struct __STRUCT_PFIFO(unsigned char, size, 2, void)

/*
 * define pfifo_rec types
 */
struct pfifo_rec_ptr_1 __STRUCT_PFIFO_PTR(unsigned char, 1, void);
struct pfifo_rec_ptr_2 __STRUCT_PFIFO_PTR(unsigned char, 2, void);

/**
 * 返回值的定义
 */
#define PFIFO_EINVAL			(-1)
#define PFIFO_ENOMEM			(-2)


/*
 * helper macro to distinguish between real in place fifo where the fifo
 * array is a part of the structure and the fifo type where the array is
 * outside of the fifo structure.
 */
#define	__is_pfifo_ptr(fifo) \
	(sizeof(*fifo) == sizeof(STRUCT_PFIFO_PTR(typeof(*(fifo)->type))))

/**
 * DECLARE_PFIFO_PTR - macro to declare a fifo pointer object
 * @fifo: name of the declared fifo
 * @type: type of the fifo elements
 */
#define DECLARE_PFIFO_PTR(fifo, type)	STRUCT_PFIFO_PTR(type) fifo

/**
 * DECLARE_PFIFO - macro to declare a fifo object
 * @fifo: name of the declared fifo
 * @type: type of the fifo elements
 * @size: the number of elements in the fifo, this must be a power of 2
 */
#define DECLARE_PFIFO(fifo, type, size)	STRUCT_PFIFO(type, size) fifo

/**
 * INIT_PFIFO - Initialize a fifo declared by DECLARE_PFIFO
 * @fifo: name of the declared fifo datatype
 */
#define INIT_PFIFO(fifo) \
(void)({ \
	typeof(&(fifo)) __tmp = &(fifo); \
	struct __pfifo *__pfifo = &__tmp->pfifo; \
	__pfifo->in = 0; \
	__pfifo->out = 0; \
	__pfifo->mask = __is_pfifo_ptr(__tmp) ? 0 : PFIFO_ARRAY_SIZE(__tmp->buf) - 1;\
	__pfifo->esize = sizeof(*__tmp->buf); \
	__pfifo->data = __is_pfifo_ptr(__tmp) ?  NULL : __tmp->buf; \
})

/**
 * DEFINE_PFIFO - macro to define and initialize a fifo
 * @fifo: name of the declared fifo datatype
 * @type: type of the fifo elements
 * @size: the number of elements in the fifo, this must be a power of 2
 *
 * Note: the macro can be used for global and local fifo data type variables.
 */
#define DEFINE_PFIFO(fifo, type, size) \
	DECLARE_PFIFO(fifo, type, size) = \
	(typeof(fifo)) { \
		.pfifo = { \
			.in	= 0, \
			.out	= 0, \
			.mask	= __is_pfifo_ptr(&(fifo)) ? \
				0 : \
				PFIFO_ARRAY_SIZE((fifo).buf) - 1, \
			.esize	= sizeof(*(fifo).buf), \
			.data	= __is_pfifo_ptr(&(fifo)) ? \
				NULL : \
				(fifo).buf, \
		}, \
	}
	

static inline unsigned int __pfifo_uint_must_check_helper(unsigned int val)
{
	return val;
}

static inline int __pfifo_int_must_check_helper(int val)
{
	return val;
}
 

/**
 * pfifo_initialized - Check if the fifo is initialized
 * @fifo: address of the fifo to check
 *
 * Return %true if fifo is initialized, otherwise %false.
 * Assumes the fifo was 0 before.INIT_PFIFO
 */
#define pfifo_initialized(fifo) ((fifo)->pfifo.mask)

/**
 * pfifo_esize - returns the size of the element managed by the fifo
 * @fifo: address of the fifo to be used
 */
#define pfifo_esize(fifo)	((fifo)->pfifo.esize)

/**
 * pfifo_recsize - returns the size of the record length field
 * @fifo: address of the fifo to be used
 */
#define pfifo_recsize(fifo)	(sizeof(*(fifo)->rectype))

/**
 * pfifo_size - returns the size of the fifo in elements
 * @fifo: address of the fifo to be used
 */
#define pfifo_size(fifo)	((fifo)->pfifo.mask + 1)

/**
 * pfifo_reset - removes the entire fifo content
 * @fifo: address of the fifo to be used
 *
 * Note: usage of pfifo_reset() is dangerous. It should be only called when the
 * fifo is exclusived locked or when it is secured that no other thread is
 * accessing the fifo.
 */
#define pfifo_reset(fifo) \
(void)({ \
	typeof((fifo) + 1) __tmp = (fifo); \
	__tmp->pfifo.in = __tmp->pfifo.out = 0; \
})

/**
 * pfifo_reset_out - skip fifo content
 * @fifo: address of the fifo to be used
 *
 * Note: The usage of pfifo_reset_out() is safe until it will be only called
 * from the reader thread and there is only one concurrent reader. Otherwise
 * it is dangerous and must be handled in the same way as pfifo_reset().
 */
#define pfifo_reset_out(fifo)	\
(void)({ \
	typeof((fifo) + 1) __tmp = (fifo); \
	__tmp->pfifo.out = __tmp->pfifo.in; \
})

/**
 * pfifo_len - returns the number of used elements in the fifo
 * @fifo: address of the fifo to be used
 */
#define pfifo_len(fifo) \
({ \
	typeof((fifo) + 1) __tmpl = (fifo); \
	__tmpl->pfifo.in - __tmpl->pfifo.out; \
})

/**
 * pfifo_is_empty - returns true if the fifo is empty
 * @fifo: address of the fifo to be used
 */
#define	pfifo_is_empty(fifo) \
({ \
	typeof((fifo) + 1) __tmpq = (fifo); \
	__tmpq->pfifo.in == __tmpq->pfifo.out; \
})

/**
 * pfifo_is_full - returns true if the fifo is full
 * @fifo: address of the fifo to be used
 */
#define	pfifo_is_full(fifo) \
({ \
	typeof((fifo) + 1) __tmpq = (fifo); \
	pfifo_len(__tmpq) > __tmpq->pfifo.mask; \
})

/**
 * pfifo_avail - returns the number of unused elements in the fifo
 * @fifo: address of the fifo to be used
 */
#define	pfifo_avail(fifo) \
__pfifo_uint_must_check_helper( \
({ \
	typeof((fifo) + 1) __tmpq = (fifo); \
	const size_t __recsize = sizeof(*__tmpq->rectype); \
	unsigned int __avail = pfifo_size(__tmpq) - pfifo_len(__tmpq); \
	(__recsize) ? ((__avail <= __recsize) ? 0 : \
	__pfifo_max_r(__avail - __recsize, __recsize)) : \
	__avail; \
}) \
)

/**
 * pfifo_skip - skip output data
 * @fifo: address of the fifo to be used
 */
#define	pfifo_skip(fifo) \
(void)({ \
	typeof((fifo) + 1) __tmp = (fifo); \
	const size_t __recsize = sizeof(*__tmp->rectype); \
	struct __pfifo *__pfifo = &__tmp->pfifo; \
	if (__recsize) \
		__pfifo_skip_r(__pfifo, __recsize); \
	else \
		__pfifo->out++; \
})

/**
 * pfifo_peek_len - gets the size of the next fifo record
 * @fifo: address of the fifo to be used
 *
 * This function returns the size of the next fifo record in number of bytes.
 */
#define pfifo_peek_len(fifo) \
__pfifo_uint_must_check_helper( \
({ \
	typeof((fifo) + 1) __tmp = (fifo); \
	const size_t __recsize = sizeof(*__tmp->rectype); \
	struct __pfifo *__pfifo = &__tmp->pfifo; \
	(!__recsize) ? pfifo_len(__tmp) * sizeof(*__tmp->type) : \
	__pfifo_len_r(__pfifo, __recsize); \
}) \
)

/**
 * pfifo_alloc - dynamically allocates a new fifo buffer
 * @fifo: pointer to the fifo
 * @size: the number of elements in the fifo, this must be a power of 2
 * @gfp_mask: get_free_pages mask, passed to kmalloc()
 *
 * This macro dynamically allocates a new fifo buffer.
 *
 * The number of elements will be rounded-up to a power of 2.
 * The fifo will be release with pfifo_free().
 * Return 0 if no error, otherwise an error code.
 */
#define pfifo_alloc(fifo, size) \
__pfifo_int_must_check_helper( \
({ \
	typeof((fifo) + 1) __tmp = (fifo); \
	struct __pfifo *__pfifo = &__tmp->pfifo; \
	__is_pfifo_ptr(__tmp) ? \
	__pfifo_alloc(__pfifo, size, sizeof(*__tmp->type)) : \
	PFIFO_EINVAL; \
}) \
)

/**
 * pfifo_free - frees the fifo
 * @fifo: the fifo to be freed
 */
#define pfifo_free(fifo) \
({ \
	typeof((fifo) + 1) __tmp = (fifo); \
	struct __pfifo *__pfifo = &__tmp->pfifo; \
	if (__is_pfifo_ptr(__tmp)) \
		__pfifo_free(__pfifo); \
})

/**
 * pfifo_init - initialize a fifo using a preallocated buffer
 * @fifo: the fifo to assign the buffer
 * @buffer: the preallocated buffer to be used
 * @size: the size of the internal buffer, this have to be a power of 2
 *
 * This macro initializes a fifo using a preallocated buffer.
 *
 * The number of elements will be rounded-up to a power of 2.
 * Return 0 if no error, otherwise an error code.
 */
#define pfifo_init(fifo, buffer, size) \
({ \
	typeof((fifo) + 1) __tmp = (fifo); \
	struct __pfifo *__pfifo = &__tmp->pfifo; \
	__is_pfifo_ptr(__tmp) ? \
	__pfifo_init(__pfifo, buffer, size, sizeof(*__tmp->type)) : \
	PFIFO_EINVAL; \
})

/**
 * pfifo_put - put data into the fifo
 * @fifo: address of the fifo to be used
 * @val: the data to be added
 *
 * This macro copies the given value into the fifo.
 * It returns 0 if the fifo was full. Otherwise it returns the number
 * processed elements.
 *
 * Note that with only one concurrent reader and one concurrent
 * writer, you don't need extra locking to use these macro.
 */
#define	pfifo_put(fifo, val) \
({ \
	typeof((fifo) + 1) __tmp = (fifo); \
	typeof(*__tmp->const_type) __val = (val); \
	unsigned int __ret; \
	size_t __recsize = sizeof(*__tmp->rectype); \
	struct __pfifo *__pfifo = &__tmp->pfifo; \
	if (__recsize) \
		__ret = __pfifo_in_r(__pfifo, &__val, sizeof(__val), \
			__recsize); \
	else { \
		__ret = !pfifo_is_full(__tmp); \
		if (__ret) { \
			(__is_pfifo_ptr(__tmp) ? \
			((typeof(__tmp->type))__pfifo->data) : \
			(__tmp->buf) \
			)[__pfifo->in & __tmp->pfifo.mask] = \
				*(typeof(__tmp->type))&__val; \
			__pfifo_smp_wmb();	\
			__pfifo->in++; \
		} \
	} \
	__ret; \
})

/**
 * pfifo_get - get data from the fifo
 * @fifo: address of the fifo to be used
 * @val: address where to store the data
 *
 * This macro reads the data from the fifo.
 * It returns 0 if the fifo was empty. Otherwise it returns the number
 * processed elements.
 *
 * Note that with only one concurrent reader and one concurrent
 * writer, you don't need extra locking to use these macro.
 */
#define	pfifo_get(fifo, val) \
__pfifo_uint_must_check_helper( \
({ \
	typeof((fifo) + 1) __tmp = (fifo); \
	typeof(__tmp->ptr) __val = (val); \
	unsigned int __ret; \
	const size_t __recsize = sizeof(*__tmp->rectype); \
	struct __pfifo *__pfifo = &__tmp->pfifo; \
	if (__recsize) \
		__ret = __pfifo_out_r(__pfifo, __val, sizeof(*__val), \
			__recsize); \
	else { \
		__ret = !pfifo_is_empty(__tmp); \
		if (__ret) { \
			*(typeof(__tmp->type))__val = \
				(__is_pfifo_ptr(__tmp) ? \
				((typeof(__tmp->type))__pfifo->data) : \
				(__tmp->buf) \
				)[__pfifo->out & __tmp->pfifo.mask]; \
			__pfifo_smp_wmb();	\
			__pfifo->out++; \
		} \
	} \
	__ret; \
}) \
)

/**
 * pfifo_peek - get data from the fifo without removing
 * @fifo: address of the fifo to be used
 * @val: address where to store the data
 *
 * This reads the data from the fifo without removing it from the fifo.
 * It returns 0 if the fifo was empty. Otherwise it returns the number
 * processed elements.
 *
 * Note that with only one concurrent reader and one concurrent
 * writer, you don't need extra locking to use these macro.
 */
#define	pfifo_peek(fifo, val) \
__pfifo_uint_must_check_helper( \
({ \
	typeof((fifo) + 1) __tmp = (fifo); \
	typeof(__tmp->ptr) __val = (val); \
	unsigned int __ret; \
	const size_t __recsize = sizeof(*__tmp->rectype); \
	struct __pfifo *__pfifo = &__tmp->pfifo; \
	if (__recsize) \
		__ret = __pfifo_out_peek_r(__pfifo, __val, sizeof(*__val), \
			__recsize); \
	else { \
		__ret = !pfifo_is_empty(__tmp); \
		if (__ret) { \
			*(typeof(__tmp->type))__val = \
				(__is_pfifo_ptr(__tmp) ? \
				((typeof(__tmp->type))__pfifo->data) : \
				(__tmp->buf) \
				)[__pfifo->out & __tmp->pfifo.mask]; \
			__pfifo_smp_wmb(); \
		} \
	} \
	__ret; \
}) \
)

/**
 * pfifo_in - put data into the fifo
 * @fifo: address of the fifo to be used
 * @buf: the data to be added
 * @n: number of elements to be added
 *
 * This macro copies the given buffer into the fifo and returns the
 * number of copied elements.
 *
 * Note that with only one concurrent reader and one concurrent
 * writer, you don't need extra locking to use these macro.
 */
#define	pfifo_in(fifo, buf, n) \
({ \
	typeof((fifo) + 1) __tmp = (fifo); \
	typeof(__tmp->ptr_const) __buf = (buf); \
	unsigned long __n = (n); \
	const size_t __recsize = sizeof(*__tmp->rectype); \
	struct __pfifo *__pfifo = &__tmp->pfifo; \
	(__recsize) ?\
	__pfifo_in_r(__pfifo, __buf, __n, __recsize) : \
	__pfifo_in(__pfifo, __buf, __n); \
})


/**
 * pfifo_out - get data from the fifo
 * @fifo: address of the fifo to be used
 * @buf: pointer to the storage buffer
 * @n: max. number of elements to get
 *
 * This macro get some data from the fifo and return the numbers of elements
 * copied.
 *
 * Note that with only one concurrent reader and one concurrent
 * writer, you don't need extra locking to use these macro.
 */
#define	pfifo_out(fifo, buf, n) \
__pfifo_uint_must_check_helper( \
({ \
	typeof((fifo) + 1) __tmp = (fifo); \
	typeof(__tmp->ptr) __buf = (buf); \
	unsigned long __n = (n); \
	const size_t __recsize = sizeof(*__tmp->rectype); \
	struct __pfifo *__pfifo = &__tmp->pfifo; \
	(__recsize) ?\
	__pfifo_out_r(__pfifo, __buf, __n, __recsize) : \
	__pfifo_out(__pfifo, __buf, __n); \
}) \
)


/**
 * pfifo_out_peek - gets some data from the fifo
 * @fifo: address of the fifo to be used
 * @buf: pointer to the storage buffer
 * @n: max. number of elements to get
 *
 * This macro get the data from the fifo and return the numbers of elements
 * copied. The data is not removed from the fifo.
 *
 * Note that with only one concurrent reader and one concurrent
 * writer, you don't need extra locking to use these macro.
 */
#define	pfifo_out_peek(fifo, buf, n) \
__pfifo_uint_must_check_helper( \
({ \
	typeof((fifo) + 1) __tmp = (fifo); \
	typeof(__tmp->ptr) __buf = (buf); \
	unsigned long __n = (n); \
	const size_t __recsize = sizeof(*__tmp->rectype); \
	struct __pfifo *__pfifo = &__tmp->pfifo; \
	(__recsize) ? \
	__pfifo_out_peek_r(__pfifo, __buf, __n, __recsize) : \
	__pfifo_out_peek(__pfifo, __buf, __n); \
}) \
)

extern int __pfifo_alloc(struct __pfifo *fifo, unsigned int size, size_t esize);
extern void __pfifo_free(struct __pfifo *fifo);
extern int __pfifo_init(struct __pfifo *fifo, void *buffer, unsigned int size, size_t esize);
extern unsigned int __pfifo_in(struct __pfifo *fifo, const void *buf, unsigned int len);
extern unsigned int __pfifo_out(struct __pfifo *fifo, void *buf, unsigned int len);
extern unsigned int __pfifo_out_peek(struct __pfifo *fifo, void *buf, unsigned int len);
extern unsigned int __pfifo_in_r(struct __pfifo *fifo, const void *buf, unsigned int len, size_t recsize);
extern unsigned int __pfifo_out_r(struct __pfifo *fifo, void *buf, unsigned int len, size_t recsize);
extern unsigned int __pfifo_len_r(struct __pfifo *fifo, size_t recsize);
extern void __pfifo_skip_r(struct __pfifo *fifo, size_t recsize);
extern unsigned int __pfifo_out_peek_r(struct __pfifo *fifo, void *buf, unsigned int len, size_t recsize);
extern unsigned int __pfifo_max_r(unsigned int len, size_t recsize);

#endif


















