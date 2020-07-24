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

#ifndef NANVIX_MPI_PT2PT_COMM_H_
#define NANVIX_MPI_PT2PT_COMM_H_

#include <mpi.h>

/**
 * @brief MPI Point-to-point communication modes.
 *
 * @note This enumeration should be in accord with the underlying constants
 * defined in mputil/comm_context.h whose defines the communication protocols
 * available in the underlying levels.
 */
typedef enum {
	READY_MODE,
	BUFFERED_MODE,
	SYNC_MODE
} mpi_comm_mode_t;

/**
 * @brief Provide detailed description.
 */
extern int mpi_send(const void *buf, int count, MPI_Datatype datatype, int dest,
	                int tag, MPI_Comm comm, mpi_comm_mode_t mode);

/**
 * @brief Provide detailed description.
 */
extern int mpi_recv(void *buf, int count, MPI_Datatype datatype, int source,
	                int tag, MPI_Comm comm, MPI_Status *status);

#endif /* NANVIX_MPI_PT2PT_COMM_H_ */
