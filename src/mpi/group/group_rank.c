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
#include <mpi/group.h>
#include <mpi.h>

static const char FUNC_NAME[] = "MPI_Group_rank";

/**
 * @brief Puts in @p rank the rank of the calling process inside @p group.
 *
 * @param group Target group.
 * @param rank  Location where will be stored the result.
 *
 * @returns Upon successful completion, MPI_SUCCESS is returned. An MPI errorcode
 * is returned instead.
 *
 * @note If the local process doesn't appear on the given group, MPI_UNDEFINED
 * appears in @p rank.
 */
PUBLIC int MPI_Group_rank(MPI_Group group, int *rank)
{
	/* Parameters checking. */
	MPI_CHECK_INIT_FINALIZE(FUNC_NAME);

	/* Bad group. */
	if ((group == MPI_GROUP_NULL) || (group == NULL))
		return (MPI_ERRHANDLER_INVOKE(MPI_COMM_WORLD, MPI_ERR_GROUP, FUNC_NAME));

	/* Bad rank holder. */
	if (rank == NULL)
		return (MPI_ERRHANDLER_INVOKE(MPI_COMM_WORLD, MPI_ERR_ARG, FUNC_NAME));

	*rank = mpi_group_rank((mpi_group_t *) group);

	return (MPI_SUCCESS);
}
