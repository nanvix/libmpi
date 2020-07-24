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
#include <nanvix/ulib.h>
#include <posix/errno.h>
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
 * @todo Provide a detailed description.
 */
PUBLIC void comm_request_build(int cid, int src, int tag, struct comm_request *req)
{
	uassert(req != NULL);

	req->cid = cid;
	req->src = src;
	req->tag = tag;
}

/**
 * @todo Provide a detailed description.
 */
PUBLIC int comm_request_match(struct comm_request *req1, struct comm_request *req2)
{
	uassert(req1 != NULL);
	uassert(req2 != NULL);

	/* Check cid. */
	if (req1->cid != req2->cid)
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

/**
 * @brief Allocates a new request and registers it in the requisitions queue.
 *
 * @param msg Requisition to be registered.
 *
 * @returns Upon successful completion, zero is returned. Upon failure, a negative
 * error code is returned instead.
 *
 * @todo Implement this function.
 *
 * @note This function needs to copy the msg attributes in a safe structure, not only
 * storing its reference.
 */
PUBLIC int comm_request_register(struct comm_message *msg)
{
	int id;
	struct comm_request_node *next;

	uassert(msg != NULL);

	/* Allocate portal. */
	if ((id = resource_alloc(&pool_rnodes)) < 0)
		return (-EAGAIN);

	/* Get the pointer to allocated node. */
	next = &rnodes[id];

	/* Configure request node. */
	next->next = NULL;
	umemcpy(&next->msg, msg, sizeof(struct comm_message));

	/* Has any node in the request queue?. */
	if (rqueue.tail)
		rqueue.tail->next = next;
	else
		rqueue.head = next;

	/* Updates tail pointer. */
	rqueue.tail = next;

	return (0);
}

/**
 * @brief Search into Request Queue.
 *
 * @param msg Request reference with target information.
 *
 * @returns If a matched request is found, consume it and returns non-zero
 * value, zero, otherwise.
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

/**
 * @todo Implement this function.
 */
PUBLIC int comm_request_init(void)
{
	rqueue.head = NULL;
	rqueue.tail = NULL;

	return (0);
}

/**
 * @todo Implement this function.
 */
PUBLIC int comm_request_finalize(void)
{
	struct comm_request_node * current;

	current = rqueue.head;

	/* Release requesitions. */
	while (current)
	{
		resource_free(&pool_rnodes, (current - rnodes));
		current = current->next;
	}

	rqueue.head = NULL;
	rqueue.tail = NULL;

	return (0);
}

