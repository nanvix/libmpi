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
#include <mpi/mpiruntime.h>

/**
 * @brief Global MPI state variable.
 */
mpi_state_t _mpi_state = MPI_STATE_NOT_INITIALIZED;

/**
 * @see mpi_init() at mpiruntime.h.
 */
PUBLIC int mpi_init(void)
{
	return (MPI_SUCCESS);
}

/**
 * @see mpi_finalize() at mpiruntime.h.
 */
PUBLIC int mpi_finalize(void)
{
	return (MPI_SUCCESS);
}

/**
 * @see mpi_abort() at mpiruntime.h.
 */
PUBLIC int mpi_abort(mpi_communicator_t *comm, int errorcode)
{
	UNUSED(comm);
	UNUSED(errorcode);

	return (MPI_SUCCESS);
}
