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
#include <nanvix/sys/mutex.h>
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

PRIVATE struct nanvix_mutex _recv_lock;

/*============================================================================*
 * Message Operations.                                                        *
 *============================================================================*/

/**
 * @todo Provide a detailed description.
 */
PRIVATE void request_header_build(struct comm_message *m, uint16_t cid, int src,
	                              int target, uint16_t type, int size, uint32_t tag,
	                              uint8_t portal_port)
{
	uassert(m != NULL);

	m->req.src              = src;
	m->req.target           = target;
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
	uassert(nanvix_mutex_init(&_recv_lock, NULL) == 0);

	return (0);
}

/**
 * @see comm_context_finalize in communication.h.
 */
PUBLIC int comm_context_finalize(void)
{
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

PRIVATE int __rsend(int cid, const void *buf, size_t size, int src, int dest,
	                mpi_process_t *dest_proc, int datatype, int tag)
{
	UNUSED(cid);
	UNUSED(buf);
	UNUSED(datatype);
	UNUSED(size);
	UNUSED(src);
	UNUSED(dest);
	UNUSED(dest_proc);
	UNUSED(tag);

	return (MPI_ERR_UNSUPPORTED_OPERATION);
}

/**
 * @todo Provide a detailed description.
 *
 * @todo Implement this mode.
 */
PRIVATE int __bsend(int cid, const void *buf, size_t size, int src, int dest,
	                mpi_process_t *dest_proc, int datatype, int tag)
{
	UNUSED(cid);
	UNUSED(buf);
	UNUSED(datatype);
	UNUSED(size);
	UNUSED(src);
	UNUSED(dest);
	UNUSED(dest_proc);
	UNUSED(tag);

	return (MPI_ERR_UNSUPPORTED_OPERATION);
}

/**
 * @todo Provide a detailed description.
 */
PRIVATE int __ssend(int cid, const void *buf, size_t size, int src, int dest,
	                mpi_process_t *dest_proc, int datatype, int tag)
{
	int ret;                     /**< Function return.              */
	int inbox;                   /**< Process stdinbox.             */
	int outbox;                  /**< Mailbox used to send request. */
	int outportal;               /**< Portal used to send data.     */
	int outportal_port;          /**< Outportal associated port.    */
	int remote;                  /**< Remote process nodenum.       */
	int remote_port;             /**< Remote process inbox port.    */
	int remote_outbox_port;      /**< Remote process outbox port.   */
	const char *remote_pname;    /**< Target process name.          */
	struct comm_message message; /**< Request message.              */
	struct comm_message confirm; /**< Confirmation message.         */

	/* Initializes some basic variables. */
	inbox = curr_mpi_proc_inbox();
	remote_pname = process_name(dest_proc);

	ret = (MPI_ERR_INTERN);

#if DEBUG
	uprintf("%s preparing to send to %s...", process_name(curr_mpi_proc()), process_name(dest_proc));
#endif /* DEBUG */

	/* Retrieves complete address from remote_pname. */
	if ((remote = nanvix_name_address_lookup(remote_pname, &remote_port)) < 0)
		goto ret0;

	/* Opens the outbox to send the request. */
	if ((outbox = nanvix_mailbox_open(remote_pname, COMM_REQ_RECV_PORT)) < 0)
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
	request_header_build(&message, cid, src, dest, datatype, size, tag, outportal_port);

#if DEBUG
	uprintf("%s sending Request-to-send to %d:%d...", process_name(curr_mpi_proc()), remote, COMM_REQ_RECV_PORT);
#endif /* DEBUG */

	/* Sends the request-to-send message to the receiver. */
	if ((ret = nanvix_mailbox_write(outbox, (const void*) &message, sizeof(struct comm_message))) < 0)
		goto ret2;

	/* Receives the confirmation message. */
	if ((ret = nanvix_mailbox_set_remote(inbox, remote, MAILBOX_ANY_PORT)) < 0)
		goto ret2;

#if DEBUG
	uprintf("%s receiving confirmation...", process_name(curr_mpi_proc()));
#endif /* DEBUG */

	if ((ret = nanvix_mailbox_read(inbox, (void *) &confirm, sizeof(struct comm_message))) < 0)
		goto ret2;

	/* Retrieves the output box port of the remote. */
	remote_outbox_port = confirm.msg.confirm.mailbox_port;

#if DEBUG
	uprintf("%s sending data from port %d...", process_name(curr_mpi_proc()), outportal_port);
#endif /* DEBUG */

	/* Sends the message to the receiver. */
	if ((ret = nanvix_portal_write(outportal, buf, size)) < 0)
		goto ret2;

#if DEBUG
	uprintf("%s waiting for ACK...", process_name(curr_mpi_proc()));
#endif /* DEBUG */

	if ((ret = nanvix_mailbox_set_remote(inbox, remote, remote_outbox_port)) < 0)
		goto ret2;

	if ((ret = nanvix_mailbox_read(inbox, (void *) &message, sizeof(struct comm_message))) < 0)
		goto ret2;

#if DEBUG
	uprintf("%s finishing protocol...", process_name(curr_mpi_proc()));
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

PUBLIC int send(int cid, const void *buf, size_t size, int src, int dest,
	            mpi_process_t *dest_proc, int datatype, int tag, int mode)
{
	int ret;

	/* Bad context. */
	if (!WITHIN(cid, 0, MPI_CONTEXT_LIMIT))
		return (MPI_ERR_INTERN);

	/* Mode selection. */
	switch (mode)
	{
		case COMM_READY_MODE:
			ret = __rsend(cid, buf, size, src, dest, dest_proc, datatype, tag);
			break;

		case COMM_BUFFERED_MODE:
			ret = __bsend(cid, buf, size, src, dest, dest_proc, datatype, tag);
			break;

		case COMM_SYNC_MODE:
			ret = __ssend(cid, buf, size, src, dest, dest_proc, datatype, tag);
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

PRIVATE int __recv(int cid, void *buf, size_t size, mpi_process_t *src, int datatype,
	               struct comm_request *req)
{
	int ret;                     /**< Function return.         */
	int inportal;                /**< Process stdinportal.     */
	int overflow;                /**< Overflow flag.           */
	int outbox;                  /**< Output mailbox.          */
	int remote_node;             /**< Remote node number.      */
	int remote_port;             /**< Remote node port number. */
	const char *remote_pname;    /**< Source process name.     */
	struct comm_message message; /**< Received message.        */
	struct comm_message reply;   /**< Requisition reply.       */

	inportal = curr_mpi_proc_inportal();
	remote_pname = process_name(src);

#if DEBUG
	uprintf("%s preparing to receive from %s...", process_name(curr_mpi_proc()), process_name(src));
#endif /* DEBUG */

	/* Builds the request to be compared with an arriving one. */
	comm_request_build(req->cid, req->src, req->target, req->tag, &message.req);

	/* Receives a requisition from the interconnection. */
	if ((ret = comm_request_receive(&message)) < 0)
		return (ret);

#if DEBUG
	uprintf("%s found matching request from %s ...", process_name(curr_mpi_proc()), remote_pname);
#endif /* DEBUG */

	/* Checks the other information that came in the message. */
	if (!mpi_datatypes_match(datatype, message.msg.send.datatype))
		return (MPI_ERR_TYPE);

	ret = MPI_ERR_INTERN;

	uassert(nanvix_mutex_lock(&_recv_lock) == 0);

	/* Gets remote inbox port number. */
	if ((remote_node = nanvix_name_address_lookup(remote_pname, &remote_port)) < 0)
		goto end;

	/* Opens a mailbox to send the ACK. */
	if ((outbox = nanvix_mailbox_open(remote_pname, remote_port)) < 0)
		goto end;

	/* Prepares the confirmation message. */
	reply.msg.confirm.mailbox_port = nanvix_mailbox_get_port(outbox);

#if DEBUG
	uprintf("%s writing confirmation...", process_name(curr_mpi_proc()));
#endif /* DEBUG */

	/* Emits a confirmation message containing the outbox port that will send the ACK. */
	if ((ret = nanvix_mailbox_write(outbox, (const void *) &reply, sizeof(struct comm_message))) < 0)
		goto end;

#if DEBUG
	uprintf("%s allowing remote portal on port %d...", process_name(curr_mpi_proc()), message.msg.send.portal_port);
#endif /* DEBUG */

	/* Allows the remote to send data. */
	if ((ret = nanvix_portal_allow2(inportal, remote_node, message.msg.send.portal_port)) < 0)
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
	uprintf("%s receiving data...", process_name(curr_mpi_proc()));
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
	uprintf("%s sending ACK...", process_name(curr_mpi_proc()));
#endif /* DEBUG */

	/* Sends an ACK message. */
	if (nanvix_mailbox_write(outbox, (const void *) &reply, sizeof(struct comm_message)) < 0)
		ret = MPI_ERR_INTERN;

#if DEBUG
	uprintf("%s finishing protocol...", process_name(curr_mpi_proc()));
#endif /* DEBUG */

end:
	uassert(nanvix_mutex_unlock(&_recv_lock) == 0);

	uassert(nanvix_mailbox_close(outbox) == 0);

	return (ret);
}

PUBLIC int recv(int cid, void *buf, size_t size, mpi_process_t *src, int datatype,
	            struct comm_request *req)
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

