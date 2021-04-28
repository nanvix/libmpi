/*
 * MIT License
 *
 * Copyright(c) 2011-2020 The Maintainers of Nanvix
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define __NEED_RESOURCE

#include <nanvix/hal/resource.h>
#include <nanvix/sys/condvar.h>
#include <nanvix/sys/mutex.h>
#include <nanvix/hlib.h>
#include <nanvix/ulib.h>
#include <mputil/buffer_slot.h>

/**
 * @brief Controls the debugging printfs.
 */
#define DEBUG 0

/**
 * @brief Maximum size of request queue.
 */
#define BUFFER_SLOTS_NR 16

/**
 * @brief Request queue node structure.
 */
PRIVATE struct buffer_slot
{
	/*
	 * XXX: Don't Touch! This Must Come First!
	 */
	struct resource resource;        /**< Generic resource information. */

	const void *buffer;
	size_t max_size;
	struct nanvix_cond_var condvar;
	struct nanvix_mutex lock;
} buffer_slots[BUFFER_SLOTS_NR];

/**
 * @brief Pool of buffer slots.
 */
PRIVATE const struct resource_pool pool_slots = {
	buffer_slots, BUFFER_SLOTS_NR, sizeof(struct buffer_slot)
};

/**
 * @todo Add a detailed description.
 */
PUBLIC int buffer_slot_reserve(const void *buffer, size_t max_size)
{
	int id;

	/* Valid buffer? */
	if (buffer == NULL)
		return (-EINVAL);

	/* Allocates a buffer resource. */
	if ((id = resource_alloc(&pool_slots)) < 0)
		return (id);

	/* Locks the specified slot to be released on condvar_wait. */
	uassert(nanvix_mutex_lock(&buffer_slots[id].lock) == 0);

	/* Sets slot buffer to point to user buffer. */
	buffer_slots[id].buffer   = buffer;
	buffer_slots[id].max_size = max_size;

	return (id);
}

/**
 * @todo Add a detailed description.
 */
PUBLIC int buffer_slot_release(int bufferid)
{
	/* Valid bufferid? */
	if (!WITHIN(bufferid, 0, BUFFER_SLOTS_NR))
		return (-EINVAL);

	/* Resets slot buffer pointer. */
	buffer_slots[bufferid].buffer   = NULL;
	buffer_slots[bufferid].max_size = 0;

	/* Unlocks slot. */
	uassert(nanvix_mutex_unlock(&buffer_slots[bufferid].lock) == 0);

	/* Frees slot resource. */
	resource_free(&pool_slots, bufferid);

	return (0);
}

/**
 * @todo Add a detailed description.
 */
PUBLIC int buffer_slot_wait(int bufferid)
{
	/* Valid bufferid? */
	if (!WITHIN(bufferid, 0, BUFFER_SLOTS_NR))
		return (-EINVAL);

	/* Waits on condvar until a read operation finalizes. */
	uassert(nanvix_cond_wait(&buffer_slots[bufferid].condvar, &buffer_slots[bufferid].lock) == 0);

	return (0);
}

/**
 * @todo Add a detailed description.
 */
PUBLIC int buffer_slot_read(int bufferid, void *buffer, size_t size)
{
	/* Valid bufferid? */
	if (!WITHIN(bufferid, 0, BUFFER_SLOTS_NR))
		return (-EINVAL);

	/* Valid buffer? */
	if (buffer == NULL)
		return (-EINVAL);

	/* Possibility of buffer overflow? */
	if (size > buffer_slots[bufferid].max_size)
		return (-EINVAL);

	/* Copies buffer slot data to user buffer. */
	umemcpy(buffer, buffer_slots[bufferid].buffer, size);

	/* Unlocks the buffer slot owner. */
	uassert(nanvix_cond_signal(&buffer_slots[bufferid].condvar) == 0);

	return (0);
}

/**
 * @todo Add a detailed description.
 */
PUBLIC int buffer_slots_init(void)
{
	/* Initialize buffer slots. */
	for (int i = 0; i < BUFFER_SLOTS_NR; ++i)
	{
		buffer_slots[i].buffer   = NULL;
		buffer_slots[i].max_size = 0;

		/* Initialize slot mutex. */
		uassert(nanvix_mutex_init(&buffer_slots[i].lock, NULL) == 0);

		/* Initialize slot condvar. */
		uassert(nanvix_cond_init(&buffer_slots[i].condvar) == 0);
	}

	return (0);
}

/**
 * @todo Add a detailed description.
 */
PUBLIC int buffer_slots_finalize(void)
{
	/* Sets buffer slots buffers to null. */
	for (int i = 0; i < BUFFER_SLOTS_NR; ++i)
		buffer_slots[i].buffer = NULL;

	return (0);
}
