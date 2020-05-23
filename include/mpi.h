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
 * @brief Error Codes and Classes.
 */
#define MPI_SUCCESS                    0 /* No error. */
#define MPI_ERR_BUFFER                 1 /* Invalid buffer pointer. */
#define MPI_ERR_COUNT                  2 /* Invalid count argument. */
#define MPI_ERR_TYPE                   3 /* Invalid datatype argument. */
#define MPI_ERR_TAG                    4 /* Invalid tag argument. */
#define MPI_ERR_COMM                   5 /* Invalid communicator. */
#define MPI_ERR_RANK                   6 /* Invalid rank. */
#define MPI_ERR_REQUEST                7 /* Invalid request (handle). */
#define MPI_ERR_ROOT                   8 /* Invalid root. */
#define MPI_ERR_GROUP                  9 /* Invalid group. */
#define MPI_ERR_OP                    10 /* Invalid operation. */
#define MPI_ERR_TOPOLOGY              11 /* Invalid topology. */
#define MPI_ERR_DIMS                  12 /* Invalid dimension argument. */
#define MPI_ERR_ARG                   13 /* Invalid argument of other kind. */
#define MPI_ERR_UNKNOWN               14 /* Unknown error. */
#define MPI_ERR_TRUNCATE              15 /* Message truncated on receive. */
#define MPI_ERR_OTHER                 16 /* Known error not in this list. */
#define MPI_ERR_INTERN                17 /* Internal MPI impl error. */
#define MPI_ERR_PENDING               18 /* Pending request. */
#define MPI_ERR_IN_STATUS             19 /* Error code is in status. */
#define MPI_ERR_ACCESS                20 /* Permission denied. */
#define MPI_ERR_AMODE                 21 /* Error related to amode on I/O. */
#define MPI_ERR_ASSERT                22 /* Invalid assert argument. */
#define MPI_ERR_BAD_FILE              23 /* Invalid file name. */
#define MPI_ERR_BASE                  24 /* Invalid base to MPI_FREE_MEM. */
#define MPI_ERR_CONVERSION            25 /* Error in a user conv function. */
#define MPI_ERR_DISP                  26 /* Invalid disp argument. */
#define MPI_ERR_DUP_DATAREP           27 /* Already defined data representation. */
#define MPI_ERR_FILE_EXISTS           28 /* File exists. */
#define MPI_ERR_FILE_IN_USE           29 /* File currently open by some process. */
#define MPI_ERR_FILE                  30 /* Invalid file handle. */
#define MPI_ERR_INFO_KEY              31 /* Key longer than MPI_MAX_INFO_KEY. */
#define MPI_ERR_INFO_NOKEY            32 /* Invalid key to MPI_INFO_DELETE. */
#define MPI_ERR_INFO_VALUE            33 /* Value longer than MPI_MAX_INFO_VAL. */
#define MPI_ERR_INFO                  34 /* Invalid info argument. */
#define MPI_ERR_IO                    35 /* Other I/O error. */
#define MPI_ERR_KEYVAL                36 /* Invalid keyval has been passed. */
#define MPI_ERR_LOCKTYPE              37 /* Invalid locktype argument. */
#define MPI_ERR_NAME                  38 /* Invalid srvc name to MPI_LOOKUP_NAME. */
#define MPI_ERR_NO_MEM                39 /* Memory is exhausted. */
#define MPI_ERR_NOT_SAME              40 /* Collective arg not identical on all procs. */
#define MPI_ERR_NO_SPACE              41 /* Not enough space. */
#define MPI_ERR_NO_SUCH_FILE          42 /* File does not exist. */
#define MPI_ERR_PORT                  43 /* Invalid port name to MPI_COMM_CONNECT. */
#define MPI_ERR_QUOTA                 44 /* Quota exceeded. */
#define MPI_ERR_READ_ONLY             45 /* Read-only file or file system. */
#define MPI_ERR_RMA_ATTACH            46
#define MPI_ERR_RMA_CONFLICT          47 /* Conflicting accesses to window. */
#define MPI_ERR_RMA_RANGE             48
#define MPI_ERR_RMA_SHARED            49
#define MPI_ERR_RMA_SYNC              50 /* Wrong synchronization of RMA calls. */
#define MPI_ERR_RMA_FLAVOR            51
#define MPI_ERR_SERVICE               52 /* Invalid srvc nameto MPI_UNPUBLISH_NAME. */
#define MPI_ERR_SIZE                  53 /* Invalid size argument. */
#define MPI_ERR_SPAWN                 54 /* Error in spawning processes. */
#define MPI_ERR_UNSUPPORTED_DATAREP   55 /* Unsupp datarep to MPI_FILE_SET_VIEW. */
#define MPI_ERR_UNSUPPORTED_OPERATION 56 /* Unsupported operation. */
#define MPI_ERR_WIN                   57 /* Invalid win argument. */
#define MPI_T_ERR_CANNOT_INIT         58
#define MPI_T_ERR_NOT_INITIALIZED     59
#define MPI_T_ERR_MEMORY              60
#define MPI_T_ERR_INVALID             61
#define MPI_T_ERR_INVALID_INDEX       62
#define MPI_T_ERR_INVALID_ITEM        63
#define MPI_T_ERR_INVALID_SESSION     64
#define MPI_T_ERR_INVALID_HANDLE      65
#define MPI_T_ERR_INVALID_NAME        66
#define MPI_T_ERR_OUT_OF_HANDLES      67
#define MPI_T_ERR_OUT_OF_SESSIONS     68
#define MPI_T_ERR_CVAR_SET_NOT_NOW    69
#define MPI_T_ERR_CVAR_SET_NEVER      70
#define MPI_T_ERR_PVAR_NO_WRITE       71
#define MPI_T_ERR_PVAR_NO_STARTSTOP   72
#define MPI_T_ERR_PVAR_NO_ATOMIC      73

/**
 * @note Used to sanytize codes validity. Should gave some room for user added error codes.
 */
#define MPI_ERR_LASTCODE              92 /* Last error code . */

/* End of error classes definitions. */

/**
 * @brief Supported thread levels constants.
 */
#define MPI_THREAD_SINGLE     0
#define MPI_THREAD_FUNNELED   1
#define MPI_THREAD_SERIALIZED 2
#define MPI_THREAD_MULTIPLE   3

/**
 * @brief External predefined structures.
 *
 * @note These are global predefined structures exported as definitions below.
 */
extern struct mpi_group_t _mpi_group_empty;
extern struct mpi_group_t _mpi_group_null;
extern struct mpi_communicator_t _mpi_comm_world;
extern struct mpi_communicator_t _mpi_comm_self;
extern struct mpi_communicator_t _mpi_comm_null;

/**
 * @brief Predefined handlers.
 */
#define MPI_GROUP_EMPTY &_mpi_group_empty
#define MPI_COMM_SELF   &_mpi_comm_self
#define MPI_COMM_WORLD  &_mpi_comm_world

/**
 * @brief Null handlers.
 */
#define MPI_GROUP_NULL &_mpi_group_null
#define MPI_COMM_NULL  &_mpi_comm_null

#endif /* NANVIX_LIBMPI_H_ */
