//---------------------------------------------------------------------------//
// Copyright (c) 2016 Jakub Szuppe <j.szuppe@gmail.com>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// See http://boostorg.github.com/compute for more information.
//---------------------------------------------------------------------------//

#include <iostream>

#include <boost/compute/core.hpp>
#include <boost/compute/container/vector.hpp>
#include <boost/compute/utility/program_cache.hpp>

void saxpy(const boost::compute::vector<float>& x,
           const boost::compute::vector<float>& y,
           const float a,
           boost::compute::command_queue& queue)
{
    std::string source =
        "__kernel void saxpy(__global float *x,"
        "                    __global float *y,"
        "                    const float a)"
        "{"
        "    const uint i = get_global_id(0);"
        "    y[i] = a * x[i] + y[i];"
        "}";

    boost::compute::context context =
        queue.get_context();

    // Online caching
    boost::shared_ptr<boost::compute::program_cache> global_cache =
        boost::compute::program_cache::get_global_cache(context);

    std::string options;
    std::string key = "__example_saxpy_";
    boost::compute::program program =
        global_cache->get_or_build(key, options, source, context);

    // create kernel
    boost::compute::kernel kernel = program.create_kernel("saxpy");
    // set arguments
    kernel.set_args(x.get_buffer(), y.get_buffer(), a); // C++11 VARIADIC TEMPLATES
    // run kernel
    boost::compute::event event =
        queue.enqueue_1d_range_kernel(kernel, 0, y.size(), 0);
    event.wait();
}

int main()
{
    // get the default command queue
    boost::compute::command_queue& queue =
        boost::compute::system::default_queue();

    // setup data
    boost::compute::vector<float> x = { 1.0f, 2.0f, 3.0f, 4.0f };
    boost::compute::vector<float> y = { 1.0f, 1.0f, 1.0f, 1.0f };
    const float a = 2.0f;

    queue.finish();

    // run saxpy
    saxpy(x, y, a, queue);

    // print out the results
    std::cout << "y: [" << y[0] << ", " // 2 * 1 + 1 = 3
                        << y[1] << ", " // 5
                        << y[2] << ", " // 7
                        << y[3] << "]" << std::endl; // 9

    return 0;
}
