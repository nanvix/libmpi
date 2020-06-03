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
#include <mpi/errhandler_predefined.h>
#include <mpi/communicator.h>
#include <mpi/mpiruntime.h>

static void print_error_message(int *errcode, va_list arglist);
static void backend_abort(mpi_communicator_t *comm, int *errcode, va_list arglist);

/*============================================================================*
 * MPI_ERRORS_ARE_FATAL                                                       *
 *============================================================================*/

PUBLIC void mpi_errors_are_fatal_comm_handler(mpi_communicator_t **comm,
                                              int *errcode, ...)
{
	int state;
	va_list arglist;

	UNUSED(comm);

	state = _mpi_state;

	va_start(arglist, errcode);

	/* Error occured within MPI_Init and MPI_Finalize? */
	if (state >= MPI_STATE_INITIALIZED && state < MPI_STATE_FINALIZE_STARTED)
	{
		/* Aborts all connected processes. */
		backend_abort(&_mpi_comm_world, errcode, arglist);
	}
	else
	{
		/* Error occured before Init / after Finalize. Abort only locally. */
		backend_abort(NULL, errcode, arglist);
	}
}

PUBLIC void mpi_errors_are_fatal_file_handler(mpi_file_t **file,
                                              int *errcode, ...)
{
	UNUSED(file);
	UNUSED(errcode);

	uprintf("File handlers not supported yet.");
}

PUBLIC void mpi_errors_are_fatal_win_handler(mpi_win_t **win,
                                             int *errcode, ...)
{
	UNUSED(win);
	UNUSED(errcode);

	uprintf("Window handlers not supported yet.");
}

/*============================================================================*
 * MPI_ERRORS_ABORT                                                           *
 *============================================================================*/

PUBLIC void mpi_errors_abort_comm_handler(mpi_communicator_t **comm,
                                          int *errcode, ...)
{
	mpi_communicator_t *abort_comm;
	va_list arglist;

	va_start(arglist, errcode);

	/* If no comm passed as argument, propagates the error through MPI_COMM_SELF. */
	if (comm == NULL)
		abort_comm = &_mpi_comm_self;
	else
		abort_comm = *comm;

	backend_abort(abort_comm, errcode, arglist);
}

PUBLIC void mpi_errors_abort_file_handler(mpi_file_t **file,
                                          int *errcode, ...)
{
	UNUSED(file);
	UNUSED(errcode);

	uprintf("File handlers not supported yet.");
}

PUBLIC void mpi_errors_abort_win_handler(mpi_win_t **win,
                                         int *errcode, ...)
{
	UNUSED(win);
	UNUSED(errcode);

	uprintf("Window handlers not supported yet.");
}

/*============================================================================*
 * MPI_ERRORS_RETURN                                                          *
 *============================================================================*/

PUBLIC void mpi_errors_return_comm_handler(mpi_communicator_t **comm,
                                           int *errcode, ...)
{
	/* Do nothing. Errcode will already be returned to the user. */
	/* @note Don't remove this piece of code. It makes compiler happy. */
	va_list arglist;
	va_start(arglist, errcode);
	va_end(arglist);

	UNUSED(comm);
	UNUSED(errcode);
}

PUBLIC void mpi_errors_return_file_handler(mpi_file_t **file,
                                           int *errcode, ...)
{
	UNUSED(file);
	UNUSED(errcode);

	uprintf("File handlers not supported yet.");
}

PUBLIC void mpi_errors_return_win_handler(mpi_win_t **win,
                                          int *errcode, ...)
{
	UNUSED(win);
	UNUSED(errcode);

	uprintf("Window handlers not supported yet.");
}

/*
 * Note that this function has to handle pre-MPI_INIT and
 * post-MPI_FINALIZE errors, which backend_fatal_aggregate() does not
 * have to handle.
 *
 * This function also intentionally does not call malloc(), just in
 * case we're being called due to some kind of stack/memory error --
 * we *might* be able to get a message out if we're not further
 * corrupting the stack by calling malloc()...
 */
static void print_error_message(int *errcode, va_list arglist)
{
	int state;
	char *arg;

	state = _mpi_state;
	arg = va_arg(arglist, char*);

	if (state < MPI_STATE_INIT_STARTED)
	{
		if (arg == NULL)
		{
			uprintf("ERROR!!! A function was called before MPI_Init() was invoked, what " \
			        "is not allowed by the MPI standard.");
		}
		else
		{
			uprintf("ERROR!!! %s() function called before MPI_Init() was invoked, what " \
			        "is not allowed by the MPI standard.", arg);
		}
	}
	else if (state >= MPI_STATE_FINALIZE_STARTED)
	{
		if (arg == NULL)
		{
			uprintf("ERROR!!! A function was called after MPI_Finalize() was invoked, what" \
			        "is not allowed by the MPI standard.");
		}
		else
		{
			uprintf("ERROR!!! %s() function called after MPI_Finalize() was invoked, what" \
			        "is not allowed by the MPI standard.", arg);
		}
	}
	else
	{
		if (arg == NULL)
			uprintf("ERROR!!!");
		else
			uprintf("ERROR!!! %s", arg);

		if (errcode != NULL)
			uprintf("Error code: %d", *errcode);
	}

	va_end(arglist);
}

static void backend_abort(mpi_communicator_t *comm, int *errcode, va_list arglist)
{
	print_error_message(errcode, arglist);

	if (errcode != NULL)
		mpi_abort(comm, *errcode);
	else
		mpi_abort(comm, 1);
}
