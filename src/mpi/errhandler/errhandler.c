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
#include <posix/errno.h>
#include <mpi/errhandler.h>
#include <mpi/communicator.h>

PRIVATE void mpi_errhandler_construct(mpi_errhandler_t *);
PRIVATE void mpi_errhandler_destruct(mpi_errhandler_t *);

OBJ_CLASS_INSTANCE(mpi_errhandler_t, &mpi_errhandler_construct,
                   &mpi_errhandler_destruct, sizeof(mpi_errhandler_t));

/**
 * @brief Predefined error handlers.
 */
mpi_errhandler_t _mpi_errhandler_null;
mpi_errhandler_t _mpi_errhandler_errors_fatal;
mpi_errhandler_t _mpi_errhandler_errors_abort;
mpi_errhandler_t _mpi_errors_return;

/**
 * @brief Error handler constructor.
 */
PRIVATE void mpi_errhandler_construct(mpi_errhandler_t * errhandler)
{
	uassert(errhandler != NULL);

	errhandler->comm_handler_fn = NULL;
	errhandler->win_handler_fn  = NULL;
	errhandler->file_handler_fn = NULL;
}

/**
 * @brief Error handler destructor.
 */
PRIVATE void mpi_errhandler_destruct(mpi_errhandler_t * errhandler)
{
	uassert(errhandler != NULL);
}

/**
 * @see mpi_errhandler_invoke() at errhandler.h.
 */
PUBLIC int mpi_errhandler_invoke(mpi_errhandler_t *errhandler, void *mpi_object,
                                 int type, int errcode, const char *message)
{
	mpi_communicator_t *comm;
	mpi_win_t *win;
	mpi_file_t *file;

	/* Invalid errhandler. Abort all connected processes. */
	if (errhandler == NULL) {
		mpi_errors_are_fatal_comm_handler(NULL, NULL, message);
		return errcode;
	}

	/* Select handler function based on @p type. */
	switch (type)
	{
		case MPI_ERRHANDLER_TYPE_COMM:
			comm = (mpi_communicator_t *) mpi_object;
			errhandler->comm_handler_fn(&comm, &errcode, message, NULL);
			break;

		case MPI_ERRHANDLER_TYPE_WIN:
			win = (mpi_win_t *) mpi_object;
			errhandler->win_handler_fn(&win, &errcode, message, NULL);
			break;

		case MPI_ERRHANDLER_TYPE_FILE:
			file = (mpi_file_t *) mpi_object;
			errhandler->file_handler_fn(&file, &errcode, message, NULL);
			break;

		default:
			/* UNREACHABLE */
			upanic("Unrecognized Errhandler type.");
			break;
	}

	return errcode;
}

/**
 * @see mpi_errhandler_free() at errhandler.h.
 */
PUBLIC int mpi_errhandler_free(mpi_errhandler_t ** errhandler)
{
	/* Bad error handler. */
	if (*errhandler == NULL)
		return (MPI_ERR_ARG);

	OBJ_RELEASE(*errhandler);
	*errhandler = MPI_ERRHANDLER_NULL;

	return (MPI_SUCCESS);
}

/**
 * @see mpi_errhandler_init() at errhandler.h.
 */
PUBLIC int mpi_errhandler_init(void)
{
	/* MPI_ERRHANDLER_ERRORS_ARE_FATAL. */
	OBJ_CONSTRUCT(&_mpi_errhandler_errors_fatal, mpi_errhandler_t);
	_mpi_errhandler_errors_fatal.errhandler_object_type = MPI_ERRHANDLER_TYPE_PREDEFINED;
	_mpi_errhandler_errors_fatal.comm_handler_fn        = &mpi_errors_are_fatal_comm_handler;
	_mpi_errhandler_errors_fatal.win_handler_fn         = &mpi_errors_are_fatal_win_handler;
	_mpi_errhandler_errors_fatal.file_handler_fn        = &mpi_errors_are_fatal_file_handler;

	/* MPI_ERRHANDLER_ERRORS_ABORT. */
	OBJ_CONSTRUCT(&_mpi_errhandler_errors_abort, mpi_errhandler_t);
	_mpi_errhandler_errors_abort.errhandler_object_type = MPI_ERRHANDLER_TYPE_PREDEFINED;
	_mpi_errhandler_errors_abort.comm_handler_fn        = &mpi_errors_abort_comm_handler;
	_mpi_errhandler_errors_abort.win_handler_fn         = &mpi_errors_abort_win_handler;
	_mpi_errhandler_errors_abort.file_handler_fn        = &mpi_errors_abort_file_handler;

	/* MPI_ERRHANDLER_ERRORS_RETURN. */
	OBJ_CONSTRUCT(&_mpi_errors_return, mpi_errhandler_t);
	_mpi_errors_return.errhandler_object_type = MPI_ERRHANDLER_TYPE_PREDEFINED;
	_mpi_errors_return.comm_handler_fn        = &mpi_errors_return_comm_handler;
	_mpi_errors_return.win_handler_fn         = &mpi_errors_return_win_handler;
	_mpi_errors_return.file_handler_fn        = &mpi_errors_return_file_handler;

	/* MPI_ERRHANDLER_NULL. */
	OBJ_CONSTRUCT(&_mpi_errhandler_null, mpi_errhandler_t);
	_mpi_errhandler_null.errhandler_object_type = MPI_ERRHANDLER_TYPE_PREDEFINED;
	_mpi_errhandler_null.comm_handler_fn        = NULL;
	_mpi_errhandler_null.win_handler_fn         = NULL;
	_mpi_errhandler_null.file_handler_fn        = NULL;

	return (MPI_SUCCESS);
}

/**
 * @see mpi_errhandler_finalize() at errhandler.h.
 */
PUBLIC int mpi_errhandler_finalize(void)
{
	/* Destructs the predefined error handlers. */
	OBJ_DESTRUCT(&_mpi_errhandler_null);
	OBJ_DESTRUCT(&_mpi_errhandler_errors_fatal);
	OBJ_DESTRUCT(&_mpi_errhandler_errors_abort);
	OBJ_DESTRUCT(&_mpi_errors_return);

	return (MPI_SUCCESS);
}
