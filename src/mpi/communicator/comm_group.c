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

static const char FUNC_NAME[] = "MPI_Comm_group";

/**
 * @brief Gets the group associated with @p comm.
 *
 * @param comm  Target communicator.
 * @param group Location where will be stored the resultant group.
 *
 * @returns Upon successful completion, MPI_SUCCESS is returned. An MPI errorcode
 * is returned instead.
 */
PUBLIC int MPI_Comm_group(MPI_Comm comm, MPI_Group *group)
{
	int ret;

	/* Parameters checking. */
	MPI_CHECK_INIT_FINALIZE(FUNC_NAME);

	/* Bad communicator. */
	if ((comm == NULL) || (comm == MPI_COMM_NULL))
		MPI_ERRHANDLER_INVOKE(MPI_COMM_WORLD, MPI_ERR_COMM, FUNC_NAME);

	/* Bad group holder. */
	if (group == NULL)
		MPI_ERRHANDLER_INVOKE(comm, MPI_ERR_ARG, FUNC_NAME);

	ret = mpi_comm_group((mpi_communicator_t *) comm, (mpi_group_t **) group);

	/* Evaluates if ret was successful and calls an errorhandler if not. */
	MPI_ERRHANDLER_CHECK(ret, comm, ret, FUNC_NAME);

	return (MPI_SUCCESS);
}
