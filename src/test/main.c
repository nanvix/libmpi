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
#include <mpi.h>

/**
 * @brief Dummy test driver.
 */
int __main2(int argc, char *argv[])
{
	int flag;
	int rank, rank2;
	int size, size2;
	MPI_Group group;
	MPI_Errhandler errhandler;

	UNUSED(argc);
	UNUSED(argv);

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

	MPI_Comm_group(MPI_COMM_WORLD, &group);

	uassert(group != MPI_GROUP_EMPTY);

	uprintf("Group asserted");

	MPI_Group_rank(group, &rank);
	uassert(rank == cluster_get_num());

	uprintf("Group rank asserted");

	MPI_Comm_rank(MPI_COMM_WORLD, &rank2);
	uassert(rank2 == cluster_get_num());

	uprintf("Comm rank asserted");

	MPI_Group_size(group, &size);
	uassert(size == PROCESSOR_CLUSTERS_NUM);

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
