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
#include <mputil/communication.h>
#include <mpi/datatype.h>
#include <mpi/mpi_errors.h>

/**
 * @brief Enables debug verbose mode.
 */
#define DEBUG 0

/**
 * @brief Defines the first free context, available to be allocated for a
 * new communicator.
 *
 * @note INITIAL VALUE: 3
 * It is defined considering the default contexts used by the predefined
 * communicators.
 */
PRIVATE int16_t _first_free_context = 3;

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
 * @see comm_context_allocate in communication.h.
 *
 * @todo Implement this function like described in the header file.
 */
PUBLIC int comm_context_allocate(void)
{
	return (_first_free_context);
}

/**
 * @see comm_context_init in communication.h.
 */
PUBLIC int comm_context_init(void)
{
	/* @todo SEE IF THIS FUNCTION STILL NECESSARY. */

	// mpi_process_t *local_proc;
	// const char *local_name;

	// /* Runtime not initialized. */
	// if ((local_proc = curr_mpi_proc()) == NULL)
	// 	return (-EAGAIN);

	// local_name = local_proc->name;

	// /* Initializes the port number of each context. */
	// for (int i = 0; i < MPI_CONTEXTS_TOTAL; ++i)
	// 	contexts[i].port = (MPI_CONTEXT_BASE + i);

	// /* Initializes the MPI_COMM_WORLD contexts. */
	// for (int i = 0; i < MPI_CONTEXTS_PREDEFINED; ++i)
	// {
	// 	/* Initializes pt2pt context. */
	// 	if ((contexts[i].inbox = nanvix_mailbox_create2(local_name, contexts[i].port)) < 0)
	// 		return (-EAGAIN);

	// 	/* Initializes collective context. */
	// 	if ((contexts[i].inportal = nanvix_portal_create2(local_name, contexts[i].port)) < 0)
	// 		return (-EAGAIN);
	// }

	// /* Initializes the MPI_COMM_WORLD collective context. */
	// contexts[1].is_collective = 1;

	return (0);
}

/**
 * @see comm_context_finalize in communication.h.
 */
PUBLIC int comm_context_finalize(void)
{
	/* @todo SEE IF THIS FUNCTION STILL NECESSARY. */

	// /* Unlinks the standard contexts inboxes/inportals. */
	// for (int i = 0; i < MPI_CONTEXTS_PREDEFINED; ++i)
	// {
	// 	if (nanvix_mailbox_unlink(contexts[i].inbox) < 0)
	// 		return (-EAGAIN);

	// 	if (nanvix_portal_unlink(contexts[i].inportal) < 0)
	// 		return (-EAGAIN);
	// }

	// /* Asserts that all allocated contexts were released. */
	// for (int i = MPI_CONTEXTS_PREDEFINED; i < MPI_CONTEXTS_TOTAL; ++i)
	// {
	// 	uassert(contexts[i].inbox == -1);
	// 	uassert(contexts[i].inportal == -1);
	// }

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
	int ret;                     /**< Function return.                */
	int inbox;                   /**< Process stdinbox.               */
	int outbox;                  /**< Mailbox used to send request.   */
	int outportal;               /**< Portal used to send data.       */
	int outportal_port;          /**< Outportal associated port.      */
	int remote;                  /**< Remote process nodenum.         */
	int remote_port;             /**< Remote process allocated port.  */
	int remote_outbox_port;
	const char *remote_pname;    /**< Target process name.            */
	struct comm_message message; /**< Request message.                */
	struct comm_message confirm;

	/* Initializes some basic variables. */
	inbox = curr_mpi_proc_inbox();
	remote_pname = process_name(dest);

	ret = (MPI_ERR_INTERN);

	/* Retrieves complete address from remote_pname. */
	if ((remote = nanvix_name_address_lookup(remote_pname, &remote_port)) < 0)
		goto ret0;

	/* Opens the outbox to send the request. */
	if ((outbox = nanvix_mailbox_open(remote_pname, remote_port)) < 0)
		goto ret0;

	/**
	 * Opens a new portal to send data.
	 *
	 * @note The remote_port is the same for outbox and outportal because
	 * LWMPI uses stdikc as default inbox/inportal.
	 */
	if ((outportal = nanvix_portal_open(remote_pname, remote_port)) < 0)
		goto ret1;

	/* Gets outportal port number to send in the request. */
	outportal_port = nanvix_portal_get_port(outportal);

	/* Constructs the message header. */
	request_header_build(&message, cid, datatype, size, tag, outportal_port);

#if DEBUG
	uprintf("Sending Request-to-send...");
#endif /* DEBUG */

	/* Sends the request-to-send message to the receiver. */
	if ((ret = nanvix_mailbox_write(outbox, (const void*) &message, sizeof(struct comm_message))) < 0)
		goto ret2;

	/* Receives the confirmation message. */
	if ((ret = nanvix_mailbox_set_remote(inbox, remote, MAILBOX_ANY_PORT)) < 0)
		goto ret2;

#if DEBUG
	uprintf("Receiving confirmation...");
#endif /* DEBUG */

	if ((ret = nanvix_mailbox_read(inbox, (void *) &confirm, sizeof(struct comm_message))) < 0)
		goto ret2;

	/* Retrieves the output box port of the remote. */
	remote_outbox_port = confirm.msg.confirm.mailbox_port;

#if DEBUG
	uprintf("Sending data...");
#endif /* DEBUG */

	/* Sends the message to the receiver. */
	if ((ret = nanvix_portal_write(outportal, buf, size)) < 0)
		goto ret2;

	if ((ret = nanvix_mailbox_set_remote(inbox, remote, remote_outbox_port)) < 0)
		goto ret2;

#if DEBUG
	uprintf("Waiting for ACK...");
#endif /* DEBUG */

	if ((ret = nanvix_mailbox_read(inbox, (void *) &message, sizeof(struct comm_message))) < 0)
		goto ret2;

#if DEBUG
	uprintf("Finishing protocol...");
#endif /* DEBUG */

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
	if (!WITHIN(cid, 0, MPI_CONTEXT_LIMIT))
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
	int inbox;                   /**< Process stdinbox.        */
	int inportal;                /**< Process stdinportal.     */
	int overflow;
	int outbox;                  /**< Output mailbox.          */
	int remote_port;
	const char *remote_pname;    /**< Source process name.     */
	struct comm_message message; /**< Received message.        */
	struct comm_message reply;   /**< Requisition reply.       */

	inbox = curr_mpi_proc_inbox();
	inportal = curr_mpi_proc_inportal();
	remote_pname = process_name(src);

again:
	comm_request_build(req->cid, req->src, req->tag, &message.req);

	/* Search for a previous requisition. */
	if (comm_request_search(&message))
		goto found;

	/* Waits for a send request. */
	if (nanvix_mailbox_read(inbox, &message, sizeof(struct comm_message)) < 0)
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

#if DEBUG
	uprintf("Found Matching request from process %s ...", remote_pname);
#endif /* DEBUG */

	/* Checks the other information that came in the message. */
	if (!mpi_datatypes_match(datatype, message.msg.send.datatype))
		return (MPI_ERR_TYPE);

	ret = MPI_ERR_INTERN;

	/* Gets remote inbox port number. */
	if (nanvix_name_address_lookup(remote_pname, &remote_port) < 0)
		return (ret);

	uassert(remote_port > 0);

	/* Opens a mailbox to send the ACK. */
	if ((outbox = nanvix_mailbox_open(remote_pname, remote_port)) < 0)
		return (ret);

	/* Prepares the confirmation message. */
	reply.msg.confirm.mailbox_port = nanvix_mailbox_get_port(outbox);

#if DEBUG
	uprintf("Writing confirmation...");
#endif /* DEBUG */

	/* Emits a confirmation message containing the outbox port that will send the ACK. */
	if ((ret = nanvix_mailbox_write(outbox, (const void *) &reply, sizeof(struct comm_message))) < 0)
		goto end;

#if DEBUG
	uprintf("Allowing remote portal...");
#endif /* DEBUG */

	/* Allows the remote to send data. */
	if ((ret = nanvix_portal_allow2(inportal, message.req.src, message.msg.send.portal_port)) < 0)
		goto end;

	overflow = 0;

	/* Checks the amount of data to be received. */
	if (size < message.msg.send.size)
	{
		req->received_size = size;
		overflow = 1;
	}
	else
		req->received_size = message.msg.send.size;

#if DEBUG
	uprintf("Receiving data...");
#endif /* DEBUG */

	/* Receives data. */
	if (nanvix_portal_read(inportal, buf, req->received_size) < 0)
	{
		req->received_size = 0;
		return (MPI_ERR_INTERN);
	}

	ret = MPI_SUCCESS;

	if (overflow)
		ret = MPI_ERR_OTHER;

	/* Builds the return message. */
	reply.req.cid         = cid;
	reply.req.src         = knode_get_num();
	reply.msg.ret.errcode = ret;

#if DEBUG
	uprintf("Sending ACK...");
#endif /* DEBUG */

	/* Sends an ACK message. */
	if (nanvix_mailbox_write(outbox, (const void *) &reply, sizeof(struct comm_message)) < 0)
		ret = MPI_ERR_INTERN;

#if DEBUG
	uprintf("Finishing protocol...");
#endif /* DEBUG */

end:
	uassert(nanvix_mailbox_close(outbox) == 0);

	return (ret);
}

PUBLIC int recv(int cid, void *buf, size_t size, mpi_process_t *src, int datatype, struct comm_request *req)
{
	int ret;

	/* Bad context. */
	if (!WITHIN(cid, 0, MPI_CONTEXT_LIMIT))
		return (MPI_ERR_INTERN);

	/* Bad request holder. */
	if (req == NULL)
		return (MPI_ERR_INTERN);

	ret = __recv(cid, buf, size, src, datatype, req);

	return (ret);
}

