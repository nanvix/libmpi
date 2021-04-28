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

#include <nanvix/runtime/pm.h>
#include <mputil/communication.h>
#include <mputil/comm_request.h>
#include <mpi/pt2pt_comm.h>
#include <mpi/communicator.h>
#include <mpi/datatype.h>

/**
 * @brief Provide detailed description.
 */
PUBLIC int mpi_send(const void *buf, int count, MPI_Datatype datatype, int dest,
	                int tag, MPI_Comm comm, mpi_comm_mode_t mode)
{
	int ret;
	int cid;
	int src;
	size_t size;
	int datatype_id;
	mpi_process_t *dest_proc;

	/* Gets the target process based in dest rank. */
	if ((ret = mpi_comm_get_proc(comm, dest, &dest_proc)) != MPI_SUCCESS)
		goto end;

	/* Gets the current process rank in comm. */
	if ((ret = mpi_comm_rank(comm, &src)) != MPI_SUCCESS)
		goto end;

	/* Gets the communicator context ID. */
	cid = mpi_comm_get_pt2pt_cid(comm);

	/* Calculates the send total size. */
	size = count * mpi_datatype_size(datatype);

	/* Gets the datatype id. */
	datatype_id = mpi_datatype_id(datatype);

	ret = send(cid, buf, size, src, dest, dest_proc, datatype_id, tag, mode);

end:
	return (ret);
}

/**
 * @brief Provide detailed description.
 */
PUBLIC int mpi_recv(void *buf, int count, MPI_Datatype datatype, int source,
	                int tag, MPI_Comm comm, MPI_Status *status)
{
	int ret;
	int cid;
	int rank;
	size_t size;
	int datatype_id;
	mpi_process_t *src_proc;
	struct comm_request request;

	/* Gets the communicator context ID. */
	cid = mpi_comm_get_pt2pt_cid(comm);

	/* Calculates the send total size. */
	size = count * mpi_datatype_size(datatype);

	/* Gets the target process based in source rank. */
	if ((ret = mpi_comm_get_proc(comm, source, &src_proc)) != MPI_SUCCESS)
		return (ret);

	/* Gets the datatype id. */
	datatype_id = mpi_datatype_id(datatype);

	/* Gets the local process rank. */
	if ((ret = mpi_comm_rank((mpi_communicator_t *) comm, &rank)) != MPI_SUCCESS)
		return (ret);

	/* Builds the recv request to be compared in the underlying function. */
	request.cid    = cid;
	request.src    = source;
	request.target = rank;
	request.tag    = tag;

	ret = recv(cid, buf, size, src_proc, datatype_id, &request);

	/* Updates the STATUS. */
	if (status != MPI_STATUS_IGNORE)
	{
		status->MPI_ERROR     = ret;
		status->received_size = request.received_size;
	}

	return (ret);
}