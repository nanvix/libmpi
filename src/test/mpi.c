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

/*============================================================================*
 * API Test: Before Init Flags                                                *
 *============================================================================*/

/**
 * @brief API Test: Flags before initialization must be false 
 */
PRIVATE void test_mpi_before_init_flags(void)
{
	int flag;

	flag = 1;
	MPI_Initialized(&flag);
	uassert(!flag);

	flag = 1;
	MPI_Finalized(&flag);
	uassert(!flag);
}

/*============================================================================*
 * API Test: MPI_Init                                                         *
 *============================================================================*/

/**
 * @brief API Test: Initialization of MPI. 
 */
PRIVATE void test_mpi_init(void)
{
	uassert(MPI_Init(&_argc, &_argv) == 0);
}

/*============================================================================*
 * API Test: After Init Flags                                                 *
 *============================================================================*/

/**
 * @brief API Test: Flags after initalization must be true. 
 */
PRIVATE void test_mpi_after_init_flags(void)
{
	int flag;

	flag = 0;
	MPI_Initialized(&flag);
	uassert(flag);

	flag = 1;
	MPI_Finalized(&flag);
	uassert(!flag);
}

/*============================================================================*
 * API Test: Datatypes Size                                                   *
 *============================================================================*/

/**
 * @brief API Test: Assert datatype sizes. 
 */
PRIVATE void test_mpi_datatype_size(void)
{
	uassert(mpi_datatype_size(MPI_INT) == sizeof(int));

	uassert(mpi_datatype_size(MPI_DATATYPE_NULL) == 0);
}

/*============================================================================*
 * API Test: Group Functions                                                  *
 *============================================================================*/

/**
 * @brief API Test: Assert group functions. 
 */
PRIVATE void test_mpi_groups(void)
{
	int size, size2;
	int rank, rank2;
	MPI_Group group;
	MPI_Errhandler errhandler;

	MPI_Comm_group(MPI_COMM_WORLD, &group);
	uassert(group != MPI_GROUP_EMPTY);

	MPI_Group_rank(group, &rank);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank2);
	uassert(rank2 == rank);

	MPI_Group_size(group, &size);
	uassert(size == MPI_PROCESSES_NR);

	MPI_Comm_size(MPI_COMM_WORLD, &size2);
	uassert(size == size2);

	MPI_Group_free(&group);
	uassert(group == MPI_GROUP_NULL);

	MPI_Comm_get_errhandler(MPI_COMM_WORLD, &errhandler);
	uassert(errhandler == MPI_ERRORS_ARE_FATAL);

	/* Std fence to avoid errhandler changes to reflect in the other threads. */
	uassert(mpi_std_fence() == 0);

	MPI_Errhandler_free(&errhandler);
	MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_ABORT);
	MPI_Comm_get_errhandler(MPI_COMM_WORLD, &errhandler);
	uassert(errhandler == MPI_ERRORS_ABORT);

	MPI_Errhandler_free(&errhandler);
	uassert(errhandler == MPI_ERRHANDLER_NULL);
}

/*============================================================================*
 * API Test: Distinct Processes Ranks                                         *
 *============================================================================*/

/**
 * @brief Special structure to assert distinct ranks for distinct threads.
 */
PRIVATE int ranks_asserted[MPI_PROCS_PER_CLUSTER_MAX] = {
       [0 ... (MPI_PROCS_PER_CLUSTER_MAX - 1)] = 0
};

/**
 * @brief API Test: Distinct ranks for threads.
 */
PRIVATE void test_mpi_processes_ranks(void)
{
	int rank;
	int local_procs;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	/* Marks one initialization in ranks_asserted array. */
#if (__LWMPI_PROC_MAP == MPI_PROCESS_SCATTER)
	uassert((rank % MPI_NODES_NR) == (knode_get_num() - MPI_BASE_NODE));
#elif (__LWMPI_PROC_MAP == MPI_PROCESS_COMPACT)
	uassert((int) (rank / MPI_PROCS_PER_CLUSTER_MAX) == (knode_get_num() - MPI_BASE_NODE));
#endif

	uassert(ranks_asserted[curr_mpi_proc_index()] == 0);
	ranks_asserted[curr_mpi_proc_index()] = 1;

	/* Fence to ensure that all threads are at the same point. */
	uassert(mpi_std_fence() == 0);

	local_procs = 0;

	/* Verify if all local ranks completed this test. */
	for (int i = 0; i < MPI_PROCS_PER_CLUSTER_MAX; ++i)
	{
		if (ranks_asserted[i])
			local_procs++;
	}

	uassert(local_procs == mpi_local_procs_nr());
}

/*============================================================================*
 * API Test: Pairs Communication                                              *
 *============================================================================*/

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
#endif /* TEST_VERBOSE */

		uassert(MPI_Send(&outbuffer, 1, MPI_INT, remote, 0, MPI_COMM_WORLD) == MPI_SUCCESS);

#if TEST_VERBOSE
		uprintf("Rank %d Waiting for reply...", rank);
#endif /* TEST_VERBOSE */

		uassert(MPI_Recv(&inbuffer, 1, MPI_INT, remote, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE) == MPI_SUCCESS);

#if TEST_VERBOSE
		uprintf("Rank %d Received!", rank);
#endif /* TEST_VERBOSE */
	}
	else
	{
		remote = rank - 1;

#if TEST_VERBOSE
		uprintf("Rank %d waiting to receive from rank %d", rank, remote);
#endif /* TEST_VERBOSE */

		uassert(MPI_Recv(&inbuffer, 1, MPI_INT, remote, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE) == MPI_SUCCESS);

#if TEST_VERBOSE
		uprintf("Rank %d Sending reply...", rank);
#endif /* TEST_VERBOSE */

		uassert(MPI_Send(&outbuffer, 1, MPI_INT, remote, 0, MPI_COMM_WORLD) == MPI_SUCCESS);

#if TEST_VERBOSE
		uprintf("Rank %d Sent!", rank);
#endif /* TEST_VERBOSE */
	}

	uassert(inbuffer == remote);
}

/*============================================================================*
 * API Test: Requisitions Queue                                               *
 *============================================================================*/

/**
 * @brief API Test: Requisition queue.
 */
PRIVATE void test_mpi_comm_req_queue(void)
{
	int rank;
	int size;
	int remote;
	int inbuffer, outbuffer;

#if TEST_VERBOSE
	uprintf("--------------------------");

	/* Fence to synchronize the print outputs for all processes. */
	uassert(mpi_std_fence() == 0);
#endif /* TEST_VERBOSE */

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
#endif /* TEST_VERBOSE */

			uassert(MPI_Recv(&inbuffer, 1, MPI_INT, remote, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE) == MPI_SUCCESS);
			uassert(inbuffer == remote);

#if TEST_VERBOSE
			uprintf("Rank %d Received!", rank);
#endif /* TEST_VERBOSE */
		}

		/* Reads even ranks. */
		for (remote = 2; remote < size; remote += 2)
		{
#if TEST_VERBOSE
			uprintf("Rank %d waiting to receive from rank %d", rank, remote);
#endif /* TEST_VERBOSE */

			uassert(MPI_Recv(&inbuffer, 1, MPI_INT, remote, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE) == MPI_SUCCESS);
			uassert(inbuffer == remote);

#if TEST_VERBOSE
			uprintf("Rank %d Received!", rank);
#endif /* TEST_VERBOSE */
		}
	}
	else
	{
		remote = 0;

#if TEST_VERBOSE
		uprintf("Rank %d preparing to send to rank %d", rank, remote);
#endif /* TEST_VERBOSE */

		uassert(MPI_Send(&outbuffer, 1, MPI_INT, remote, 0, MPI_COMM_WORLD) == MPI_SUCCESS);

#if TEST_VERBOSE
		uprintf("Rank %d Sent!", rank);
#endif /* TEST_VERBOSE */
	}
}

/*============================================================================*
 * API Test: Broadcast Communication                                          *
 *============================================================================*/

/**
 * @brief API Test: Broadcast for all processes.
 */
PRIVATE void test_mpi_comm_broadcast(void)
{
	int rank;
	int inbuffer, outbuffer;

#if TEST_VERBOSE
	uprintf("--------------------------");

	/* Fence to synchronize the print outputs for all processes. */
	uassert(mpi_std_fence() == 0);
#endif /* TEST_VERBOSE */

	/* Communication test. */
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	/* Master process? */
	if (rank == 0)
	{
		/* Sends a message for everybody. */
		for (int i = 1; i < MPI_PROCESSES_NR; ++i)
		{
			outbuffer = i;

#if TEST_VERBOSE
			uprintf("Master preparing to send to rank %d", i);
#endif /* TEST_VERBOSE */

			uassert(MPI_Send(&outbuffer, 1, MPI_INT, i, 0, MPI_COMM_WORLD) == MPI_SUCCESS);

#if TEST_VERBOSE
		uprintf("Master Sent!");
#endif /* TEST_VERBOSE */
		}
	}
	else
	{
		inbuffer = (-1);

#if TEST_VERBOSE
		uprintf("Rank %d waiting to receive from master...", rank);
#endif /* TEST_VERBOSE */

		uassert(MPI_Recv(&inbuffer, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE) == MPI_SUCCESS);

		uassert(inbuffer == rank);

#if TEST_VERBOSE
		uprintf("Rank %d Received!", rank);
#endif /* TEST_VERBOSE */
	}
}

/*============================================================================*
 * API Test: Broadcast Communication                                          *
 *============================================================================*/

/**
 * @brief API Test: Broadcast for all processes.
 */
PRIVATE void test_mpi_comm_gather(void)
{
	int rank;
	int inbuffer, outbuffer;

#if TEST_VERBOSE
	uprintf("--------------------------");

	/* Fence to synchronize the print outputs for all processes. */
	uassert(mpi_std_fence() == 0);
#endif /* TEST_VERBOSE */

	/* Communication test. */
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	/* Master process? */
	if (rank != 0)
	{
		outbuffer = rank;

#if TEST_VERBOSE
		uprintf("Rank %d preparing to send to master...", rank);
#endif /* TEST_VERBOSE */

		uassert(MPI_Send(&outbuffer, 1, MPI_INT, 0, 0, MPI_COMM_WORLD) == MPI_SUCCESS);

#if TEST_VERBOSE
		uprintf("Rank %d Sent!", rank);
#endif /* TEST_VERBOSE */
	}
	else
	{
		for (int i = 1; i < MPI_PROCESSES_NR; ++i)
		{
			inbuffer = (-1);

#if TEST_VERBOSE
			uprintf("Master waiting to receive from rank %d...", i);
#endif /* TEST_VERBOSE */

			uassert(MPI_Recv(&inbuffer, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE) == MPI_SUCCESS);

			uassert(inbuffer == i);

#if TEST_VERBOSE
			uprintf("Master Received!");
#endif /* TEST_VERBOSE */
		}
	}
}

/*============================================================================*
 * API Test: Ping Pong Communication                                          *
 *============================================================================*/

/**
 * @brief API Test: Ping pong communication with all processes.
 */
PRIVATE void test_mpi_comm_ping_pong(void)
{
	int rank;
	int inbuffer, outbuffer;

#if TEST_VERBOSE
	uprintf("--------------------------");

	/* Fence to synchronize the print outputs for all processes. */
	uassert(mpi_std_fence() == 0);
#endif /* TEST_VERBOSE */

	/* Communication test. */
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	inbuffer = (-1);

	/* Master process? */
	if (rank == 0)
	{
		/* Sends a message for everybody. */
		for (int i = 1; i < MPI_PROCESSES_NR; ++i)
		{
			outbuffer = i;

#if TEST_VERBOSE
			uprintf("Master preparing to send to rank %d", i);
#endif /* TEST_VERBOSE */

			uassert(MPI_Send(&outbuffer, 1, MPI_INT, i, 0, MPI_COMM_WORLD) == MPI_SUCCESS);

#if TEST_VERBOSE
			uprintf("Waiting for reply...");
#endif /* TEST_VERBOSE */

			uassert(MPI_Recv(&inbuffer, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE) == MPI_SUCCESS);

#if TEST_VERBOSE
		uprintf("Received!");
#endif /* TEST_VERBOSE */

			uassert(inbuffer == i);

			inbuffer = (-1);
		}
	}
	else
	{
		outbuffer = rank;

#if TEST_VERBOSE
		uprintf("Rank %d waiting to receive from master...", rank);
#endif /* TEST_VERBOSE */

		uassert(MPI_Recv(&inbuffer, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE) == MPI_SUCCESS);

		uassert(inbuffer == rank);

#if TEST_VERBOSE
		uprintf("Rank %d Sending reply...", rank);
#endif /* TEST_VERBOSE */

		uassert(MPI_Send(&outbuffer, 1, MPI_INT, 0, 0, MPI_COMM_WORLD) == MPI_SUCCESS);

#if TEST_VERBOSE
		uprintf("Rank %d Sent!", rank);
#endif /* TEST_VERBOSE */
	}
}

/*============================================================================*
 * API Test: MPI_Finalize                                                     *
 *============================================================================*/

/**
 * @brief API Test: Finalize of MPI. 
 */
PRIVATE void test_mpi_finalize(void)
{
	uassert(MPI_Finalize() >= 0);
}

/*============================================================================*
 * API Test: After Finalize Flags                                             *
 *============================================================================*/

/**
 * @brief API Test: Flags after finalize. 
 */
PRIVATE void test_mpi_after_finalize_flags(void)
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
	{ test_mpi_before_init_flags,    "[test][mpi][init]     Flags before MPI_Init    [passed]" },
	{ test_mpi_init,                 "[test][mpi][init]     Initialization           [passed]" },
	{ test_mpi_after_init_flags,     "[test][mpi][init]     Flags after MPI_Init     [passed]" },
	{ test_mpi_datatype_size,        "[test][mpi][datatype] Datatype size            [passed]" },
	{ test_mpi_groups,               "[test][mpi][group]    Group functions          [passed]" },
	{ test_mpi_processes_ranks,      "[test][mpi][group]    Distinct Processes Ranks [passed]" },
	{ test_mpi_comm_pairs,           "[test][mpi][comm]     Pairs communication      [passed]" },
	{ test_mpi_comm_req_queue,       "[test][mpi][comm]     Requisition queue        [passed]" },
	{ test_mpi_comm_broadcast,       "[test][mpi][comm]     Broadcast                [passed]" },
	{ test_mpi_comm_gather,          "[test][mpi][comm]     Gather                   [passed]" },
	{ test_mpi_comm_ping_pong,       "[test][mpi][comm]     Ping Pong                [passed]" },
	{ test_mpi_finalize,             "[test][mpi][finalize] Finalization             [passed]" },
	{ test_mpi_after_finalize_flags, "[test][mpi][finalize] Flags after MPI_Finalize [passed]" },
	{ NULL,                           NULL                                                     },
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
	if ((nodenum == PROCESSOR_NODENUM_LEADER) && curr_proc_is_master())
		uprintf("--------------------------------------------------------------------------------");

	for (int i = 0; test_api_mpi[i].test_fn != NULL; i++)
	{
		test_api_mpi[i].test_fn();

		if ((nodenum == PROCESSOR_NODENUM_LEADER) && curr_proc_is_master())
        	uprintf(test_api_mpi[i].name);
	}

	/* API Tests */
	if ((nodenum == PROCESSOR_NODENUM_LEADER) && curr_proc_is_master())
		uprintf("--------------------------------------------------------------------------------");
}

