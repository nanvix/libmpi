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

static const char FUNC_NAME[] = "MPI_Init";

/**
 * @brief Initializes the MPI environment.
 *
 * @param argc Number of arguments in @p argv.
 * @param argv Program arguments.
 *
 * @returns Upon successful completion, MPI_SUCCESS is returned.
 * Upon failure, a MPI error code is returned and the environment
 * aborted.
 *
 * @note Initialization functions like MPI_Init or MPI_Init_thread
 * can be called once and exclusively in a MPI execution. All
 * subsequent calls will be erroneous.
 */
PUBLIC int MPI_Init(int *argc, char ***argv)
{
	int ret;

	/* Call the backend initialization function. */
	if ((argc != NULL) && (argv != NULL))
		ret = mpi_init(*argc, *argv);
	else
		ret = mpi_init(0, NULL);

	/* Checks if an error occured during initialization. */
	if (ret != MPI_SUCCESS)
	{
		/* Calls the default error handler to abort execution. */
		return (mpi_errhandler_invoke(NULL, NULL, MPI_ERRHANDLER_TYPE_COMM,
		                              ret, FUNC_NAME)
		       );
	}

	return (MPI_SUCCESS);
}
