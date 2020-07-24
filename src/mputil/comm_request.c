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
#include <mputil/comm_request.h>
#include <mpi.h>

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
 * @todo Implement this function.
 */
PUBLIC int comm_request_register(struct comm_request *req)
{
	UNUSED(req);

	return (0);
}

/**
 * @todo Implement this function.
 */
PUBLIC int comm_request_init(void)
{
	return (0);
}

/**
 * @todo Implement this function.
 */
PUBLIC int comm_request_finalize(void)
{
	return (0);
}