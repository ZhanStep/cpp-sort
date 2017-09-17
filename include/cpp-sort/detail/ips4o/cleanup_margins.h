/*
 * ips4o/cleanup_margins.hpp
 *
 * In-place Parallel Super Scalar Samplesort (IPS⁴o)
 *
 ******************************************************************************
 * BSD 2-Clause License
 *
 * Copyright © 2017, Michael Axtmann <michael.axtmann@kit.edu>
 * Copyright © 2017, Daniel Ferizovic <daniel.ferizovic@student.kit.edu>
 * Copyright © 2017, Sascha Witt <sascha.witt@kit.edu>
 * All rights reserved.
 *
 * Modified in 2017 by Morwenn for inclusion into cpp-sort.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef CPPSORT_DETAIL_IPS4O_CLEANUP_MARGINS_H_
#define CPPSORT_DETAIL_IPS4O_CLEANUP_MARGINS_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <limits>
#include <utility>
#include <cpp-sort/sorters/insertion_sorter.h>
#include "ips4o_fwd.h"
#include "memory.h"
#include "../assume.h"

namespace cppsort
{
namespace detail
{
namespace ips4o
{
namespace detail
{
    /*
     * Saves margins at thread boundaries.
     */
    template<typename Cfg>
    auto Sorter<Cfg>::saveMargins(int last_bucket)
        -> std::pair<int, typename Cfg::difference_type>
    {
        diff_t tail = 0;
        diff_t end = 0;
        // The last thread doesn't have to do anything
        if (my_id_ != num_threads_ - 1) {
            // Find last bucket boundary in this thread's area
            tail = bucket_start_[last_bucket];
            end = Cfg::alignToNextBlock(tail);
            // Don't need to do anything if there is no overlap
            if (tail != end) {
                const auto start_of_last_block = end - Cfg::kBlockSize;
                // Find bucket this last block belongs to
                diff_t last_start;
                do {
                    --last_bucket;
                    last_start = bucket_start_[last_bucket];
                } while (last_start > start_of_last_block);

                // Check if the last block has been written
                const auto write = shared_->bucket_pointers[last_bucket].getWrite();
                if (write < end) {
                    tail = end;
                } else {
                    tail = bucket_start_[last_bucket + 1];
                }
            }
        }

        // Read excess elements, if necessary
        if (tail != end) {
            local_.swap[0].readFrom(begin_ + tail, end - tail);
        }
        return {last_bucket, end - tail};
    }

    /*
     * Fills margins from buffers.
     */
    template<typename Cfg>
    auto Sorter<Cfg>::writeMargins(const int first_bucket, const int last_bucket,
                                   const int overflow_bucket, const int swap_bucket,
                                   const diff_t in_swap_buffer)
        -> void
    {
        const bool is_last_level = end_ - begin_ <= Cfg::kSingleLevelThreshold;
        auto compare = classifier_->getComparator();

        for (int i = first_bucket ; i < last_bucket ; ++i) {
            // Get bucket information
            const auto bstart = bucket_start_[i];
            const auto bend = bucket_start_[i + 1];
            const auto bwrite = bucket_pointers_[i].getWrite();
            // Destination where elements can be written
            auto dst = begin_ + bstart;
            auto remaining = Cfg::alignToNextBlock(bstart) - bstart;

            if (i == overflow_bucket && overflow_) {
                // Overflow buffer has been written => write pointer must be at end of bucket
                CPPSORT_ASSUME(Cfg::alignToNextBlock(bend) == bwrite);

                auto src = overflow_->data();
                CPPSORT_ASSUME((bend - (bwrite - Cfg::kBlockSize)) + remaining >= Cfg::kBlockSize);
                auto tail_size = Cfg::kBlockSize - remaining;

                // Fill head
                while (remaining--) {
                    *dst++ = std::move(*src++);
                }

                // Write remaining elements into tail
                dst = begin_ + bwrite - Cfg::kBlockSize;
                remaining = std::numeric_limits<diff_t>::max();
                while (tail_size--) {
                    *dst++ = std::move(*src++);
                }

                overflow_->reset(Cfg::kBlockSize);
            } else if (i == swap_bucket && in_swap_buffer) {
                // Bucket of last block in this thread's area => write swap buffer
                auto src = local_.swap[0].data();
                auto n = remaining <= in_swap_buffer ? remaining : in_swap_buffer;

                // Write to head first
                remaining -= n;
                const auto left_in_swap = in_swap_buffer - n;
                while (n--) {
                    *dst++ = std::move(*src++);
                }

                if (not remaining) {
                    // Then, write to tail
                    dst = begin_ + bwrite;
                    remaining = std::numeric_limits<diff_t>::max();

                    n = left_in_swap;
                    while (n--) {
                        *dst++ = std::move(*src++);
                    }
                }

                local_.swap[0].reset(in_swap_buffer);
            } else if (bwrite > bend && bend - bstart > Cfg::kBlockSize) {
                // Final block has been written => move excess elements to head
                CPPSORT_ASSUME(Cfg::alignToNextBlock(bend) == bwrite);

                auto src = begin_ + bend;
                auto head_size = bwrite - bend;
                // Must fit, no other empty space left
                CPPSORT_ASSUME(head_size <= remaining);

                // Write to head
                remaining -= head_size;
                while (head_size--) {
                    *dst++ = std::move(*src++);
                }
            }

            // Write elements from buffers
            for (int t = 0 ; t < num_threads_ ; ++t) {
                auto& buffers = shared_ ? shared_->local[t]->buffers : local_.buffers;
                auto src = buffers.data(i);
                auto count = buffers.size(i);

                while (count) {
                    // Fill the head first ...
                    auto n = count <= remaining ? count : remaining;
                    count -= n;
                    remaining -= n;
                    while (n--) {
                        *dst++ = std::move(*src++);
                    }

                    if (not remaining) {
                        // ... then write the remaining elements into empty blocks and the tail
                        dst = begin_ + bwrite;
                        remaining = std::numeric_limits<diff_t>::max();
                    }
                }
                buffers.reset(i);
            }

            // Perform final base case sort here, while the data is still cached
            if (is_last_level || (bend - bstart) <= 2 * Cfg::kBaseCaseSize) {
                cppsort::insertion_sort(begin_ + bstart, begin_ + bend, compare);
            }
        }
    }
}}}}

#endif // CPPSORT_DETAIL_IPS4O_CLEANUP_MARGINS_H_