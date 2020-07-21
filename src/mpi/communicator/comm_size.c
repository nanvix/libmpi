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

#include <mpi/errhandler.h>
#include <mpi/communicator.h>
#include <mpi.h>

static const char FUNC_NAME[] = "MPI_Comm_size";

/**
 * @brief Gets the number of processes in the group of @p comm.
 *
 * @param comm Target communicator.
 * @param size Location where will be stored the result.
 *
 * @returns Upon successful completion, MPI_SUCCESS is returned. An MPI errorcode
 * is returned instead.
 */
PUBLIC int MPI_Comm_size(MPI_Comm comm, int *size)
{
	/* Parameters checking. */
	MPI_CHECK_INIT_FINALIZE(FUNC_NAME);

	/* Bad communicator. */
	if (!mpi_comm_is_valid(comm))
		return (MPI_ERRHANDLER_INVOKE(MPI_COMM_WORLD, MPI_ERR_COMM, FUNC_NAME));

	/* Bad size holder. */
	if (size == NULL)
		return (MPI_ERRHANDLER_INVOKE(comm, MPI_ERR_ARG, FUNC_NAME));

	*size = mpi_comm_size((mpi_communicator_t *) comm);

	return (MPI_SUCCESS);
}
