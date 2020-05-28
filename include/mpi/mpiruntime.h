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

#ifndef NANVIX_MPI_RUNTIME_H_
#define NANVIX_MPI_RUNTIME_H_

#include <mpi.h>

/**
 * @note Forward struct declaration to avoid circular references.
 */
struct mpi_communicator_t;

/**
 * @brief MPI runtime possible states enum.
 */
typedef enum {
	MPI_STATE_NOT_INITIALIZED = 0,
	MPI_STATE_INIT_STARTED,
	MPI_STATE_INITIALIZED,
	MPI_STATE_FINALIZE_STARTED,
	MPI_STATE_FINALIZED
} mpi_state_t;

/**
 * @brief Global MPI state variable.
 */
extern mpi_state_t _mpi_state;

/**
 * @brief Initializes the MPI runtime.
 *
 * @returns Upon successful completion, MPI_SUCCESS is returned. A
 * negative error code is returned instead.
 */
extern int mpi_init(void);

/**
 * @brief Finalizes the MPI runtime.
 *
 * @returns Upon successful completion, MPI_SUCCESS is returned. A
 * negative error code is returned instead.
 */
extern int mpi_finalize(void);

/**
 * @brief Aborts the MPI runtime.
 *
 * @param comm     Communicator that have the group of processes to be aborted.
 * @param errocode Errorcode to be returned to the caller environment.
 *
 * @returns Upon successful completion, MPI_SUCCESS is returned. A
 * negative error code is returned instead.
 */
extern int mpi_abort(struct mpi_communicator_t *comm, int errorcode);

#endif /* NANVIX_MPI_RUNTIME_H_ */
