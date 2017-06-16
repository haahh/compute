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

#include <boost/compute/distributed/context.hpp>
#include <boost/compute/distributed/command_queue.hpp>
#include <boost/compute/distributed/vector.hpp>
#include <boost/compute/distributed/distributed_iterator.hpp>

#include "distributed_check_functions.hpp"
#include "distributed_queue_setup.hpp"

#include "check_macros.hpp"
#include "context_setup.hpp"

namespace bc = boost::compute;

BOOST_AUTO_TEST_CASE(basic)
{
    // construct distributed::command_queue
    bc::distributed::command_queue distributed_queue =
        get_distributed_queue(queue);

    bc::int_ data[] = { 1, -2, -3, -4, 5, 6, -7, -8, -9, 10 };
    bc::distributed::vector<bc::int_> distributed_vector(
        data, data + 10, distributed_queue
    );
    distributed_queue.finish();

    typedef bc::distributed::vector<bc::int_>::iterator itype;

    std::vector<itype> begins;
    std::vector<itype> ends;
    for(size_t i = 0; i < distributed_vector.parts(); i++)
    {
        begins.push_back(distributed_vector.begin(i));
        ends.push_back(distributed_vector.end(i));
    }
    bc::distributed_iterator<itype> iter(begins, ends);

    // for(size_t i = 0; i < 10; i++)
    // {
    //     std::cout << *iter << std::endl;
    //     iter++;
    // }

    for(size_t i = 0; i < 10; i++)
    {
        std::cout << iter.read(distributed_queue) << std::endl;
        iter++;
    }
}

BOOST_AUTO_TEST_SUITE_END()
