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
#include <nanvix/runtime/pm.h>
#include <mputil/comm_context.h>
#include <mputil/proc.h>

/**
 * @brief Defines the number of predefined contexts.
 *
 * @note This is a standard definition related to the MPI Spec, and SHOULD NOT CHANGE.
 */
#define MPI_CONTEXTS_PREDEFINED 3

/**
 * @brief Total number of available contexts.
 *
 * @brief This is a convenience macro and only exists to avoid the manual sum. 
 * IT SHOULD NEVER CHANGE.
 */
#define MPI_CONTEXTS_TOTAL (MPI_CONTEXTS_PREDEFINED + MPI_CONTEXTS_ALLOCATE_MAX)

/**
 * @brief Struct that defines a basic communication context.
 */
PRIVATE struct mpi_comm_context contexts[MPI_CONTEXTS_TOTAL] = {
	[0 ... (MPI_CONTEXTS_TOTAL - 1)] = {
		.port          = -1,
		.inbox         = -1,
		.inportal      = -1,
		.is_collective =  0,
	},
};

/**
 * @see comm_context_allocate in comm_context.h.
 *
 * @todo Implement this function like described in the header file.
 */
PUBLIC int comm_context_allocate(void)
{
	return (0);
}

/**
 * @see comm_context_init in comm_context.h.
 */
PUBLIC int comm_context_init(void)
{
	mpi_process_t *local_proc;
	const char *local_name;

	/* Runtime not initialized. */
	if ((local_proc = process_local()) == NULL)
		return (-EAGAIN);

	local_name = local_proc->name;

	/* Initializes the port number of each context. */
	for (int i = 0; i < MPI_CONTEXTS_TOTAL; ++i)
		contexts[i].port = (MPI_CONTEXT_BASE + i);

	/* Initializes the MPI_COMM_WORLD contexts. */
	for (int i = 0; i < MPI_CONTEXTS_PREDEFINED; ++i)
	{
		/* Initializes pt2pt context. */
		if ((contexts[i].inbox = nanvix_mailbox_create2(local_name, contexts[i].port)) < 0)
			return (-EAGAIN);

		/* Initializes collective context. */
		if ((contexts[i].inportal = nanvix_portal_create2(local_name, contexts[i].port)) < 0)
			return (-EAGAIN);
	}

	/* Initializes the MPI_COMM_WORLD collective context. */
	contexts[1].is_collective = 1;

	return (0);
}

/**
 * @see comm_context_finalize in comm_context.h.
 */
PUBLIC int comm_context_finalize(void)
{
	/* Unlinks the standard contexts inboxes/inportals. */
	for (int i = 0; i < MPI_CONTEXTS_PREDEFINED; ++i)
	{
		if (nanvix_mailbox_unlink(contexts[i].inbox) < 0)
			return (-EAGAIN);

		if (nanvix_portal_unlink(contexts[i].inportal) < 0)
			return (-EAGAIN);
	}

	/* Asserts that all allocated contexts were released. */
	for (int i = MPI_CONTEXTS_PREDEFINED; i < MPI_CONTEXTS_TOTAL; ++i)
	{
		uassert(contexts[i].inbox == -1);
		uassert(contexts[i].inportal == -1);
	}

	return (0);
}
