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

#ifndef NANVIX_MPI_ERRHANDLER_H_
#define NANVIX_MPI_ERRHANDLER_H_

#include <mputil/object.h>
#include <mpi/mpiruntime.h>
#include <mpi/errhandler_predefined.h>
#include <mpi.h>

/**
 * @brief Enum to describe the type of object that the error handler is defined.
 */
enum mpi_errhandler_type_t {
    MPI_ERRHANDLER_TYPE_PREDEFINED,
    MPI_ERRHANDLER_TYPE_COMM,
    MPI_ERRHANDLER_TYPE_WIN,
    MPI_ERRHANDLER_TYPE_FILE
};
typedef enum mpi_errhandler_type_t mpi_errhandler_type_t;

/**
 * @brief Struct that defines a mpi_errhandler.
 */
struct mpi_errhandler_t
{
	object_t super; /* Base object class. */

	mpi_errhandler_type_t errhandler_object_type;  /* Errhandler defined for... */
	MPI_Comm_errhandler_function *comm_handler_fn; /* Comm_errhandler function. */
	MPI_Win_errhandler_function *win_handler_fn;   /* File_errhandler function. */
	MPI_File_errhandler_function *file_handler_fn; /* Win_errhandler function.  */
};

typedef struct mpi_errhandler_t mpi_errhandler_t;

/* Class declaration. */
OBJ_CLASS_DECLARATION(mpi_errhandler_t);

/**
 * @brief Checks the current state of MPI environment and calls an error
 * handler if not valid (not initialized or already finished).
 *
 * @param name Name of the erroneus function.
 *
 * @note Calls MPI_ERRORS_ARE_FATAL handler directly because MPI_Comm_World doesn't
 * exist (not initialized before MPI_Init or already destroyed after MPI_Finalize).
 */
#define MPI_CHECK_INIT_FINALIZE(name)				             \
	{		                                                     \
		int state = _mpi_state;		                             \
		if ((state < MPI_STATE_INITIALIZED) ||		             \
		    (state >= MPI_STATE_FINALIZE_STARTED)) {		     \
			mpi_errors_are_fatal_comm_handler(NULL, NULL, name); \
		}	                                                     \
	}

/**
 * @brief Invokes an object error handler.
 *
 * @param mpi_object The MPI object to invoke the errhandler.
 * @param errcode    The error code to be returned.
 * @param message    Aditional error message.
 */
#define MPI_ERRHANDLER_INVOKE(mpi_object, errcode, message)	      \
	mpi_errhandler_invoke((mpi_object)->error_handler,            \
	                      (mpi_object),                           \
	                      (int)((mpi_object)->errhandler_type),   \
	                      errcode, message                        \
	)

/**
 * @brief Conditionally invokes an MPI error handler.
 *
 * @param rc         The return code to be checked.
 * @param mpi_object The MPI object to invoke the errhandler.
 * @param errcode    The error code to be returned.
 * @param message    Aditional error message.
 *
 * @note This macro will invoke the error handler if the return
 * code is not MPI_SUCCESS, and return @p errorcode.
 */
#define MPI_ERRHANDLER_CHECK(rc, mpi_object, errcode, message) 			\
	if (rc != MPI_SUCCESS) {                                        	\
		mpi_errhandler_invoke((mpi_object)->error_handler,              \
		                      (mpi_object),                             \
		                      (int)(mpi_object)->errhandler_type,       \
		                      errcode, message                          \
		);                                                              \
		return (errcode);                                               \
	}

/**
 * @brief Frees the specified error handler.
 *
 * @param errhandler The error handler to be freed.
 *
 * @returns Upon successful completion, zero is returned. An error code is
 * returned instead.
 */
extern int mpi_errhandler_free(mpi_errhandler_t ** errhandler);

/**
 * @brief Invokes an object error handler.
 *
 * @param errhandler The error handler to invoke.
 * @param mpi_object The object to invoke the errhandler.
 * @param type       The type of @p mpi_object.
 * @param errcode    The error code associated to the call.
 * @param message    Additional error message.
 *
 * @returns @p errcode.
 *
 * @note This function should be called from one of the macros given above.
 */
extern int mpi_errhandler_invoke(mpi_errhandler_t *errhandler, void *mpi_object,
                                 int type, int errcode, const char *message);

/**
 * @brief Checks if an error handler pointer is valid.
 *
 * @param errhandler Target error handler.
 *
 * @returns Zero if the error handler is not valid, and a non-zero
 * value otherwise.
 */
static inline int mpi_errhandler_is_valid(mpi_errhandler_t* errhandler)
{
    return ((errhandler != NULL) && (errhandler != MPI_ERRHANDLER_NULL));
}

/**
 * @brief Initializes the error handling submodule.
 *
 * @returns Upon successful completion, zero is returned. A
 * negative error code is returned instead.
 */
extern int mpi_errhandler_init(void);

/**
 * @brief Finalizes the error handling submodule.
 *
 * @returns Upon successful completion, zero is returned. A
 * negative error code is returned instead.
 */
extern int mpi_errhandler_finalize(void);

#endif /* NANVIX_MPI_ERRHANDLER_H_ */
