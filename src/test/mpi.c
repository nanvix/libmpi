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

#include <nanvix/sys/noc.h>
#include <nanvix/ulib.h>
#include <nanvix/config.h>
#include <nanvix/limits.h>
#include <mputil/proc.h>
#include <mpi.h>

#include <mpi/datatype.h>

#include "test.h"

/**
 * @brief Launch verbose tests?
 */
#define TEST_VERBOSE 0

/*============================================================================*
 * API Tests                                                                  *
 *============================================================================*/

/**
 * @brief API Test: Flags before initialization must be false 
 */
PRIVATE void test_mpi_before_init(void)
{
	int flag;

	flag = 1;
	MPI_Initialized(&flag);
	uassert(!flag);

	flag = 1;
	MPI_Finalized(&flag);
	uassert(!flag);
}

/**
 * @brief API Test: Initialization of MPI. 
 */
PRIVATE void test_mpi_init(void)
{
	uassert(MPI_Init(&_argc, &_argv) == 0);
}

/**
 * @brief API Test: Flags after initalization must be true. 
 */
PRIVATE void test_mpi_after_init(void)
{
	int flag;

	flag = 0;
	MPI_Initialized(&flag);
	uassert(flag);

	flag = 1;
	MPI_Finalized(&flag);
	uassert(!flag);
}

/**
 * @brief API Test: Assert datatype sizes. 
 */
PRIVATE void test_mpi_datatype_size(void)
{
	uassert(mpi_datatype_size(MPI_INT) == sizeof(int));

	uassert(mpi_datatype_size(MPI_DATATYPE_NULL) == 0);
}

/**
 * @brief API Test: Assert group functions. 
 */
PRIVATE void test_mpi_groups(void)
{
	int size, size2;
	int rank, rank2;
	int expected_rank;

	expected_rank = (cluster_get_num() - SPAWNERS_NUM);

	MPI_Group group;
	MPI_Errhandler errhandler;

	MPI_Comm_group(MPI_COMM_WORLD, &group);
	uassert(group != MPI_GROUP_EMPTY);

	MPI_Group_rank(group, &rank);
	uassert(rank == expected_rank);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank2);
	uassert(rank2 == expected_rank);

	MPI_Group_size(group, &size);
	uassert(size == MPI_PROCESSES_NR);

	MPI_Comm_size(MPI_COMM_WORLD, &size2);
	uassert(size == size2);

	MPI_Group_free(&group);
	uassert(group == MPI_GROUP_NULL);

	MPI_Comm_get_errhandler(MPI_COMM_WORLD, &errhandler);
	uassert(errhandler == MPI_ERRORS_ARE_FATAL);

	MPI_Errhandler_free(&errhandler);
	MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_ABORT);
	MPI_Comm_get_errhandler(MPI_COMM_WORLD, &errhandler);
	uassert(errhandler == MPI_ERRORS_ABORT);

	MPI_Errhandler_free(&errhandler);
	uassert(errhandler == MPI_ERRHANDLER_NULL);
}

/**
 * @brief API Test: Pair communication.
 */
PRIVATE void test_mpi_comm_pairs(void)
{
	int rank;
	int remote;
	int inbuffer, outbuffer;

	/* Communication test. */
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	outbuffer = rank;
	inbuffer  = (-1);

	/* Pairs communication. */
	if ((rank % 2) == 0)
	{
		remote = rank + 1;

#if TEST_VERBOSE
		uprintf("Rank %d preparing to send to rank %d", rank, remote);
#endif

		uassert(MPI_Send(&outbuffer, 1, MPI_INT, remote, 0, MPI_COMM_WORLD) == MPI_SUCCESS);

#if TEST_VERBOSE
		uprintf("Waiting for reply...");
#endif

		uassert(MPI_Recv(&inbuffer, 1, MPI_INT, remote, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE) == MPI_SUCCESS);

#if TEST_VERBOSE
		uprintf("Received!");
#endif
	}
	else
	{
		remote = rank - 1;

#if TEST_VERBOSE
		uprintf("Rank %d waiting to receive from rank %d", rank, remote);
#endif

		uassert(MPI_Recv(&inbuffer, 1, MPI_INT, remote, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE) == MPI_SUCCESS);

#if TEST_VERBOSE
		uprintf("Sending reply...");
#endif

		uassert(MPI_Send(&outbuffer, 1, MPI_INT, remote, 0, MPI_COMM_WORLD) == MPI_SUCCESS);

#if TEST_VERBOSE
		uprintf("Sent!");
#endif
	}

	uassert(inbuffer == remote);

#if TEST_VERBOSE
	if (rank == 0)
		uprintf("Pairs Communication done!");
#endif
}

/**
 * @brief API Test: Requisition queue.
 */
PRIVATE void test_mpi_comm_req_queue(void)
{
	int rank;
	int size;
	int remote;
	int inbuffer, outbuffer;

	/* Communication test. */
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	outbuffer = rank;
	inbuffer  = (-1);

	/* Gather communication. */
	if (rank == 0)
	{
		/* Communication test. */
		MPI_Comm_size(MPI_COMM_WORLD, &size);

		/* Reads odd ranks. */
		for (remote = 1; remote < size; remote += 2)
		{
			inbuffer = 0;

#if TEST_VERBOSE
			uprintf("Rank %d waiting to receive from rank %d", rank, remote);
#endif

			uassert(MPI_Recv(&inbuffer, 1, MPI_INT, remote, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE) == MPI_SUCCESS);
			uassert(inbuffer == remote);

#if TEST_VERBOSE
			uprintf("Received!");
#endif
		}

		/* Reads even ranks. */
		for (remote = 2; remote < size; remote += 2)
		{
#if TEST_VERBOSE
			uprintf("Rank %d waiting to receive from rank %d", rank, remote);
#endif

			uassert(MPI_Recv(&inbuffer, 1, MPI_INT, remote, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE) == MPI_SUCCESS);
			uassert(inbuffer == remote);

#if TEST_VERBOSE
			uprintf("Received!");
#endif
		}
	}
	else
	{
		remote = 0;

#if TEST_VERBOSE
		uprintf("Rank %d preparing to send to rank %d", rank, remote);
#endif

		uassert(MPI_Send(&outbuffer, 1, MPI_INT, remote, 0, MPI_COMM_WORLD) == MPI_SUCCESS);

#if TEST_VERBOSE
		uprintf("Sent!");
#endif
	}

#if TEST_VERBOSE
	if (rank == 0)
		uprintf("Requisition Queue done!");
#endif
}

/**
 * @brief API Test: Finalize of MPI. 
 */
PRIVATE void test_mpi_finalize(void)
{
	uassert(MPI_Finalize() >= 0);
}

/**
 * @brief API Test: Flags after finalize. 
 */
PRIVATE void test_mpi_after_finalize(void)
{
	int flag;

	flag = 0;
	MPI_Initialized(&flag);
	uassert(flag);

	flag = 0;
	MPI_Finalized(&flag);
	uassert(flag);
}

/*============================================================================*
 * Test Driver                                                                *
 *============================================================================*/

/**
 * @brief API Tests.
 */
PRIVATE struct test test_api_mpi[] = {
	{ test_mpi_before_init,    "[test][mpi][init]     Flags before MPI_Init       [passed]" },
	{ test_mpi_init,           "[test][mpi][init]     Initialization              [passed]" },
	{ test_mpi_after_init,     "[test][mpi][init]     Flags after MPI_Init        [passed]" },
	{ test_mpi_datatype_size,  "[test][mpi][datatype] Datatype size               [passed]" },
	{ test_mpi_groups,         "[test][mpi][group]    Group functions             [passed]" },
	{ test_mpi_comm_pairs,     "[test][mpi][comm]     Communication between pairs [passed]" },
	{ test_mpi_comm_req_queue, "[test][mpi][comm]     Requisition queue           [passed]" },
	{ test_mpi_finalize,       "[test][mpi][finish]   Finalization                [passed]" },
	{ test_mpi_after_finalize, "[test][mpi][finish]   Flags after MPI_Finalize    [passed]" },
	{ NULL,                    NULL                                                         },
};

/**
 * The test_mpi() function launches regression tests on the mpi
 * Interface of the Processor Abstraction Layer.
 *
 * @author Pedro Henrique Penna
 * @author João Vicente Souto
 */
PUBLIC void test_mpi(void)
{
	int nodenum;

	nodenum = knode_get_num();

	/* API Tests */
	if (nodenum == PROCESSOR_NODENUM_LEADER)
		uprintf("--------------------------------------------------------------------------------");

	for (int i = 0; test_api_mpi[i].test_fn != NULL; i++)
	{
		test_api_mpi[i].test_fn();

		if (nodenum == PROCESSOR_NODENUM_LEADER)
        	uprintf(test_api_mpi[i].name);
	}

	/* API Tests */
	if (nodenum == PROCESSOR_NODENUM_LEADER)
		uprintf("--------------------------------------------------------------------------------");
}

