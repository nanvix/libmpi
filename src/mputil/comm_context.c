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
#include <nanvix/sys/mailbox.h>
#include <mputil/comm_context.h>
#include <mpi/datatype.h>
#include <mpi/mpi_errors.h>

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

/*============================================================================*
 * Message Operations.                                                        *
 *============================================================================*/

/**
 * @todo Provide a detailed description.
 */
PRIVATE void request_header_build(struct comm_message *m, uint16_t cid, uint16_t type,
	                              int size, uint32_t tag, uint8_t portal_port)
{
	uassert(m != NULL);

	m->req.src              = knode_get_num();
	m->req.cid              = cid;
	m->req.tag              = tag;
	m->msg.send.datatype    = type;
	m->msg.send.size        = size;
	m->msg.send.portal_port = portal_port;
}

/*============================================================================*
 * Context Operations.                                                        *
 *============================================================================*/

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

/*============================================================================*
 * Comm Operations.                                                           *
 *============================================================================*/

/*============================================================================*
 * nanvix_send()                                                              *
 *============================================================================*/

/**
 * @todo Provide a detailed description.
 *
 * @todo Implement this mode.
 */

PRIVATE int __rsend(int cid, const void *buf, size_t size, mpi_process_t *dest, int datatype, int tag)
{
	UNUSED(cid);
	UNUSED(buf);
	UNUSED(datatype);
	UNUSED(size);
	UNUSED(dest);
	UNUSED(tag);

	return (MPI_ERR_UNSUPPORTED_OPERATION);
}

/**
 * @todo Provide a detailed description.
 *
 * @todo Implement this mode.
 */
PRIVATE int __bsend(int cid, const void *buf, size_t size, mpi_process_t *dest, int datatype, int tag)
{
	UNUSED(cid);
	UNUSED(buf);
	UNUSED(datatype);
	UNUSED(size);
	UNUSED(dest);
	UNUSED(tag);

	return (MPI_ERR_UNSUPPORTED_OPERATION);
}

/**
 * @todo Provide a detailed description.
 */
PRIVATE int __ssend(int cid, const void *buf, size_t size, mpi_process_t *dest, int datatype, int tag)
{
	int ret;                     /**< Function return.              */
	int outbox;                  /**< Mailbox used to send request. */
	int outportal;               /**< Portal used to send data.     */
	int port;                    /**< Outportal allocated port.     */
	int remote;
	struct comm_message message; /**< Request message.              */
	const char *name = process_name(dest);

	ret = (MPI_ERR_UNKNOWN);

	/* Opens the outbox to send the request. */
	if ((outbox = nanvix_mailbox_open(name, contexts[cid].port)) < 0)
		goto ret0;

	/* Opens a new portal to send data. */
	if ((outportal = nanvix_portal_open(name, contexts[cid].port)) < 0)
		goto ret1;

	port = nanvix_portal_get_port(outportal);

	/* Constructs the message header. */
	request_header_build(&message, cid, datatype, size, tag, port);

	/* Sends a request to send to the receiver. */
	if ((ret = nanvix_mailbox_write(outbox, (const void*) &message, sizeof(struct comm_message))) < 0)
		goto ret2;

	/* Sends the message to the receiver. */
	if ((ret = nanvix_portal_write(outportal, buf, size)) < 0)
		goto ret2;

	/* Waits for recv ACK from the specified remote. */
	if ((remote = nanvix_name_lookup(name)) < 0)
		goto ret2;

	if ((ret = nanvix_mailbox_set_remote(contexts[cid].inbox, remote, MAILBOX_ANY_PORT)) < 0)
		goto ret2;

	if ((ret = nanvix_mailbox_read(contexts[cid].inbox, (void *) &message, sizeof(struct comm_message))) < 0)
		goto ret2;

	/* Confirms the ACK. */
	ret = message.msg.ret.errcode;

ret2:
	/* Closes the outportal. */
	if (nanvix_portal_close(outportal) < 0)
		ret = (MPI_ERR_UNKNOWN);

ret1:
	/* Closes the outbox. */
	if (nanvix_mailbox_close(outbox) < 0)
		ret = (MPI_ERR_UNKNOWN);

ret0:
	return (ret);
}

PUBLIC int send(int cid, const void *buf, size_t size, mpi_process_t *dest, int datatype, int tag, int mode)
{
	int ret;

	/* Bad context. */
	if (!WITHIN(cid, 0, MPI_CONTEXTS_TOTAL))
		return (MPI_ERR_INTERN);

	/* Point-to-point context? */
	if (contexts[cid].is_collective)
		return (MPI_ERR_INTERN);

	/* Mode selection. */
	switch (mode)
	{
		case COMM_READY_MODE:
			ret = __rsend(cid, buf, size, dest, datatype, tag);
			break;

		case COMM_BUFFERED_MODE:
			ret = __bsend(cid, buf, size, dest, datatype, tag);
			break;

		case COMM_SYNC_MODE:
			ret = __ssend(cid, buf, size, dest, datatype, tag);
			break;

		default:
			/* Bad mode. */
			ret = (MPI_ERR_ARG);
	}

	return (ret);
}

/*============================================================================*
 * nanvix_recv()                                                              *
 *============================================================================*/

PRIVATE int __recv(int cid, void *buf, size_t size, mpi_process_t *src, int datatype, struct comm_request *req)
{
	int ret;
	int overflow;
	int outbox;                    /**< Output mailbox.       */
	struct comm_message message;   /**< Received message.     */
	struct comm_message reply;     /**< Requisition reply.    */

again:
	comm_request_build(req->cid, req->src, req->tag, &message.req);

	/* Search for a previous requisition. */
	if (comm_request_search(&message))
		goto found;

	/* Waits for a send request. */
	if (nanvix_mailbox_read(contexts[cid].inbox, &message, sizeof(struct comm_message)) < 0)
		return (MPI_ERR_UNKNOWN);

	/* Checks if the expected and received requests match. */
	if (!comm_request_match(req, &message.req))
	{
		/* Error when enqueue requisition. */
		if (comm_request_register(&message) != 0)
			return (MPI_ERR_INTERN);

		/* TODO: may another thread have enqueued another request? */
		goto again;
	}

found:
	/* Checks the other information that came in the message. */
	if (!mpi_datatypes_match(datatype, message.msg.send.datatype))
		return (MPI_ERR_TYPE);

	/* Allows the remote to send data. */
	if (nanvix_portal_allow2(contexts[cid].inportal, message.req.src, message.msg.send.portal_port) < 0)
		return (MPI_ERR_INTERN);

	overflow = 0;

	/* Checks the amount of data to be received. */
	if (size < message.msg.send.size)
	{
		req->received_size = size;
		overflow = 1;
	}
	else
		req->received_size = message.msg.send.size;

	/* Receives data. */
	if (nanvix_portal_read(contexts[cid].inportal, buf, req->received_size) < 0)
	{
		req->received_size = 0;
		return (MPI_ERR_INTERN);
	}

	/* Opens a mailbox to send the ACK. */
	if ((outbox = nanvix_mailbox_open(process_name(src), contexts[cid].port)) < 0)
		return (MPI_ERR_UNKNOWN);

	ret = MPI_SUCCESS;

	if (overflow)
		ret = MPI_ERR_OTHER;

	/* Builds the return message. */
	reply.req.cid         = cid;
	reply.req.src         = knode_get_num();
	reply.msg.ret.errcode = ret;

	/* Sends an ACK message. */
	if (nanvix_mailbox_write(outbox, (const void *) &reply, sizeof(struct comm_message)) < 0)
		ret = MPI_ERR_INTERN;

	uassert(nanvix_mailbox_close(outbox) == 0);

	return (ret);
}

PUBLIC int recv(int cid, void *buf, size_t size, mpi_process_t *src, int datatype, struct comm_request *req)
{
	int ret;

	/* Bad context. */
	if (!WITHIN(cid, 0, MPI_CONTEXTS_TOTAL))
		return (MPI_ERR_INTERN);

	/* Point-to-point context? */
	if (contexts[cid].is_collective)
		return (MPI_ERR_INTERN);

	/* Bad request holder. */
	if (req == NULL)
		return (MPI_ERR_INTERN);

	ret = __recv(cid, buf, size, src, datatype, req);

	return (ret);
}

