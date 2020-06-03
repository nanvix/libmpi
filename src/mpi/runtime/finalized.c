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
#include <mpi/communicator.h>
#include <mpi.h>

static const char FUNC_NAME[] = "MPI_Initialized";

/**
 * @brief Determines whether MPI_Finalized() was already called or not.
 *
 * @param flag Flag where the result will be stored.
 *
 * @returns Upon successful completion, MPI_SUCCESS is returned. An MPI errorcode
 * is returned instead.
 *
 * @note The result at @p flag will be TRUE, in the case where MPI_Finalize() was
 * already called by the callig process, and FALSE otherwise.
 *
 * @note This function is one of the few routines that can be called outside
 * the MPI_Init / MPI_Finalize boundary.
 *
 * @todo This function must always be thread-safe. No concurrent functions can
 * progress while this one is being called.
 */
PUBLIC int MPI_Finalized(int *flag)
{
	int state;

	state = _mpi_state;

	/* Bad flag. */
	if (flag == NULL)
	{
		/* Checks where the execution is to correctly invoke the errhandler. */
		if ((state < MPI_STATE_INIT_STARTED) || (state > MPI_STATE_FINALIZE_STARTED))
			return (mpi_errhandler_invoke(NULL, NULL, -1, MPI_ERR_ARG, FUNC_NAME));
		else
			return (MPI_ERRHANDLER_INVOKE(MPI_COMM_WORLD, MPI_ERR_ARG, FUNC_NAME));
	}

	*flag = (state >= MPI_STATE_FINALIZE_DESTRUCT_COMM_SELF);

	return (MPI_SUCCESS);
}
