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

#ifndef NANVIX_COMM_REQUEST_H_
#define NANVIX_COMM_REQUEST_H_

/**
 * @brief Struct that defines a basic communication request.
 */
struct comm_request
{
	int cid;           /* Request context. */
	int src;           /* Source.          */
	int tag;           /* Message tag.     */
	int received_size; /* Received size.   */
};

/**
 * @brief Builds a new communication requisition.
 *
 * @param cid Request context.
 * @param src Request src.
 * @param tag Request tag.
 * @param req Request reference to be initialized.
 *
 * @todo Add a hash function to make requests comparation quicker.
 */
extern void comm_request_build(int cid, int src, int tag, struct comm_request *req);

/**
 * @brief Allocates a new request and registers it in the requisitions queue.
 *
 * @param req Requisition to be registered.
 *
 * @returns Upon successful completion, zero is returned. Upon failure, a negative
 * error code is returned instead.
 *
 * @todo Implement this function.
 *
 * @note This function needs to copy the req attributes in a safe structure, not only
 * storing its reference.
 */
extern int comm_request_register(struct comm_request *req);

/**
 * @brief Compares if two communication requisitions are equal.
 *
 * @param req1 First requisition to be compared.
 * @param req2 Second requisition.
 *
 * @returns Returns ZERO if the requisitions are different and a NON-ZERO value if
 * they match correctly.
 */
extern int comm_request_match(struct comm_request *req1, struct comm_request *req2);

/**
 * @brief Initializes the requests submodule.
 *
 * @returns Upon successful completion, zero is returned. A
 * negative MPI error code is returned instead.
 */
extern int comm_request_init(void);

/**
 * @brief Finalizes the requests submodule.
 *
 * @returns Upon successful completion, zero is returned. A
 * negative error code is returned instead.
 */
extern int comm_request_finalize(void);

#endif /* NANVIX_COMM_REQUEST_H_ */
