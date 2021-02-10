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

#include <nanvix/hlib.h>
#include <posix/errno.h>
#include <mputil/ptr_array.h>

#define POINTER_ARRAY_BITMAP_SIZE (sizeof(uint64_t) * 8)

PRIVATE void pointer_array_construct(pointer_array_t *);
PRIVATE void pointer_array_destruct(pointer_array_t *);
PRIVATE int grow_table(pointer_array_t *array, int at_least);

OBJ_CLASS_INSTANCE(pointer_array_t, &pointer_array_construct,
	               &pointer_array_destruct,
	               sizeof(pointer_array_t));

/**
 * @brief Pointer_array constructor.
 */
PRIVATE void pointer_array_construct(pointer_array_t *array)
{
	uassert(array != NULL);

	nanvix_mutex_init(&array->lock, NULL);
    array->lowest_free = 0;
    array->size = 0;
    array->max_size = 0;
    array->used_bits = NULL;
    array->addr = NULL;
}

/**
 * @brief Pointer_array destructor.
 */
PRIVATE void pointer_array_destruct(pointer_array_t *array)
{
    if(array->used_bits != NULL) {
        ufree(array->used_bits);
        array->used_bits = NULL;
    }

    if(array->addr != NULL) {
        ufree(array->addr);
        array->addr = NULL;
    }

    array->size = 0;
}

/**
 * @brief Sets a bit to ONE.
 *
 * @param bits  Pointer to the bits array.
 * @param index Index of the bit to be set.
 */
#define SET_BIT(bits, index)                           \
    (bits[(int)(index/POINTER_ARRAY_BITMAP_SIZE)]) |= \
    (1 << index % POINTER_ARRAY_BITMAP_SIZE)

/**
 * @brief Sets a bit to ZERO.
 *
 * @param bits  Pointer to the bits variable.
 * @param index Index of the bit to be unset.
 */
#define UNSET_BIT(bits, index)                         \
    (bits[(int)(index/POINTER_ARRAY_BITMAP_SIZE)]) &= \
    ~(1 << index % POINTER_ARRAY_BITMAP_SIZE)

/**
 * @brief Finds the first zero in an uint64_t.
 *
 * @param bits  Pointer to bits array.
 * @param start First index of search (included).
 *
 * @returns The index of the first zero found.
 */
static inline int pointer_array_find_first_free(pointer_array_t * array, int start)
{
	int index;
	int limit;
	int pos;
	uint64_t check;

	if (array->size == array->max_size)
		return (array->size);

	index = (start / POINTER_ARRAY_BITMAP_SIZE);
	limit = ((array->max_size - 1) / POINTER_ARRAY_BITMAP_SIZE);

	for (; (array->used_bits[index] == 0xFFFFFFFFFFFFFFFFu) && (index <= limit); ++index);

	check = array->used_bits[index];
	pos = 0;

	if (0x00000000FFFFFFFFu == (check & 0x00000000FFFFFFFFu))
	{
		check >>= 32; pos += 32;
	}

	if (0x000000000000FFFFu == (check & 0x000000000000FFFFu))
	{
		check >>= 16; pos += 16;
	}

	if (0x00000000000000FFu == (check & 0x00000000000000FFu))
	{
		check >>= 8; pos += 8;
	}

	if (0x000000000000000Fu == (check & 0x000000000000000Fu))
	{
		check >>= 4; pos += 4;
	}

	if (0x0000000000000003u == (check & 0x0000000000000003u))
	{
		check >>= 2; pos += 2;
	}

	if (0x0000000000000001u == (check & 0x0000000000000001u))
	{
		pos += 1;
	}

	return ((index * 8 * sizeof(uint64_t)) + pos);
}

/**
 * @brief Initializes the pointers array with initial values.
 *
 * @param array              Pointer to a pointer array.
 * @param initial_allocation Number of elements initially allocated.
 * @param block_size         Size for all subsequent grows.
 *
 * @returns Zero if initialization was succesfull. A negative
 * error code is returned instead.
 */
PUBLIC int pointer_array_init(pointer_array_t* array,
                              int initial_allocation,
                              int block_size)
{
	/* Bad array pointer. */
	if (array == NULL)
		return (-EINVAL);

	/* Bad initial allocation. */
	if (initial_allocation <= 0)
		return (-EINVAL);

	/* Bad block size. */
	if (block_size <= 0)
		return (-EINVAL);

	/* Allocates the addr array. */
	array->addr = (void **) umalloc(initial_allocation * sizeof(void *));
	if (array->addr == NULL)
		return (-ENOMEM);

	/* Allocates the used_bits variables. */
	array->used_bits = (uint64_t *) ucalloc(
										TRUNCATE(
											initial_allocation,
											POINTER_ARRAY_BITMAP_SIZE
										) / POINTER_ARRAY_BITMAP_SIZE,
										sizeof(uint64_t)
									);

	if (array->used_bits == NULL)
	{
		ufree(array->addr);
		array->addr = NULL;
		return (-ENOMEM);
	}

	for (int i = 0; i < initial_allocation; ++i)
		array->addr[i] = NULL;

	/* Initializes the other attributes. */
	array->lowest_free = 0;
	array->size        = 0;
	array->max_size    = initial_allocation;
	array->block_size  = block_size;

	return (0);
}

/**
 * @brief Inserts a pointer in the array.
 *
 * @param array Pointer to the respective array.
 * @param ptr   Pointer value to be added.
 *
 * @returns In successful insertion, the index of inserted element is returned.
 * A negative error code is returned instead.
 */
PUBLIC int pointer_array_insert(pointer_array_t *array, void *ptr)
{
	int ret; 

	/* Bad array pointer. */
	if (array == NULL)
		return (-EINVAL);

	/* Bad pointer. */
	if (ptr == NULL)
		return (-EINVAL);

	/* Asserts that the array was already initialized. */
	uassert(array->max_size > 0);

	/* Locks the array. */
	nanvix_mutex_lock(&array->lock);

		/* Verifies if there is a need for more space. */
		if (array->size == array->max_size)
		{
			/* Cannot allocate more space. */
			if ((ret = grow_table(array, array->size + 1)) < 0)
			{
				nanvix_mutex_unlock(&array->lock);
				return (ret);
			}
		}

		/* Adds another pointer to the addr table. */
		ret = array->lowest_free;
		uassert(array->addr[ret] == NULL);
		array->addr[ret] = ptr;
		array->size++;
		SET_BIT(array->used_bits, ret);
	
		array->lowest_free = pointer_array_find_first_free(array, ret + 1);

	/* Unlocks the array. */
	nanvix_mutex_unlock(&array->lock);

	return (ret);
}

/**
 * @brief Grows the array table by the specified block size.
 *
 * @param array Array pointer to grow table.
 *
 * @returns Upon successful completion, zero is returned.
 * A negative error code is returned instead.
 */
PRIVATE int grow_table(pointer_array_t *array, int at_least)
{
	int grow;
	int new_size;
	int old_int;
	int new_int;
	void *p;

	uassert(array != NULL);

	/* Calculates new size. */
	grow = at_least - array->size;

	if (grow <= array->block_size)
		new_size = array->max_size + array->block_size;
	else
		new_size = array->max_size + TRUNCATE(at_least, array->block_size);

	/* Reallocates the addr table. */
	p = (void **) urealloc(array->addr, new_size * sizeof(void *));
	if (p == NULL)
		return (-ENOMEM);

	array->addr = (void **) p;

	for (int i = array->max_size; i < new_size; ++i)
		array->addr[i] = NULL;

	/* Verifies the necessity of reallocating the used_bits array. */
	new_int = TRUNCATE(new_size, POINTER_ARRAY_BITMAP_SIZE) / POINTER_ARRAY_BITMAP_SIZE;
	old_int = TRUNCATE(array->max_size, POINTER_ARRAY_BITMAP_SIZE) / POINTER_ARRAY_BITMAP_SIZE;
	
	if (new_int != old_int)
	{
		p = (uint64_t *) urealloc(array->used_bits, new_int * sizeof(uint64_t));
		if (p == NULL)
			return (-ENOMEM);

		array->used_bits = (uint64_t *) p;

		for (int i = old_int; i < new_int; ++i)
			array->used_bits[i] = 0;
	}

	/* Updates array size. */
	array->max_size = new_size;

	return (0);
}

/**
 * @brief Sets an element in the array.
 *
 * @param array Pointer to array.
 * @param index Index of the element to be tested.
 * @param value New value to be set at @p index.
 *
 * @returns Upon successful completion, zero is returned. A negative
 * error code is returned instead.
 */
PUBLIC int pointer_array_set_item(pointer_array_t *array, int index, void *value)
{
	int ret;

	/* Bad array pointer. */
	if (array == NULL)
		return (-EINVAL);

	/* Bad value. */
	if (value == NULL)
		return (-EINVAL);

	/* Bad index. */
	if (index < 0)
		return (-EINVAL);

	/* Locks the array. */
	nanvix_mutex_lock(&array->lock);

		/* Need to grow table? */
		if (index >= array->max_size)
		{
			if ((ret = grow_table(array, index)) < 0)
			{
				nanvix_mutex_unlock(&array->lock);
				return (ret);
			}
		}

		if (array->addr[index] == NULL)
		{
			array->size++;
			SET_BIT(array->used_bits, index);
		}

		array->addr[index] = value;

		/* Checks necessity of update lowest_free. */
		if (index == array->lowest_free)
			array->lowest_free = pointer_array_find_first_free(array, index + 1);

	/* Unlocks the array. */
	nanvix_mutex_unlock(&array->lock);

	return (0);
}

/**
 * @brief Removes and return an item from the array.
 *
 * @param array Pointer to the respective array.
 * @param index Index of the element to be popped.
 * @param ptr   Pointer where the popped object will be accessible.
 *
 * @returns Upon successful completion, zero is returned and the popped object
 * is accessible in @p ptr. A negative error code is returned instead, with NULL
 * in @p ptr.
 *
 * @note This function needs a valid pointer to store the item. If don't desire to
 * store the element returned use pointer_array_remove() instead.
 */
PUBLIC int pointer_array_pop(pointer_array_t *array, int index, void * ptr)
{
	/* Bad array pointer. */
	if (array == NULL)
		return (-EINVAL);

	/* Bad index. */
	if (!WITHIN(index, 0, array->max_size))
		return (-EINVAL);

	if (ptr == NULL)
		return (-EINVAL);

	/* Locks the array. */
	nanvix_mutex_lock(&array->lock);

		ptr = array->addr[index];

		/* Addr is already free. */
		if (ptr == NULL)
			goto unlock;

		/* Releases the addr. */
		array->addr[index] = NULL;
		array->size--;
		UNSET_BIT(array->used_bits, index);

		if (index < array->lowest_free)
			array->lowest_free = index;

unlock:
	/*Unlocks the array. */
	nanvix_mutex_unlock(&array->lock);

	return (0);
}

/**
 * @brief Removes an item from the array.
 *
 * @param array Pointer to the respective array.
 * @param index Index of the element to be dropped.
 *
 * @returns Upon successful completion, zero is returned. A negative error code is
 * returned instead.
 */
PUBLIC int pointer_array_remove(pointer_array_t *array, int index)
{
	/* Bad array pointer. */
	if (array == NULL)
		return (-EINVAL);

	/* Bad index. */
	if (!WITHIN(index, 0, array->max_size))
		return (-EINVAL);

	/* Locks the array. */
	nanvix_mutex_lock(&array->lock);

		/* Addr is already free. */
		if (array->addr[index] == NULL)
			goto unlock;

		/* Releases the addr. */
		array->addr[index] = NULL;
		array->size--;
		UNSET_BIT(array->used_bits, index);

		if (index < array->lowest_free)
			array->lowest_free = index;

unlock:
	/* Unlocks the array. */
	nanvix_mutex_unlock(&array->lock);

	return (0);
}

/**
 * @brief Clears the pointers array.
 *
 * @param array Pointer to array.
 */
PUBLIC void pointer_array_clear(pointer_array_t *array)
{
	int limit;

	/* Checks if array already empty. */
	if(array->size == 0)
		return;

	limit = ((array->max_size - 1) / POINTER_ARRAY_BITMAP_SIZE);

	nanvix_mutex_lock(&array->lock);
		array->lowest_free = 0;
		array->size = 0;

		for (int i = 0; i < array->max_size; i++)
			array->addr[i] = NULL;

		for (int i = 0; i <= limit; ++i)
			array->used_bits[i] = 0;

	nanvix_mutex_unlock(&array->lock);
}
