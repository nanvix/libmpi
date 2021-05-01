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

#ifndef NANVIX_LIBMPI_MPI_H_
#define NANVIX_LIBMPI_MPI_H_

#include <posix/stdint.h>
#include <mpi/mpi_errors.h>

/**
 * @brief Defines the Upper bound for tag values.
 *
 * @note This constant is implementation dependant and 32767 is the minimum value.
 *
 * @todo See where to define this constant correctly.
 */
#define UB 32768

/**
 * @note Temporary definitions. Declared only for convenience on errhandlers definition.
 * It should be dropped from here and defined in specific files for each when properly
 * supported.
 */
typedef struct mpi_win_t mpi_win_t;
typedef struct mpi_file_t mpi_file_t;

/**
 * @brief User exported typedefs.
 */
typedef struct mpi_communicator_t *MPI_Comm;
typedef struct mpi_group_t        *MPI_Group;
typedef struct mpi_win_t          *MPI_Win;
typedef struct mpi_file_t         *MPI_File;
typedef struct mpi_errhandler_t   *MPI_Errhandler;
typedef struct mpi_datatype_t     *MPI_Datatype;
typedef struct mpi_status_t        MPI_Status;

/**
 * @brief Another predefined datatypes.
 */
typedef uint64_t *MPI_Aint;
typedef uint64_t *MPI_Count;
typedef uint64_t *MPI_Offset;

/**
 * @brief MPI_Status definition.
 *
 * @note Defined here cause MPI Specification defines that this struct
 * has public fields that may be accessed by the user.
 */
struct mpi_status_t
{
	int MPI_SOURCE; /* MPI message sender.     */
	int MPI_TAG;    /* MPI message tag.        */
	int MPI_ERROR;  /* MPI message error code. */

	/**
	 * @note The following fields are internal to the MPI implementation and
	 * should not be directly accessed, besides the implementation itself.
	 */
	int received_size; /* Received size in transfer. */
};

/**
 * @brief User exported function typedefs.
 */
typedef void MPI_Comm_errhandler_function(MPI_Comm *, int *, ...);
typedef void MPI_Win_errhandler_function(MPI_Win *, int *, ...);
typedef void MPI_File_errhandler_function(MPI_File *, int *, ...);

/**
 * @brief Maximum string sizes constants.
 */
#define MPI_MAX_DATAREP_STRING          64
#define MPI_MAX_ERROR_STRING           128
#define MPI_MAX_INFO_KEY                32
#define MPI_MAX_INFO_VAL               256
#define MPI_MAX_LIBRARY_VERSION_STRING 256
#define MPI_MAX_OBJECT_NAME             64
#define MPI_MAX_PORT_NAME              128
#define MPI_MAX_PROCESSOR_NAME         128

/* End of assorted constants. */

/**
 * @brief Special STATUS constants.
 */
#define MPI_STATUS_IGNORE   NULL
#define MPI_STATUSES_IGNORE NULL

/**
 * @brief Assorted constants.
 */
#define MPI_PROC_NULL          (-2) /* Rank of null process.         */
#define MPI_ANY_SOURCE         (-1) /* Any source rank.              */
#define MPI_ANY_TAG            (-1) /* Match any message tag.        */
#define MPI_UNDEFINED      (-32766) /* Undefined stuff.              */
#define MPI_BSEND_OVERHEAD     128  /* Size of bsend header + ptr.   */
#define MPI_KEYVAL_INVALID     (-1) /* Invalid key value.            */
#define MPI_LOCK_EXCLUSIVE       1
#define MPI_LOCK_SHARED          2
#define MPI_ROOT               (-4) /* Special value for intercomms. */

/* End of assorted constants. */

/**
 * @brief Results of the compare operations.
 */
#define MPI_IDENT     0
#define MPI_CONGRUENT 1
#define MPI_SIMILAR   2
#define MPI_UNEQUAL   3

/**
 * @brief Supported thread levels constants.
 */
#define MPI_THREAD_SINGLE     0
#define MPI_THREAD_FUNNELED   1
#define MPI_THREAD_SERIALIZED 2
#define MPI_THREAD_MULTIPLE   3

/**
 * @brief Predefined MPI_Datatype external structures.
 */
extern struct mpi_datatype_t _mpi_datatype_char;
extern struct mpi_datatype_t _mpi_datatype_short;
extern struct mpi_datatype_t _mpi_datatype_int;
extern struct mpi_datatype_t _mpi_datatype_long;
extern struct mpi_datatype_t _mpi_datatype_long_long;
extern struct mpi_datatype_t _mpi_datatype_long_long;
extern struct mpi_datatype_t _mpi_datatype_signed_char;
extern struct mpi_datatype_t _mpi_datatype_unsigned_char;
extern struct mpi_datatype_t _mpi_datatype_unsigned_short;
extern struct mpi_datatype_t _mpi_datatype_unsigned;
extern struct mpi_datatype_t _mpi_datatype_unsigned_long;
extern struct mpi_datatype_t _mpi_datatype_unsigned_long_long;
extern struct mpi_datatype_t _mpi_datatype_float;
extern struct mpi_datatype_t _mpi_datatype_double;
extern struct mpi_datatype_t _mpi_datatype_long_double;
extern struct mpi_datatype_t _mpi_datatype_wchar;
extern struct mpi_datatype_t _mpi_datatype_cbool;
extern struct mpi_datatype_t _mpi_datatype_int8;
extern struct mpi_datatype_t _mpi_datatype_int16;
extern struct mpi_datatype_t _mpi_datatype_int32;
extern struct mpi_datatype_t _mpi_datatype_int64;
extern struct mpi_datatype_t _mpi_datatype_uint8;
extern struct mpi_datatype_t _mpi_datatype_uint16;
extern struct mpi_datatype_t _mpi_datatype_uint32;
extern struct mpi_datatype_t _mpi_datatype_uint64;
extern struct mpi_datatype_t _mpi_datatype_ccomplex;
extern struct mpi_datatype_t _mpi_datatype_ccomplex;
extern struct mpi_datatype_t _mpi_datatype_double_complex;
extern struct mpi_datatype_t _mpi_datatype_long_double_complex;
extern struct mpi_datatype_t _mpi_datatype_byte;
extern struct mpi_datatype_t _mpi_datatype_packed;
extern struct mpi_datatype_t _mpi_datatype_aint;
extern struct mpi_datatype_t _mpi_datatype_offset;
extern struct mpi_datatype_t _mpi_datatype_count;

/**
 * @brief Predefined datatypes.
 */
#define MPI_CHAR                  &_mpi_datatype_char
#define MPI_SHORT                 &_mpi_datatype_short
#define MPI_INT                   &_mpi_datatype_int
#define MPI_LONG                  &_mpi_datatype_long
#define MPI_LONG_LONG_INT         &_mpi_datatype_long_long
#define MPI_LONG_LONG             &_mpi_datatype_long_long
#define MPI_SIGNED_CHAR           &_mpi_datatype_signed_char
#define MPI_UNSIGNED_CHAR         &_mpi_datatype_unsigned_char
#define MPI_UNSIGNED_SHORT        &_mpi_datatype_unsigned_short
#define MPI_UNSIGNED              &_mpi_datatype_unsigned
#define MPI_UNSIGNED_LONG         &_mpi_datatype_unsigned_long
#define MPI_UNSIGNED_LONG_LONG    &_mpi_datatype_unsigned_long_long
#define MPI_FLOAT                 &_mpi_datatype_float
#define MPI_DOUBLE                &_mpi_datatype_double
#define MPI_LONG_DOUBLE           &_mpi_datatype_long_double
#define MPI_WCHAR                 &_mpi_datatype_wchar
#define MPI_C_BOOL                &_mpi_datatype_cbool
#define MPI_INT8_T                &_mpi_datatype_int8
#define MPI_INT16_T               &_mpi_datatype_int16
#define MPI_INT32_T               &_mpi_datatype_int32
#define MPI_INT64_T               &_mpi_datatype_int64
#define MPI_UINT8_T               &_mpi_datatype_uint8
#define MPI_UINT16_T              &_mpi_datatype_uint16
#define MPI_UINT32_T              &_mpi_datatype_uint32
#define MPI_UINT64_T              &_mpi_datatype_uint64
#define MPI_C_COMPLEX             &_mpi_datatype_ccomplex
#define MPI_C_FLOAT_COMPLEX       &_mpi_datatype_ccomplex
#define MPI_C_DOUBLE_COMPLEX      &_mpi_datatype_double_complex
#define MPI_C_LONG_DOUBLE_COMPLEX &_mpi_datatype_long_double_complex
#define MPI_BYTE                  &_mpi_datatype_byte
#define MPI_PACKED                &_mpi_datatype_packed
#define MPI_AINT                  &_mpi_datatype_aint
#define MPI_OFFSET                &_mpi_datatype_offset
#define MPI_COUNT                 &_mpi_datatype_count

/**
 * @brief External predefined structures.
 *
 * @note These are global predefined structures exported as definitions below.
 */
extern struct mpi_group_t        _mpi_group_empty;
extern struct mpi_group_t        _mpi_group_null;
extern struct mpi_communicator_t _mpi_comm_world;
extern struct mpi_communicator_t _mpi_comm_self;
extern struct mpi_communicator_t _mpi_comm_null;
extern struct mpi_errhandler_t   _mpi_errhandler_errors_fatal;
extern struct mpi_errhandler_t   _mpi_errhandler_errors_abort;
extern struct mpi_errhandler_t   _mpi_errors_return;
extern struct mpi_errhandler_t   _mpi_errhandler_null;
extern struct mpi_datatype_t     _mpi_datatype_null;

/**
 * @brief Predefined handlers.
 */
#define MPI_GROUP_EMPTY      &_mpi_group_empty
#define MPI_COMM_SELF        &_mpi_comm_self
#define MPI_COMM_WORLD       &_mpi_comm_world
#define MPI_ERRORS_ARE_FATAL &_mpi_errhandler_errors_fatal
#define MPI_ERRORS_ABORT     &_mpi_errhandler_errors_abort
#define MPI_ERRORS_RETURN    &_mpi_errors_return;

/**
 * @brief Null handlers.
 */
#define MPI_GROUP_NULL      &_mpi_group_null
#define MPI_COMM_NULL       &_mpi_comm_null
#define MPI_ERRHANDLER_NULL &_mpi_errhandler_null
#define MPI_DATATYPE_NULL   &_mpi_datatype_null

/**
 * @brief MPI_Functions declaration.
 */
extern int MPI_Abort(MPI_Comm comm, int errorcode);
extern int MPI_Comm_group(MPI_Comm comm, MPI_Group *group);
extern int MPI_Comm_rank(MPI_Comm comm, int *rank);
extern int MPI_Comm_size(MPI_Comm comm, int *size);
extern int MPI_Comm_get_errhandler(MPI_Comm comm, MPI_Errhandler *errhandler);
extern int MPI_Comm_set_errhandler(MPI_Comm comm, MPI_Errhandler errhandler);
extern int MPI_Errhandler_free(MPI_Errhandler *errhandler);
extern int MPI_Finalize(void);
extern int MPI_Finalized(int *flag);
extern int MPI_Get_count(const MPI_Status *status, MPI_Datatype datatype, int *count);
extern int MPI_Group_rank(MPI_Group group, int *rank);
extern int MPI_Group_size(MPI_Group group, int *size);
extern int MPI_Group_free(MPI_Group *group);
extern int MPI_Init(int *argc, char ***argv);
extern int MPI_Initialized(int *flag);
extern int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status);
extern int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);

extern int mpi_std_barrier(void);

#endif /* NANVIX_LIBMPI_H_ */
