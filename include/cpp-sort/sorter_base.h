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
#ifndef CPPSORT_SORTER_BASE_H_
#define CPPSORT_SORTER_BASE_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <cstddef>
#include <type_traits>

namespace cppsort
{
    // This class is a CRTP base class whose sorters inherit
    // from which gives them the ability to convert to function
    // pointers. This mechanism is only possible if sorters are
    // stateless.
    template<typename Sorter>
    class sorter_base
    {
        protected:

            template<typename Iterable>
            using fptr_t = std::result_of_t<Sorter(Iterable&)>(*)(Iterable&);

            template<typename Iterable, typename Compare>
            using fptr_cmp_t = std::result_of_t<Sorter(Iterable&, Compare)>(*)(Iterable&, Compare);

        public:

            template<typename Iterable>
            operator fptr_t<Iterable>() const
            {
                return [](Iterable& iterable) {
                    return Sorter{}(iterable);
                };
            }

            template<typename Iterable, typename Compare>
            operator fptr_cmp_t<Iterable, Compare>() const
            {
                return [](Iterable& iterable, Compare compare) {
                    return Sorter{}(iterable, compare);
                };
            }
    };
}

#endif // CPPSORT_SORTER_BASE_H_