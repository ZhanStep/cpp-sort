/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Morwenn
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef CPPSORT_DETAIL_SORTING_NETWORK_SORT24_H_
#define CPPSORT_DETAIL_SORTING_NETWORK_SORT24_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <functional>
#include "../swap_if.h"

namespace cppsort
{
namespace detail
{
    template<>
    struct sorting_network_sorter_impl<24u>
    {
        template<
            typename RandomAccessIterator,
            typename Compare = std::less<>
        >
        auto operator()(RandomAccessIterator first, RandomAccessIterator, Compare compare={}) const
            -> void
        {
            // Sort both halves of the array
            sorting_network_sorter<12u>{}(first, first+12u, compare);
            sorting_network_sorter<12u>{}(first+12u, first+24u, compare);

            // Merge even indices
            detail::swap_if(first[0u], first[12u], compare);
            detail::swap_if(first[2u], first[14u], compare);
            detail::swap_if(first[4u], first[16u], compare);
            detail::swap_if(first[6u], first[18u], compare);
            detail::swap_if(first[8u], first[20u], compare);
            detail::swap_if(first[10u], first[22u], compare);
            detail::swap_if(first[2u], first[12u], compare);
            detail::swap_if(first[10u], first[20u], compare);
            detail::swap_if(first[4u], first[12u], compare);
            detail::swap_if(first[6u], first[14u], compare);
            detail::swap_if(first[8u], first[16u], compare);
            detail::swap_if(first[10u], first[18u], compare);
            detail::swap_if(first[8u], first[12u], compare);
            detail::swap_if(first[10u], first[14u], compare);
            detail::swap_if(first[6u], first[8u], compare);
            detail::swap_if(first[10u], first[12u], compare);
            detail::swap_if(first[14u], first[16u], compare);

            // Merge odd indices
            detail::swap_if(first[1u], first[13u], compare);
            detail::swap_if(first[3u], first[15u], compare);
            detail::swap_if(first[5u], first[17u], compare);
            detail::swap_if(first[7u], first[19u], compare);
            detail::swap_if(first[9u], first[21u], compare);
            detail::swap_if(first[11u], first[23u], compare);
            detail::swap_if(first[3u], first[13u], compare);
            detail::swap_if(first[11u], first[21u], compare);
            detail::swap_if(first[5u], first[13u], compare);
            detail::swap_if(first[7u], first[15u], compare);
            detail::swap_if(first[9u], first[17u], compare);
            detail::swap_if(first[11u], first[19u], compare);
            detail::swap_if(first[9u], first[13u], compare);
            detail::swap_if(first[11u], first[15u], compare);
            detail::swap_if(first[7u], first[9u], compare);
            detail::swap_if(first[11u], first[13u], compare);
            detail::swap_if(first[15u], first[17u], compare);

            // Last range of swaps
            detail::swap_if(first[1u], first[2u], compare);
            detail::swap_if(first[3u], first[4u], compare);
            detail::swap_if(first[5u], first[6u], compare);
            detail::swap_if(first[7u], first[8u], compare);
            detail::swap_if(first[9u], first[10u], compare);
            detail::swap_if(first[11u], first[12u], compare);
            detail::swap_if(first[13u], first[14u], compare);
            detail::swap_if(first[15u], first[16u], compare);
            detail::swap_if(first[17u], first[18u], compare);
            detail::swap_if(first[19u], first[20u], compare);
            detail::swap_if(first[21u], first[22u], compare);
        }
    };
}}

#endif // CPPSORT_DETAIL_SORTING_NETWORK_SORT24_H_
