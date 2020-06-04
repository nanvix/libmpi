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

static const char FUNC_NAME[] = "MPI_Group_free";

/**
 * @brief Marks a group object for deallocation and releases its reference.
 *
 * @param group Group to be freed.
 *
 * @returns Upon successful completion, MPI_SUCCESS is returned. An MPI errorcode
 * is returned instead.
 */
PUBLIC int MPI_Group_free(MPI_Group *group)
{
	int ret;

	/* Parameters checking. */
	MPI_CHECK_INIT_FINALIZE(FUNC_NAME);

	/* Bad group. */
	if ((group == NULL) || (*group == NULL) || (*group == MPI_GROUP_NULL))
		return (MPI_ERRHANDLER_INVOKE(MPI_COMM_WORLD, MPI_ERR_GROUP, FUNC_NAME));

	ret = mpi_group_free((mpi_group_t **) group);

	/* Checks if the group was successfully freed. */
	MPI_ERRHANDLER_CHECK(ret, MPI_COMM_WORLD, ret, FUNC_NAME);

	return (MPI_SUCCESS);
}
