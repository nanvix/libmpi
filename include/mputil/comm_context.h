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

#ifndef NANVIX_COMM_CONTEXT_H_
#define NANVIX_COMM_CONTEXT_H_

#include <nanvix/limits.h>

/**
 * @brief Defines the first context ID available for MPI communicators.
 *
 * @note Constant defined in nanvix/limits/pm.h.
 */
#define MPI_CONTEXT_BASE NANVIX_GENERAL_PORTS_BASE

/**
 * @brief Defines the max number of contexts to be allocated (excluding predefined).
 */
#define MPI_CONTEXTS_ALLOCATE_MAX 0

/**
 * @brief Struct that defines a basic communication context.
 */
struct mpi_comm_context
{
	int port;              /* Context port number. */
	int inbox;             /* Inbox ID.            */
	int inportal;          /* Inportal ID.         */
	uint8_t is_collective; /* Collective context?  */
};

/**
 * @brief Allocates a context and propagates it through the other processes.
 *
 * @returns An integer relative to the newly allocated context ID.
 *
 * @todo Implement this function when new communicators creation become available.
 * It may be necessary the implementation of a consensus protocol here, based on the
 * MPI_COMM_WORLD.
 */
extern int comm_context_allocate(void);

/**
 * @brief Initializes the contexts submodule.
 *
 * @returns Upon successful completion, zero is returned. A
 * negative MPI error code is returned instead.
 */
extern int comm_context_init(void);

/**
 * @brief Finalizes the contexts submodule.
 *
 * @returns Upon successful completion, zero is returned. A
 * negative error code is returned instead.
 */
extern int comm_context_finalize(void);

#endif /* NANVIX_COMM_CONTEXT_H_ */
