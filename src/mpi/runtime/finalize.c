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

#include <mpi/mpiruntime.h>
#include <mpi/errhandler.h>
#include <mpi.h>

static const char FUNC_NAME[] = "MPI_Finalize";

/**
 * @brief Cleans all MPI state and finalizes the MPI environment.
 *
 * @returns Upon successful completion, MPI_SUCCESS is returned.
 * Upon failure, a MPI error code is returned and the environment
 * aborted.
 *
 * @note This function is collective over all connected processes, and each
 * process in the job should call this function once.
 *
 * @note Before calling this function, the caller process should have finished
 * all communications where it participates.
 */
PUBLIC int MPI_Finalize(void)
{
	/* Checks if MPI is in a valid state. */
	MPI_CHECK_INIT_FINALIZE(FUNC_NAME);

	return (mpi_finalize());
}
