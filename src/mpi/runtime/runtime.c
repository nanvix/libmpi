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
#include <mputil/proc.h>
#include <mpi/mpiruntime.h>
#include <mpi/errhandler.h>
#include <mpi/group.h>
#include <mpi/communicator.h>

/**
 * @brief Global MPI state variable.
 */
mpi_state_t _mpi_state = MPI_STATE_NOT_INITIALIZED;

spinlock_t _runtime_lock;

/**
 * @see mpi_init() at mpiruntime.h.
 */
PUBLIC int mpi_init(int argc, char **argv)
{
	int ret;

	UNUSED(argc);
	UNUSED(argv);

	/* Locks the runtime to evaluate mpi_state. */
	spinlock_lock(&_runtime_lock);

		/* Checks if this function was already called. */
		if (_mpi_state != MPI_STATE_NOT_INITIALIZED)
		{
			spinlock_unlock(&_runtime_lock);
			uprintf("ERROR!!! MPI_Init() called twice");
			return (MPI_ERR_OTHER);
		}

		_mpi_state = MPI_STATE_INIT_STARTED;

	spinlock_unlock(&_runtime_lock);

	/* Initialize MPI_Datatypes. */

	/* Initializes processes. */
	if ((ret = mpi_proc_init()) != MPI_SUCCESS)
	{
		uprintf("ERROR!!! mpi_proc_init() failed");
		goto end;
	}

	/* Initialize MPI_Ops. */

	/* Initialize Buffered Send component. */

	/* Initialize MPI_Requests. */

	/* Initialize MPI_Messages. */

	/* Initialize MPI_Info. */

	/* Initialize MPI_Errhandlers. */
	if ((ret = mpi_errhandler_init()) != MPI_SUCCESS)
	{
		uprintf("ERROR!!! mpi_errhandler_init() failed");
		goto end;
	}

	/* Initialize errorcodes. */

	/* Initialize MPI_Groups. */
	if ((ret = mpi_group_init()) != MPI_SUCCESS)
	{
		uprintf("ERROR!!! mpi_group_init() failed");
		goto end;
	}

	/* Initialize MPI_Communicators. */
	if ((ret = mpi_comm_init()) != MPI_SUCCESS)
	{
		uprintf("ERROR!!! mpi_comm_init() failed");
		goto end;
	}

	/* Initialize MPI_Files. */

	/* Initialize MPI_Windows. */

	/* Initialize MPI_Attributes. */

	/* Include a barrier here? */

	/* Locks the runtime to set the mpi_state again. */
	spinlock_lock(&_runtime_lock);

		_mpi_state = MPI_STATE_INITIALIZED;

	spinlock_unlock(&_runtime_lock);

end:
	return (ret);
}

/**
 * @see mpi_finalize() at mpiruntime.h.
 *
 * @todo Change the static fence call by a non-blocking version.
 */
PUBLIC int mpi_finalize(void)
{
	int ret;

	/* Locks the runtime to assert the current mpi_state. */
	spinlock_lock(&_runtime_lock);

		/* Checks if this function was already called. */
		if (_mpi_state != MPI_STATE_INITIALIZED)
		{
			if (_mpi_state < MPI_STATE_INITIALIZED)
				uprintf("ERROR!!! MPI not initialized while calling MPI_Finalize()");
			else
				uprintf("ERROR!!! MPI_Finalize() called twice");

			spinlock_unlock(&_runtime_lock);

			return (MPI_ERR_OTHER);
		}

		_mpi_state = MPI_STATE_FINALIZE_STARTED;

	spinlock_unlock(&_runtime_lock);

	/* Destructs MPI_COMM_SELF. */
	if ((ret = mpi_destruct_comm_self()) != MPI_SUCCESS)
	{
		uprintf("ERROR!!! MPI_COMM_SELF could not be freed");
		goto end;
	}

	/* Locks the runtime to update mpi_state. */
	spinlock_lock(&_runtime_lock);

		/* MPI_COMM_SELF destructed. From this point, MPI_Finalized returns TRUE. */
		_mpi_state = MPI_STATE_FINALIZE_DESTRUCT_COMM_SELF;

	spinlock_unlock(&_runtime_lock);

	/* Fence to ensure that everybody finalized communication. */
	/* @todo Uncomment when spawning all clusters properly. */
#if 0
	if ((ret = mpi_std_fence()) != MPI_SUCCESS)
	{
		uprintf("ERROR!!! Could not ensure that all processes were finalized");
		goto end;
	}
#endif

	/* Start to finalize what were initialized in mpi_init in the reverse order. */

	/* Finalize MPI_Communicators. */
	if ((ret = mpi_comm_finalize()) != MPI_SUCCESS)
	{
		uprintf("ERROR!!! mpi_comm_finalize() failed");
		goto end;
	}

	/* Finalize MPI_Groups. */
	if ((ret = mpi_group_finalize()) != MPI_SUCCESS)
	{
		uprintf("ERROR!!! mpi_group_finalize() failed");
		goto end;
	}

	/* Finalize MPI_Errhandlers. */
	if ((ret = mpi_errhandler_finalize()) != MPI_SUCCESS)
	{
		uprintf("ERROR!!! mpi_errhandler_finalize() failed");
		goto end;
	}

	/* Finalize processes. */
	if ((ret = mpi_proc_finalize()) != MPI_SUCCESS)
	{
		uprintf("ERROR!!! mpi_proc_finalize() failed");
		goto end;
	}

end:
	/* Finalizes the mpi_state and returns. */
	spinlock_lock(&_runtime_lock);

		_mpi_state = MPI_STATE_FINALIZED;

	spinlock_unlock(&_runtime_lock);

	return (ret);
}

/**
 * @see mpi_abort() at mpiruntime.h.
 */
PUBLIC int mpi_abort(mpi_communicator_t *comm, int errorcode)
{
	UNUSED(comm);
	UNUSED(errorcode);

	return (MPI_SUCCESS);
}
