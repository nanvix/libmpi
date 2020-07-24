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

#include <mpi/pt2pt_comm.h>
#include <mpi/communicator.h>
#include <mpi/datatype.h>
#include <mpi.h>

static const char FUNC_NAME[] = "MPI_Recv";

/**
 * @brief Receives a message from a target process inside @p comm context.
 *
 * @param buf      Data buffer to be sent.
 * @param count    Number of @p datatype entries to in @p buf.
 * @param datatype Type of data to be transfered.
 * @param source   Source process rank.
 * @param tag      Message identifier.
 * @param comm     Communicator that defines the context of communication.
 * @param status   Status variable to hold extra information about the communication.
 *
 * @returns Upon successful completion, MPI_SUCCESS is returned. An MPI errorcode
 * is returned instead.
 */
PUBLIC int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
{
	int ret; /* Function return. */

	/* Parameters checking. */
	MPI_CHECK_INIT_FINALIZE(FUNC_NAME);

	/* Bad communicator. */
	if (!mpi_comm_is_valid(comm))
		return (MPI_ERRHANDLER_INVOKE(MPI_COMM_WORLD, MPI_ERR_COMM, FUNC_NAME));

	ret = MPI_SUCCESS;

	/* Bad count. */
	if (count < 0)
		ret = MPI_ERR_COUNT;

	/* Bad tag. */
	if (!WITHIN(tag, 0, UB) && (tag != MPI_ANY_TAG))
		ret = MPI_ERR_TAG;

	/* Bad buffer. */
	if ((buf == NULL) && (count > 0))
		ret = MPI_ERR_BUFFER;

	/* Bad datatype. */
	if (!mpi_datatype_is_valid(datatype))
		ret = MPI_ERR_TYPE;

	/**
	 * @note MPI_Status == NULL means MPI_STATUS_IGNORE, and so this parameter
	 * doesn't need to be verified.
	 */

	/* Bad source. */
	if ((source != MPI_PROC_NULL) && (source != MPI_ANY_SOURCE) && (!mpi_comm_peer_rank_is_valid(comm, source)))
		ret = MPI_ERR_RANK;

	/* Checks if there was an error and calls an error handler case positive. */
	MPI_ERRHANDLER_CHECK(ret, comm, ret, FUNC_NAME);

	/* Initializes status variable. */
	if (status != MPI_STATUS_IGNORE)
	{
		status->MPI_SOURCE = source;
		status->MPI_TAG    = tag;
	}

	/* Null recv. */
	if (source == MPI_PROC_NULL)
	{
		if (status != MPI_STATUS_IGNORE)
		{
			status->MPI_ERROR     = MPI_SUCCESS;
			status->received_size = 0;
		}

		return (MPI_SUCCESS);
	}

	ret = mpi_recv(buf, count, datatype, source, tag, comm, status);

	/* Checks if the send op was successful. */
	MPI_ERRHANDLER_CHECK(ret, comm, ret, FUNC_NAME);	

	return (MPI_SUCCESS);
}
