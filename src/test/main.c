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

#include <nanvix/hal.h>
#include <nanvix/ulib.h>
#include <nanvix/config.h>
#include <nanvix/limits.h>
#include <mpi.h>

#include <mpi/datatype.h>

/**
 * @brief Dummy test driver.
 */
int __main3(int argc, char *argv[])
{
	int flag;
	int rank, rank2;
	size_t usize;
	int size2, size;
	int expected_rank;
	MPI_Group group;
	MPI_Errhandler errhandler;

	int inbuffer, outbuffer;
	int remote;

	UNUSED(argc);
	UNUSED(argv);

	expected_rank = cluster_get_num() - SPAWNERS_NUM;

	uprintf("---------------------------------------------");

	MPI_Initialized(&flag);
	uassert(!flag);
	uprintf("Asserted Not initialized");

	MPI_Finalized(&flag);
	uassert(!flag);
	uprintf("Asserted Not finalized");

	MPI_Init(&argc, &argv);
	uprintf("Init successful!");

	MPI_Initialized(&flag);
	uassert(flag);
	uprintf("Asserted initialization");

	MPI_Finalized(&flag);
	uassert(!flag);
	uprintf("Asserted Not finalized");

	usize = mpi_datatype_size(MPI_INT);
	uassert(usize == sizeof(int));
	uprintf("Datatype size asserted");

	usize = mpi_datatype_size(MPI_DATATYPE_NULL);
	uassert(usize == 0);
	uprintf("NULL Datatype size asserted");

	MPI_Comm_group(MPI_COMM_WORLD, &group);

	uassert(group != MPI_GROUP_EMPTY);

	uprintf("Group asserted");

	MPI_Group_rank(group, &rank);
	uassert(rank == expected_rank);

	uprintf("Group rank asserted");

	MPI_Comm_rank(MPI_COMM_WORLD, &rank2);
	uassert(rank2 == expected_rank);

	uprintf("Comm rank asserted");

	MPI_Group_size(group, &size);
	uassert(size == NANVIX_PROC_MAX);

	uprintf("Group size asserted");

	MPI_Comm_size(MPI_COMM_WORLD, &size2);

	uassert(size == size2);
	uprintf("Asserted MPI_Comm_size");

	MPI_Group_free(&group);

	uassert(group == MPI_GROUP_NULL);

	uprintf("NULL Group asserted");

	MPI_Comm_get_errhandler(MPI_COMM_WORLD, &errhandler);

	uassert(errhandler == MPI_ERRORS_ARE_FATAL);

	uprintf("COMM_WORLD errorhandler asserted");

	MPI_Errhandler_free(&errhandler);

	MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_ABORT);

	MPI_Comm_get_errhandler(MPI_COMM_WORLD, &errhandler);

	uassert(errhandler == MPI_ERRORS_ABORT);

	uprintf("Asserted Comm_set_errhandler");

	MPI_Errhandler_free(&errhandler);

	uassert(errhandler == MPI_ERRHANDLER_NULL);

	uprintf("MPI_ERRHANDLER_NULL asserted");

	uprintf("---------------------------------------------");

	/* Communication test. */
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	outbuffer = rank;
	inbuffer = (-1);

	/* Pairs communication. */
	if ((rank % 2) == 0)
	{
		remote = rank + 1;

		uprintf("Rank %d preparing to send to rank %d", rank, remote);

		MPI_Send(&outbuffer, 1, MPI_INT, remote, 0, MPI_COMM_WORLD);

		uprintf("Waiting for reply...");

		MPI_Recv(&inbuffer, 1, MPI_INT, remote, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
	else
	{
		remote = rank - 1;

		uprintf("Rank %d waiting to receive from rank %d", rank, remote);

		MPI_Recv(&inbuffer, 1, MPI_INT, remote, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		uprintf("Sending reply...");

		MPI_Send(&outbuffer, 1, MPI_INT, remote, 0, MPI_COMM_WORLD);
	}

	uprintf("Communication done!");

	uassert(inbuffer == remote);

	uprintf("Successful communication :)");

	uprintf("---------------------------------------------");

	/* Finalization. */
	MPI_Finalize();

	uprintf("Finalize Successful");

	MPI_Initialized(&flag);
	uassert(flag);
	uprintf("Asserted initialization");

	MPI_Finalized(&flag);
	uassert(flag);
	uprintf("Asserted finalized");

	uprintf("Successful!");

	uprintf("---------------------------------------------");

	return (0);
}
