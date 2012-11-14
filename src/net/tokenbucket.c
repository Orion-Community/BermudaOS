/*
 *  BermudaNet - Token bucket library
 *  Copyright (C) 2012   Michel Megens
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <tokenbucket.h>

/**
 * \file src/tokenbucket.c.
 * \brief With a tokenbucket, the maximum transfer rate (i.e. bitrate) of the connection can be limited.
 * 
 * \section algo Algorithm
 * The concept of the algorithm is as follows: \n
 * * Let \(R \) be the bitrate, \(t \) the amount of tokens added and $n$ the amount of tokens needed.\n
 * &nbsp;&nbsp; \(f(t) = 1/R \) \n
 * * The bucket can have at the most $B$ tokens. If a token arrives when the bucket is full, it is
 * discarded. \n
 * * When a network layer PDU arrives of $n$ bytes, $n$ bytes are removed from the bucket, and the
 * PDU is sent to the network driver. \n
 * * If viewer than $n$ tokens are available, the packet will be queued.
 * 
 */

/**
 * \brief Add tokens to the token bucket.
 * \param bucket Token bucket to add the tokens to.
 * \param tokens Amount of tokens to add.
 * \return 0 if the tokens are added successfully, 1 the given amount of tokens would cause a roll
 *         over.
 */
PUBLIC int cash_tokens(struct tbucket *bucket, size_t tokens)
{
	/*
	 * Don't roll over!
	 */
	if((bucket->tokens + tokens) < bucket->tokens) {
		bucket->tokens = 0;
		bucket->tokens = ~(bucket->tokens);
		return 1;
	} else {
		bucket->tokens += tokens;
		return 0;
	}
}
