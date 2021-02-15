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

#include <nanvix/sys/mutex.h>
#include <nanvix/runtime/stdikc.h>
#include <mputil/proc.h>
#include <mputil/communication.h>
#include <mputil/comm_request.h>
#include <mpi/mpiruntime.h>
#include <mpi/errhandler.h>
#include <mpi/group.h>
#include <mpi/communicator.h>
#include <mpi/datatype.h>

/**
 * @brief Enable/Disable debug mode.
 */
#define DEBUG 0

/**
 * @brief Global MPI state variable.
 */
mpi_state_t _mpi_state = MPI_STATE_NOT_INITIALIZED;

struct nanvix_mutex _runtime_lock;

/**
 * @see mpi_init() at mpiruntime.h.
 */
PUBLIC int mpi_init(int argc, char **argv)
{
	int ret;

	UNUSED(argc);
	UNUSED(argv);

	/* The master thread exclusively executes the initialization. */
	if (!curr_proc_is_master())
		goto slave;

	/* Initializes runtime mutex. */
	uassert(nanvix_mutex_init(&_runtime_lock, NULL) == 0);

	/* Locks the runtime to evaluate mpi_state. */
	nanvix_mutex_lock(&_runtime_lock);

		/* Checks if this function was already called. */
		if (_mpi_state != MPI_STATE_NOT_INITIALIZED)
		{
			nanvix_mutex_unlock(&_runtime_lock);
			uprintf("ERROR!!! MPI_Init() called twice");
			return (MPI_ERR_OTHER);
		}

		_mpi_state = MPI_STATE_INIT_STARTED;

	nanvix_mutex_unlock(&_runtime_lock);

#if DEBUG
	uprintf("%s waiting in first fence...", process_name(curr_mpi_proc()));
#endif /* DEBUG */

	/* First local fence. */
	uassert(mpi_std_fence() == 0);

#if DEBUG
	uprintf("%s initializing local structures...", process_name(curr_mpi_proc()));
#endif /* DEBUG */

	/* Initialize the thread local structures. */
	if ((ret = mpi_local_proc_init()) != MPI_SUCCESS)
	{
		uprintf("ERROR!!! %s failed to initialize its local structures.",
				process_name(curr_mpi_proc()));
		return (ret);
	}

	/* Initialize MPI_Datatypes. */
	if ((ret = mpi_datatype_init()) != MPI_SUCCESS)
	{
		uprintf("ERROR!!! mpi_datatype_init() failed");
		goto end;
	}

	/* Initialize MPI_Ops. */

	/* Initialize Buffered Send component. */

	/* Initialize requests underlying module. */
	if ((ret = comm_request_init()) != MPI_SUCCESS)
	{
		uprintf("ERROR!!! comm_request_init() failed");
		goto end;
	}

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

	/* Initialize comm_contexts module. */
	if ((ret = comm_context_init()) != MPI_SUCCESS)
	{
		uprintf("ERROR!!! comm_context_init() failed");
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

	/* Locks the runtime to set the mpi_state again. */
	nanvix_mutex_lock(&_runtime_lock);

		_mpi_state = MPI_STATE_INITIALIZED;

	nanvix_mutex_unlock(&_runtime_lock);

#if DEBUG
	uprintf("%s waiting in last barrier...", process_name(curr_mpi_proc()));
#endif /* DEBUG */

	/* Barrier to ensure that everybody is at the same point in the initialization. */
	if ((ret = mpi_std_barrier()) != MPI_SUCCESS)
	{
		uprintf("ERROR!!! Could not ensure that all processes were initialized");
		goto end;
	}

#if DEBUG
	uprintf("MPI initialization completed...");
#endif

end:
	return (ret);

slave:
	/* Initialize std structures for spawned threads. */
	uassert(__stdmailbox_setup() == 0);
	uassert(__stdportal_setup() == 0);

#if DEBUG
	uprintf("%s waiting in first fence...", process_name(curr_mpi_proc()));
#endif /* DEBUG */

	/* First fence to ensure that local processes were correctly initialized. */
	uassert(mpi_std_fence() == 0);

#if DEBUG
	uprintf("%s initializing local structures...", process_name(curr_mpi_proc()));
#endif /* DEBUG */

	/* Initialize the thread local structures. */
	if ((ret = mpi_local_proc_init()) != MPI_SUCCESS)
	{
		uprintf("ERROR!!! %s failed to initialize its local structures.",
				process_name(curr_mpi_proc()));
		return (ret);
	}

#if DEBUG
	uprintf("%s waiting in last barrier...", process_name(curr_mpi_proc()));
#endif /* DEBUG */

	/* Barrier. */
	ret = mpi_std_barrier();

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

	/* Fence to ensure that all threads called finalize in the local cluster. */
	uassert(mpi_std_fence() == 0);

	/* The master thread exclusively executes the cleanup. */
	if (!curr_proc_is_master())
		goto slaves;

	/* Locks the runtime to assert the current mpi_state. */
	nanvix_mutex_lock(&_runtime_lock);

		/* Checks if this function was already called. */
		if (_mpi_state != MPI_STATE_INITIALIZED)
		{
			if (_mpi_state < MPI_STATE_INITIALIZED)
				uprintf("ERROR!!! MPI not initialized while calling MPI_Finalize()");
			else
				uprintf("ERROR!!! MPI_Finalize() called twice");

			nanvix_mutex_unlock(&_runtime_lock);

			return (MPI_ERR_OTHER);
		}

		_mpi_state = MPI_STATE_FINALIZE_STARTED;

	nanvix_mutex_unlock(&_runtime_lock);

	/* Destructs MPI_COMM_SELF. */
	if ((ret = mpi_destruct_comm_self()) != MPI_SUCCESS)
	{
		uprintf("ERROR!!! MPI_COMM_SELF could not be freed");
		goto end;
	}

	/* Locks the runtime to update mpi_state. */
	nanvix_mutex_lock(&_runtime_lock);

		/* MPI_COMM_SELF destructed. From this point, MPI_Finalized returns TRUE. */
		_mpi_state = MPI_STATE_FINALIZE_DESTRUCT_COMM_SELF;

	nanvix_mutex_unlock(&_runtime_lock);

#if DEBUG
	uprintf("%s waiting in finalize barrier", process_name(curr_mpi_proc()));
#endif

	/* Fence to ensure that everybody finalized communication. */
	if ((ret = mpi_std_barrier()) != MPI_SUCCESS)
	{
		uprintf("ERROR!!! Could not ensure that all processes were finalized");
		goto end;
	}

#if DEBUG
	uprintf("%s finalizing local structures", process_name(curr_mpi_proc()));
#endif

	/* Finalize the thread local structures. */
	if ((ret = mpi_local_proc_finalize()) != MPI_SUCCESS)
	{
		uprintf("ERROR!!! %s failed to finalize its local structures.",
				process_name(curr_mpi_proc()));
		return (ret);
	}

	/* Start to finalize what were initialized in mpi_init in the reverse order. */

	/* Finalize MPI_Communicators. */
	if ((ret = mpi_comm_finalize()) != MPI_SUCCESS)
	{
		uprintf("ERROR!!! mpi_comm_finalize() failed");
		goto end;
	}

	/* Finalize comm_contexts module. */
	if ((ret = comm_context_finalize()) != MPI_SUCCESS)
	{
		uprintf("ERROR!!! comm_context_finalize() failed");
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

	/* Finalize requests underlying module. */
	if ((ret = comm_request_finalize()) != MPI_SUCCESS)
	{
		uprintf("ERROR!!! comm_request_finalize() failed");
		goto end;
	}

	/* Finalize datatypes. */
	if ((ret = mpi_datatype_finalize()) != MPI_SUCCESS)
	{
		uprintf("ERROR!!! mpi_datatype_finalize() failed");
		goto end;
	}

#if DEBUG
	uprintf("%s waiting in last fence", process_name(curr_mpi_proc()));
#endif /* DEBUG */

	/* Last fence. */
	uassert(mpi_std_fence() == 0);

	/* Finalizes the mpi_state and returns. */
	nanvix_mutex_lock(&_runtime_lock);

		_mpi_state = MPI_STATE_FINALIZED;

	nanvix_mutex_unlock(&_runtime_lock);

#if DEBUG
	uprintf("MPI finalization completed");
#endif

end:
	return (ret);

slaves:
#if DEBUG
	uprintf("%s waiting in finalize barrier", process_name(curr_mpi_proc()));
#endif /* DEBUG */

	ret = mpi_std_barrier();

#if DEBUG
	uprintf("%s finalizing local structures", process_name(curr_mpi_proc()));
#endif /* DEBUG */

	/* Finalize the thread local structures. */
	if ((ret = mpi_local_proc_finalize()) != MPI_SUCCESS)
	{
		uprintf("ERROR!!! %s failed to finalize its local structures.",
				process_name(curr_mpi_proc()));
		return (ret);
	}

#if DEBUG
	uprintf("%s waiting in last fence", process_name(curr_mpi_proc()));
#endif /* DEBUG */

	/* Last fence. */
	uassert(mpi_std_fence() == 0);

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
