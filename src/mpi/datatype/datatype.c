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
#include <nanvix/ulib.h>
#include <mpi/datatype.h>

PRIVATE void mpi_datatype_construct(mpi_datatype_t *);
PRIVATE void mpi_datatype_destruct(mpi_datatype_t *);

OBJ_CLASS_INSTANCE(mpi_datatype_t, &mpi_datatype_construct,
                   &mpi_datatype_destruct, sizeof(mpi_datatype_t));

/**
 * @brief Predefined datatypes.
 */
mpi_datatype_t _mpi_datatype_char                = MPI_DATATYPE_INITIALIZER_CHAR;
mpi_datatype_t _mpi_datatype_short               = MPI_DATATYPE_INITIALIZER_SHORT;
mpi_datatype_t _mpi_datatype_int                 = MPI_DATATYPE_INITIALIZER_INT;
mpi_datatype_t _mpi_datatype_long                = MPI_DATATYPE_INITIALIZER_LONG;
mpi_datatype_t _mpi_datatype_long_long           = MPI_DATATYPE_INITIALIZER_LONG_LONG;
mpi_datatype_t _mpi_datatype_signed_char         = MPI_DATATYPE_INITIALIZER_SIGNED_CHAR;
mpi_datatype_t _mpi_datatype_unsigned_char       = MPI_DATATYPE_INITIALIZER_UNSIGNED_CHAR;
mpi_datatype_t _mpi_datatype_unsigned_short      = MPI_DATATYPE_INITIALIZER_UNSIGNED_SHORT;
mpi_datatype_t _mpi_datatype_unsigned            = MPI_DATATYPE_INITIALIZER_UNSIGNED;
mpi_datatype_t _mpi_datatype_unsigned_long       = MPI_DATATYPE_INITIALIZER_UNSIGNED_LONG;
mpi_datatype_t _mpi_datatype_unsigned_long_long  = MPI_DATATYPE_INITIALIZER_UNSIGNED_LONG_LONG;
mpi_datatype_t _mpi_datatype_float               = MPI_DATATYPE_INITIALIZER_FLOAT;
mpi_datatype_t _mpi_datatype_double              = MPI_DATATYPE_INITIALIZER_DOUBLE;
mpi_datatype_t _mpi_datatype_long_double         = MPI_DATATYPE_INITIALIZER_LONG_DOUBLE;
mpi_datatype_t _mpi_datatype_wchar               = MPI_DATATYPE_INITIALIZER_WCHAR;
mpi_datatype_t _mpi_datatype_cbool               = MPI_DATATYPE_INITIALIZER_C_BOOL;
mpi_datatype_t _mpi_datatype_int8                = MPI_DATATYPE_INITIALIZER_INT8;
mpi_datatype_t _mpi_datatype_int16               = MPI_DATATYPE_INITIALIZER_INT16;
mpi_datatype_t _mpi_datatype_int32               = MPI_DATATYPE_INITIALIZER_INT32;
mpi_datatype_t _mpi_datatype_int64               = MPI_DATATYPE_INITIALIZER_INT64;
mpi_datatype_t _mpi_datatype_uint8               = MPI_DATATYPE_INITIALIZER_UINT8;
mpi_datatype_t _mpi_datatype_uint16              = MPI_DATATYPE_INITIALIZER_UINT16;
mpi_datatype_t _mpi_datatype_uint32              = MPI_DATATYPE_INITIALIZER_UINT32;
mpi_datatype_t _mpi_datatype_uint64              = MPI_DATATYPE_INITIALIZER_UINT64;
mpi_datatype_t _mpi_datatype_ccomplex            = MPI_DATATYPE_INITIALIZER_C_COMPLEX;
mpi_datatype_t _mpi_datatype_double_complex      = MPI_DATATYPE_INITIALIZER_C_DOUBLE_COMPLEX;
mpi_datatype_t _mpi_datatype_long_double_complex = MPI_DATATYPE_INITIALIZER_C_LONG_DOUBLE_COMPLEX;
mpi_datatype_t _mpi_datatype_byte                = MPI_DATATYPE_INITIALIZER_BYTE;
mpi_datatype_t _mpi_datatype_packed              = MPI_DATATYPE_INITIALIZER_PACKED;
mpi_datatype_t _mpi_datatype_aint                = MPI_DATATYPE_INITIALIZER_AINT;
mpi_datatype_t _mpi_datatype_offset              = MPI_DATATYPE_INITIALIZER_OFFSET;
mpi_datatype_t _mpi_datatype_count               = MPI_DATATYPE_INITIALIZER_COUNT;

/**
 * @brief Null datatype.
 */
mpi_datatype_t _mpi_datatype_null;

/**
 * @brief Predefined datatypes table.
 */
const mpi_datatype_t *_mpi_predefined_datatypes[MPI_DATATYPE_MAX_PREDEFINED] = {
	[MPI_DATATYPE_CHAR]                  = MPI_CHAR,
	[MPI_DATATYPE_SHORT]                 = MPI_SHORT,
	[MPI_DATATYPE_INT]                   = MPI_INT,
	[MPI_DATATYPE_LONG]                  = MPI_LONG,
	[MPI_DATATYPE_LONG_LONG_INT]         = MPI_LONG_LONG_INT,
	[MPI_DATATYPE_LONG_LONG]             = MPI_LONG_LONG,
	[MPI_DATATYPE_SIGNED_CHAR]           = MPI_SIGNED_CHAR,
	[MPI_DATATYPE_UNSIGNED_CHAR]         = MPI_UNSIGNED_CHAR,
	[MPI_DATATYPE_UNSIGNED_SHORT]        = MPI_UNSIGNED_SHORT,
	[MPI_DATATYPE_UNSIGNED]              = MPI_UNSIGNED,
	[MPI_DATATYPE_UNSIGNED_LONG]         = MPI_UNSIGNED_LONG,
	[MPI_DATATYPE_UNSIGNED_LONG_LONG]    = MPI_UNSIGNED_LONG_LONG,
	[MPI_DATATYPE_FLOAT]                 = MPI_FLOAT,
	[MPI_DATATYPE_DOUBLE]                = MPI_DOUBLE,
	[MPI_DATATYPE_LONG_DOUBLE]           = MPI_LONG_DOUBLE,
	[MPI_DATATYPE_WCHAR]                 = MPI_WCHAR,
	[MPI_DATATYPE_C_BOOL]                = MPI_C_BOOL,
	[MPI_DATATYPE_INT8_T]                = MPI_INT8_T,
	[MPI_DATATYPE_INT16_T]               = MPI_INT16_T,
	[MPI_DATATYPE_INT32_T]               = MPI_INT32_T,
	[MPI_DATATYPE_INT64_T]               = MPI_INT64_T,
	[MPI_DATATYPE_UINT8_T]               = MPI_UINT8_T,
	[MPI_DATATYPE_UINT16_T]              = MPI_UINT16_T,
	[MPI_DATATYPE_UINT32_T]              = MPI_UINT32_T,
	[MPI_DATATYPE_UINT64_T]              = MPI_UINT64_T,
	[MPI_DATATYPE_C_COMPLEX]             = MPI_C_COMPLEX,
	[MPI_DATATYPE_C_FLOAT_COMPLEX]       = MPI_C_FLOAT_COMPLEX,
	[MPI_DATATYPE_C_DOUBLE_COMPLEX]      = MPI_C_DOUBLE_COMPLEX,
	[MPI_DATATYPE_C_LONG_DOUBLE_COMPLEX] = MPI_C_LONG_DOUBLE_COMPLEX,
	[MPI_DATATYPE_BYTE]                  = MPI_BYTE,
	[MPI_DATATYPE_PACKED]                = MPI_PACKED,
	[MPI_DATATYPE_AINT]                  = MPI_AINT,
	[MPI_DATATYPE_OFFSET]                = MPI_OFFSET,
	[MPI_DATATYPE_COUNT]                 = MPI_COUNT
};

/**
 * @brief Datatype constructor.
 */
PRIVATE void mpi_datatype_construct(mpi_datatype_t * datatype)
{
	uassert(datatype != NULL);

	datatype->id   = MPI_UNDEFINED;
	datatype->size = MPI_UNDEFINED;
}

/**
 * @brief Datatype destructor.
 */
PRIVATE void mpi_datatype_destruct(mpi_datatype_t * datatype)
{
	uassert(datatype != NULL);
}

/**
 * @brief Provide a detailed description.
 *
 * @note This dummy implementation assumes that only the
 * predefined datatypes are available to be used.
 */
PUBLIC int mpi_datatypes_match(int type1, int type2)
{
	uassert(WITHIN(type1, 0, MPI_DATATYPE_MAX_PREDEFINED));
	uassert(WITHIN(type2, 0, MPI_DATATYPE_MAX_PREDEFINED));

	/* Same datatype. */
	if (type1 == type2)
		return (1);

	/* One of they is MPI_BYTE. */
	if ((type1 == MPI_DATATYPE_BYTE) || (type2 == MPI_DATATYPE_BYTE))
		return (1);

	return (0);
}

/**
 * @see mpi_datatype_init() at datatype.h.
 *
 * @note Empty function. Should be implemented when dynamic 
 */
PUBLIC int mpi_datatype_init(void)
{
	/* Initializes MPI_DATATYPE_NULL. */
	OBJ_CONSTRUCT(&_mpi_datatype_null, mpi_datatype_t);
	_mpi_datatype_null.id   = -1;
	_mpi_datatype_null.size = 0;

	return (MPI_SUCCESS);
}

/**
 * @see mpi_datatype_finalize() at datatype.h.
 */
PUBLIC int mpi_datatype_finalize(void)
{
	/* Destroys MPI_DATATYPE_NULL. */
	OBJ_DESTRUCT(&_mpi_datatype_null);

	return (MPI_SUCCESS);
}
