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

#ifndef NANVIX_POINTER_ARRAY_H_
#define NANVIX_POINTER_ARRAY_H_

#include <nanvix/sys/mutex.h>
#include <mputil/object.h>

/**
 * @brief Struct that defines a dynamic pointer array.
 */
struct pointer_array_t
{
	object_t super;           /* Base object class.                */

	struct nanvix_mutex lock; /* Lock resource.                    */
	int32_t lowest_free;      /* Lowest free index (optimization). */
	int32_t size;             /* List size.                        */
	int32_t max_size;         /* Array max size.                   */
	int32_t block_size;       /* block size for each allocation    */
	void **addr;              /* Array of object pointers.         */
};

typedef struct pointer_array_t pointer_array_t;

/* Class declaration. */
OBJ_CLASS_DECLARATION(pointer_array_t);

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
extern int pointer_array_init(pointer_array_t* array,
                              int initial_allocation,
                              int block_size);

/**
 * @brief Inserts a pointer in the array.
 *
 * @param array Pointer to the respective array.
 * @param ptr   Pointer value to be added.
 *
 * @returns In successful insertion, the index of inserted element is returned.
 * A negative error code is returned instead.
 */
extern int pointer_array_insert(pointer_array_t *array, void *ptr);

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
 */
extern int pointer_array_pop(pointer_array_t *array, int index, void * ptr);

/**
 * @brief Removes an item from the array.
 *
 * @param array Pointer to the respective array.
 * @param index Index of the element to be dropped.
 *
 * @returns Upon successful completion, zero is returned. A negative error code is
 * returned instead.
 */
extern int pointer_array_remove(pointer_array_t *array, int index);

/**
 * @brief Get the value of an element in array.
 *
 * @param array Pointer to array.
 * @param index Index of element to be returned.
 *
 * @returns Upon successful completion, a pointer to the element at @p index is
 * returned. A NULL pointer is returned instead.
 */
static inline void * pointer_array_get_item(pointer_array_t *array, int index)
{
	void *p;

	if (!WITHIN(index, 0, array->max_size))
		return NULL;

	nanvix_mutex_lock(&array->lock);
		p = array->addr[index];
	nanvix_mutex_unlock(&array->lock);

	return p;
}

/**
 * @brief Get the number of elements inside the array.
 *
 * @param array Pointer to array.
 *
 * @returns The number of elements inside the array.
 */
static inline int pointer_array_get_size(pointer_array_t *array)
{
	return array->size;
}

/**
 * @brief Get the max number of elements of the array.
 *
 * @param array Pointer to array.
 *
 * @returns The max number of elements of the array.
 */
static inline int pointer_array_get_max_size(pointer_array_t *array)
{
	return array->max_size;
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
extern int pointer_array_set_item(pointer_array_t *array,
                                  int index, void *value);

/**
 * @brief Clears the pointers array.
 *
 * @param array Pointer to array.
 */
extern void pointer_array_clear(pointer_array_t *array);

#endif /* NANVIX_POINTER_ARRAY_H_ */
