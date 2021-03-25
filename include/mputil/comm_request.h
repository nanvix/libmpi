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

#include <nanvix/kernel/mailbox.h>

/**
 * @brief Predefined port number that is used to receive communication requests.
 */
#define COMM_REQ_RECV_PORT (KMAILBOX_PORT_NR - 1)

/**
 * @brief Struct that defines a basic communication request.
 */
struct comm_request
{
	int16_t cid;           /**< Request context. */
	int16_t src;           /**< Source rank.     */
	int16_t target;        /**< Target rank.     */
	int32_t tag;           /**< Message tag.     */
	int32_t received_size; /**< Received size.   */
};

/**
 * @brief Struct that defines a message to establish communication.
 */
struct comm_message
{
	struct comm_request req; /**< Request information.  */

	union
	{
		struct
		{
			uint16_t datatype;   /**< Datatype.          */
			size_t size;         /**< Message size.      */
			uint8_t portal_port; /**< Port Number.       */
			uint8_t inbox_port;  /**< Inbox Port Number. */
			uint8_t nodenum;     /**< Node Number.       */
		} send;

		struct
		{
			uint8_t mailbox_port; /**< Outbox port_nr. */
		} confirm;

		struct
		{
			int errcode; /**< Function return. */
		} ret;
	} msg;
};

/**
 * @brief Builds a new communication requisition.
 *
 * @param cid    Request context.
 * @param src    Request src.
 * @param target Request target.
 * @param tag    Request tag.
 * @param req    Request reference to be initialized.
 *
 * @todo Add a hash function to make requests comparation quicker.
 */
extern void comm_request_build(int cid, int src, int target, int tag, struct comm_request *req);

/**
 * @brief Search into Request Queue.
 *
 * @param msg Message with a valid request information.
 *
 * @returns If a matched request is found, consume it and returns non-zero
 * value, zero, otherwise.
 */
extern int comm_request_search(struct comm_message *msg);

/**
 * @brief Receives a new communication request from the IKC facility.
 *
 * @param msg Message holder with a valid request information.
 *
 * @returns Upon successful completion, zero is returned with the received message
 * copied into @p msg. Upon failure, a negative error code is returned instead.
 */
extern int comm_request_receive(struct comm_message *msg);

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
