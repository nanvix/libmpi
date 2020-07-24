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

#ifndef NANVIX_MPI_DATATYPE_H_
#define NANVIX_MPI_DATATYPE_H_

#include <mputil/object.h>
#include <mpi/datatype_predefined.h>
#include <mpi.h>

/**
 * @brief Struct that defines a mpi_datatype_t.
 */
struct mpi_datatype_t
{
	object_t super; /* Base object class.                */

	uint16_t id;    /* Index in the datatypes array.     */
	int size;       /* Total size in bytes used by data. */
};

typedef struct mpi_datatype_t mpi_datatype_t;

/* Class declaration. */
OBJ_CLASS_DECLARATION(mpi_datatype_t);

/**
 * @brief Gets the size of the given datatype.
 *
 * @param datatype The target datatype.
 *
 * @returns Returns the size of @p datatype.
 */
static inline size_t mpi_datatype_size(const mpi_datatype_t *datatype)
{
	uassert(datatype != NULL);
	return (datatype->size);
}

/**
 * @brief Gets the ID associated to @p datatype.
 *
 * @param datatype Target datatype.
 *
 * @returns Upon receiving a valid datatype, the id associated to @p datatype
 * is returned. A negative error code is returned instead.
 */
static inline int mpi_datatype_id(const mpi_datatype_t *datatype)
{
	uassert(datatype != NULL);
	return (datatype->id);
}

/**
 * @brief Checks if a datatype pointer is valid.
 *
 * @param type Target datatype.
 *
 * @returns Zero if the datatype is not valid, and a non-zero
 * value otherwise.
 */
static inline int mpi_datatype_is_valid(mpi_datatype_t* type)
{
	return ((type != NULL) && (type != MPI_DATATYPE_NULL));
}

/**
 * @brief Compares two different datatypes and evaluates if they match.
 *
 * @param type1 First datatype.
 * @param type2 The datatype to be compared.
 *
 * @returns Zero if the datatypes don't match, and a Non-zero value
 * otherwise.
 */
extern int mpi_datatypes_match(int type1, int type2);

/**
 * @brief Initializes the datatypes submodule.
 *
 * @returns Upon successful completion, zero is returned. A
 * negative error code is returned instead.
 */
extern int mpi_datatype_init(void);

/**
 * @brief Finalizes the datatypes submodule.
 *
 * @returns Upon successful completion, zero is returned. A
 * negative error code is returned instead.
 */
extern int mpi_datatype_finalize(void);

#endif /* NANVIX_MPI_DATATYPE_H_ */
