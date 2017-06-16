//---------------------------------------------------------------------------//
// Copyright (c) 2016 Jakub Szuppe <j.szuppe@gmail.com>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// See http://boostorg.github.com/compute for more information.
//---------------------------------------------------------------------------//

#define BOOST_TEST_MODULE TestDistributedIterator
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <iostream>

#include <boost/compute/iterator/counting_iterator.hpp>
#include <boost/compute/iterator/transform_iterator.hpp>

#include <boost/compute/distributed/context.hpp>
#include <boost/compute/distributed/command_queue.hpp>
#include <boost/compute/distributed/vector.hpp>
#include <boost/compute/distributed/multi_iterator.hpp>

#include "distributed_check_functions.hpp"
#include "distributed_queue_setup.hpp"

#include "check_macros.hpp"
#include "context_setup.hpp"

namespace bc = boost::compute;

BOOST_AUTO_TEST_CASE(basic_read_write)
{
    bc::distributed::command_queue distributed_queue =
        get_distributed_queue(queue);

    bc::int_ data[] = { 1, -2, -3, -4, 5, 6, -7, -8, -9, 10 };
    bc::distributed::vector<bc::int_> distributed_vector(
        data, data + 10, distributed_queue
    );
    distributed_queue.finish();

    typedef bc::distributed::vector<bc::int_>::iterator Iterator;

    std::vector<Iterator> begins;
    std::vector<Iterator> ends;
    for(size_t i = 0; i < distributed_vector.parts(); i++)
    {
        begins.push_back(distributed_vector.begin(i));
        ends.push_back(distributed_vector.end(i));
    }

    // Create
    bc::distributed::multi_iterator<Iterator> iter(begins, ends);

    // Modify (read and write)
    for(size_t i = 0; i < 10; i++)
    {
        *iter = *iter * -1;
        iter++;
    }

    bc::int_ host_expected[] = { -1, 2, 3, 4, -5, -6, 7, 8, 9, -10 };
    BOOST_CHECK(
        distributed_equal(
            distributed_vector,
            host_expected, host_expected + 10,
            distributed_queue
        )
    );
}

BOOST_AUTO_TEST_CASE(using_counting_iterator)
{
    typedef bc::counting_iterator<bc::int_> Iterator;
    typedef bc::distributed::multi_iterator<Iterator> MultiIterator;

    std::vector<Iterator> begins;
    std::vector<Iterator> ends;
    for(bc::int_ i = 0; i < 6; i += 3)
    {
        begins.push_back(
            bc::make_counting_iterator<bc::int_>(i)
        );
        ends.push_back(
            bc::make_counting_iterator<bc::int_>(i) + 3
        );
    }

    MultiIterator iter(begins, ends);
    MultiIterator end = iter + 6;

    for(bc::int_ i = 0; iter != end; iter++, i++)
    {
        BOOST_CHECK_EQUAL(i, *iter);
    }
}

BOOST_AUTO_TEST_CASE(using_transform_iterator)
{
    typedef boost::compute::transform_iterator<
                boost::compute::buffer_iterator<bc::int_>,
                boost::compute::abs<bc::int_>
            > Iterator;
    typedef bc::distributed::multi_iterator<Iterator> MultiIterator;

    bc::distributed::command_queue distributed_queue =
        get_distributed_queue(queue);

    bc::int_ data[] = { -1, -2, -3, -4, -5, -6 };
    bc::distributed::vector<bc::int_> distributed_vector(
        data, data + 6, distributed_queue
    );
    distributed_queue.finish();

    std::vector<Iterator> begins;
    std::vector<Iterator> ends;
    for(size_t i = 0; i < distributed_vector.parts(); i++)
    {
        begins.push_back(
            bc::make_transform_iterator(
                distributed_vector.begin(i),
                boost::compute::abs<bc::int_>()
            )
        );
        ends.push_back(
            bc::make_transform_iterator(
                distributed_vector.end(i),
                boost::compute::abs<bc::int_>()
            )
        );
    }

    MultiIterator iter(begins, ends);
    MultiIterator end = iter + 6;

    for(bc::int_ i = 1; iter != end; iter++, i++)
    {
        BOOST_CHECK_EQUAL(i, *iter);
    }
}


BOOST_AUTO_TEST_SUITE_END()
