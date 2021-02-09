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

#include <mputil/proc.h>
#include <mputil/comm_request.h>

/**
 * @brief Defines the limit for the context ID.
 */
#define MPI_CONTEXT_LIMIT 32768

/**
 * @brief Defines the available communication modes for pt2pt communication.
 */
#define COMM_READY_MODE    0
#define COMM_BUFFERED_MODE 1
#define COMM_SYNC_MODE     2

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
 * negative error code is returned instead.
 */
extern int comm_context_init(void);

/**
 * @brief Finalizes the contexts submodule.
 *
 * @returns Upon successful completion, zero is returned. A
 * negative error code is returned instead.
 */
extern int comm_context_finalize(void);

/*============================================================================*
 * Comm Operations.                                                           *
 *============================================================================*/

/**
 * @todo Provide a detailed description.
 */
extern int send(int cid, const void *buf, size_t size, int src, mpi_process_t *dest, int datatype, int tag, int mode);

/**
 * @todo Provide a detailed description.
 */
extern int recv(int cid, void *buf, size_t size, mpi_process_t *src, int datatype, struct comm_request *req);

#endif /* NANVIX_COMM_CONTEXT_H_ */
