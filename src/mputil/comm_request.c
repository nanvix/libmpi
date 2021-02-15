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

#define __NEED_RESOURCE

#include <nanvix/hal/resource.h>
#include <nanvix/sys/mailbox.h>
#include <nanvix/sys/mutex.h>
#include <nanvix/sys/noc.h>
#include <nanvix/ulib.h>
#include <mputil/comm_request.h>
#include <mpi.h>

/**
 * @brief Maximum size of request queue.
 */
#define RQUEUE_MAX_SIZE 32

/**
 * @brief Request queue node structure.
 */
PRIVATE struct comm_request_node 
{
	/*
	 * XXX: Don't Touch! This Must Come First!
	 */
	struct resource resource;        /**< Generic resource information. */

	struct comm_message msg;
	struct comm_request_node * next;
} rnodes[RQUEUE_MAX_SIZE];

/**
 * @brief Pool of request nodes.
 */
PRIVATE const struct resource_pool pool_rnodes = {
	rnodes, RQUEUE_MAX_SIZE, sizeof(struct comm_request_node)
};

/**
 * @brief Linked queue ordered by arrival.
 */
PRIVATE struct comm_request_queue
{
	struct comm_request_node * head;
	struct comm_request_node * tail;
} rqueue;

/**
 * @brief Standard inbox to receive communication requests.
 */
PRIVATE int _inbox = -1;

/**
 * @brief Sinalizes that there is already a thread looking for new requests.
 */
PRIVATE int _inbox_occupied = 0;

/**
 * @brief Request queue lock.
 */
PRIVATE struct nanvix_mutex _rqueue_lock;

/*============================================================================*
 * AUXILIAR FUNCTIONS                                                         *
 *============================================================================*/

/*============================================================================*
 * comm_request_insert                                                        *
 *============================================================================*/

/**
 * @brief Registers a requisition node in the requisitions queue.
 *
 * @param node Node to be registered.
 */
PRIVATE void comm_request_insert(struct comm_request_node *node)
{
	uassert(node != NULL);

	/* Configure request node. */
	node->next = NULL;

	/* Has any node in the request queue?. */
	if (rqueue.tail)
		rqueue.tail->next = node;
	else
		rqueue.head = node;

	/* Updates tail pointer. */
	rqueue.tail = node;
}

/*============================================================================*
 * comm_request_match                                                         *
 *============================================================================*/

/**
 * @brief Compares if two communication requisitions are equal.
 *
 * @param req1 First requisition to be compared.
 * @param req2 Second requisition.
 *
 * @returns Returns ZERO if the requisitions are different and a NON-ZERO value if
 * they match correctly.
 */
PRIVATE int comm_request_match(struct comm_request *req1, struct comm_request *req2)
{
	uassert(req1 != NULL);
	uassert(req2 != NULL);

	/* Check cid. */
	if (req1->cid != req2->cid)
		return (0);

	/* Check target. */
	if (req1->target != req2->target)
		return (0);

	/* Check src. */
	if ((req1->src != MPI_ANY_SOURCE) && (req2->src != MPI_ANY_SOURCE))
	{
		if (req1->src != req2->src)
			return (0);
	}

	/* Check tag. */
	if ((req1->tag != MPI_ANY_TAG) && (req2->tag != MPI_ANY_TAG))
	{
		if (req1->tag != req2->tag)
			return (0);
	}

	return (1);
}

/*============================================================================*
 * EXTERNAL FUNCTIONS                                                         *
 *============================================================================*/

/*============================================================================*
 * comm_request_build                                                         *
 *============================================================================*/

/**
 * @todo Provide a detailed description.
 */
PUBLIC void comm_request_build(int cid, int src, int target, int tag, struct comm_request *req)
{
	uassert(req != NULL);

	req->cid    = cid;
	req->src    = src;
	req->target = target;
	req->tag    = tag;
}

/*============================================================================*
 * comm_request_receive                                                       *
 *============================================================================*/

/**
 * @brief Receives a new communication request from the IKC facility.
 *
 * @param msg Message holder with a valid request information.
 *
 * @returns Upon successful completion, zero is returned with the received message
 * copied into @p msg. Upon failure, a negative error code is returned instead.
 */
PUBLIC int comm_request_receive(struct comm_message *msg)
{
	int id;
	int ret;
	struct comm_request_node *node;

	uassert(msg != NULL);

	ret = 0;

search:
	/* Have another thread received a new request? */
	if (comm_request_search(msg))
		goto end;

	nanvix_mutex_lock(&_rqueue_lock);

		if (_inbox_occupied)
		{
			nanvix_mutex_unlock(&_rqueue_lock);
			goto search;
		}

		_inbox_occupied = 1;

	nanvix_mutex_unlock(&_rqueue_lock);

again:
	/* Allocates a temporary node to receive the new request. */
	if ((id = resource_alloc(&pool_rnodes)) < 0)
	{
		ret = (-MPI_ERR_NO_MEM);
		goto desoccupy;
	}

	/* Gets the pointer to the allocated node. */
	node = &rnodes[id];

	/* Waits for a send request. */
	if (kmailbox_read(_inbox, &node->msg, sizeof(struct comm_message)) < 0)
	{
		ret = (-MPI_ERR_UNKNOWN);
		goto desoccupy;
	}

	/* Checks if the expected and received requests match. */
	if (!comm_request_match(&msg->req, &node->msg.req))
	{
		/* Enqueue the received requisition. */
		comm_request_insert(node);
		goto again;
	}

desoccupy:
	nanvix_mutex_lock(&_rqueue_lock);

		_inbox_occupied = 0;

	nanvix_mutex_unlock(&_rqueue_lock);

	/* Copies the received message to @p message and frees the allocated node. */
	if (ret == 0)
	{
		umemcpy(msg, &node->msg, sizeof(struct comm_message));
		resource_free(&pool_rnodes, id);
	}

end:
	return (ret);
}

/*============================================================================*
 * comm_request_search                                                        *
 *============================================================================*/

/**
 * @brief Search into Request Queue.
 *
 * @param msg Request reference with target information.
 *
 * @returns If a matched request is found, consume it and returns a non-zero
 * value. Zero is returned instead.
 */
PUBLIC int comm_request_search(struct comm_message *msg)
{
	struct comm_request_node *previous;
	struct comm_request_node *current;

	uassert(msg != NULL);

	if (rqueue.head == NULL)
		return (0);

	previous = NULL;
	current  = rqueue.head;

	/* Search msg in the request queue. */
	while (current && !comm_request_match(&msg->req, &current->msg.req))
	{
		previous = current;
		current  = current->next;
	}

	/* Found? (current != NULL) */
	if (current)
	{
		/* Consumes the message. */
		umemcpy(msg, &current->msg, sizeof(struct comm_message));

		/* Consumes head node. */ 
		if (previous == NULL)
			rqueue.head = current->next;

		/* Consumes intermediare node (if current == tail then previous->next == NULL). */
		else
			previous->next = current->next;
	
		/* Consumes tail node (if head == tail => previous == NULL). */
		if (current == rqueue.tail)
			rqueue.tail = previous;

		/* Releases resource. */
		resource_free(&pool_rnodes, (current - rnodes));

		return (1);
	}

	return (0);
}

/*============================================================================*
 * comm_request_init                                                          *
 *============================================================================*/

/**
 * @todo Implement this function.
 */
PUBLIC int comm_request_init(void)
{
	rqueue.head = NULL;
	rqueue.tail = NULL;

	/* Initialize default inbox for receiving communication requests. */
	if ((_inbox = kmailbox_create(knode_get_num(), COMM_REQ_RECV_PORT)) < 0)
		return (-MPI_ERR_INTERN);

	/* Initialize requisition queue mutex. */
	uassert(nanvix_mutex_init(&_rqueue_lock, NULL) == 0);

	return (0);
}

/*============================================================================*
 * comm_request_finalize                                                      *
 *============================================================================*/

/**
 * @todo Implement this function.
 *
 * @todo This function must handle the restant requisitions before returning.
 */
PUBLIC int comm_request_finalize(void)
{
	struct comm_request_node * current;

	current = rqueue.head;

	/**
	 * @todo We should handle all requisitions here leading them to a discard.
	 * For now only returns an error sinalizing an inconsistent state.
	 */
	if (current)
		return (-MPI_ERR_PENDING);

	/* Unlinks the mailbox used to receive the communication requests. */
	if (kmailbox_unlink(_inbox) < 0)
		return (-MPI_ERR_UNKNOWN);

	rqueue.head = NULL;
	rqueue.tail = NULL;

	return (0);
}
