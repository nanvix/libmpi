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

#include <nanvix/ulib.h>
#include <mputil/ptr_array.h>
#include <mputil/proc.h>
#include <mputil/object.h>
#include <mpi/group.h>
#include <mpi.h>

/**
 * @brief Dummy test driver.
 */
int __main2(int argc, const char *argv[])
{
	mpi_process_t *local;
	mpi_group_t *group;
	UNUSED(argc);
	UNUSED(argv);

	uprintf("---------------------------------------------");

	uassert(mpi_proc_init() == 0);

	uprintf("Proc system initialized");

	local = process_local();

	uassert(process_nodenum(local) == cluster_get_num());

	uprintf("Asserted local process");

	uassert(mpi_group_init() == 0);

	uprintf("Group system initialized");

	group = mpi_group_allocate(2);

	uassert(group != MPI_GROUP_EMPTY);

	uprintf("Group asserted");

	mpi_group_free(&group);

	uassert(group == MPI_GROUP_NULL);

	uprintf("NULL Group asserted");

	group = mpi_group_allocate(0);

	uassert(group == MPI_GROUP_EMPTY);

	uprintf("EMPTY Group asserted");

	mpi_group_free(&group);

	uassert(mpi_group_finalize() == 0);

	uprintf("Group system finalized");

	uassert(mpi_proc_finalize() == 0);

	uprintf("Proc system finalized");

	uprintf("Successful!");

	uprintf("---------------------------------------------");

	return (0);
}
