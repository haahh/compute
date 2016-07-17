//---------------------------------------------------------------------------//
// Copyright (c) 2013-2014 Denis Demidov
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// See http://boostorg.github.com/compute for more information.
//---------------------------------------------------------------------------//

#ifndef BOOST_COMPUTE_TEST_CONTEXT_SETUP_HPP
#define BOOST_COMPUTE_TEST_CONTEXT_SETUP_HPP

#include <boost/compute/system.hpp>
#include <boost/compute/command_queue.hpp>

#include "opencl_version_check.hpp"

namespace boost {
namespace compute {

class device_mockup : public device
{
public:
    device_mockup()
        : device()
    {
    }

    explicit device_mockup(cl_device_id id, bool retain = true)
        : device(id, retain)
    {

    }

    device_mockup(const device &other)
        : device(other)
    {

    }

    device_mockup(device&& other) BOOST_NOEXCEPT
        : device(other)
    {

    }

    cl_device_type type() const
    {
        return device::gpu;
    }
};

}
}

struct Context {
    boost::compute::device        real_device;
    boost::compute::device_mockup device;

    boost::compute::context       context;
    boost::compute::command_queue queue;

    Context() :
        real_device ( boost::compute::system::default_device() ),
        device ( real_device ),
        context( boost::compute::context(device) ),
        queue  ( boost::compute::command_queue(context, device) )
    {

    }
};

BOOST_FIXTURE_TEST_SUITE(compute_test, Context)

#endif
