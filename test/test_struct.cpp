//---------------------------------------------------------------------------//
// Copyright (c) 2013-2014 Kyle Lutz <kyle.r.lutz@gmail.com>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// See http://boostorg.github.com/compute for more information.
//---------------------------------------------------------------------------//

#include <boost/compute/config.hpp>

#define BOOST_TEST_MODULE TestStruct
#include <boost/test/unit_test.hpp>

#include <boost/compute/function.hpp>
#include <boost/compute/algorithm/find_if.hpp>
#include <boost/compute/algorithm/transform.hpp>
#include <boost/compute/container/vector.hpp>
#include <boost/compute/functional/field.hpp>
#include <boost/compute/types/struct.hpp>
#include <boost/compute/type_traits/type_name.hpp>
#include <boost/compute/type_traits/type_definition.hpp>
#include <boost/compute/utility/source.hpp>
#include <boost/compute/utility/program_cache.hpp>
#include <boost/compute/detail/sha1.hpp>

namespace compute = boost::compute;

// example code defining an atom class
//namespace chemistry {
//
//struct Atom
//{
//    Atom(float _x, float _y, float _z, int _number)
//        : x(_x), y(_y), z(_z), number(_number)
//    {
//    }
//
//    float x;
//    float y;
//    float z;
//    int number;
//};
//
//} // end chemistry namespace

// adapt the chemistry::Atom class
//BOOST_COMPUTE_ADAPT_STRUCT(chemistry::Atom, Atom, (x, y, z, number))
//
//struct StructWithArray {
//    int value;
//    int array[3];
//};
//
//BOOST_COMPUTE_ADAPT_STRUCT(StructWithArray, StructWithArray, (value, array))

struct myStruct
{
    myStruct(int x)
        : id_attributes(10 + x,x)
        , c(0.0f,0.0f,0.0f,10000000.0f)
        , d(10.0f,10.0f,10.0f,10000.0f)
    {
    }

//    myStruct(int x)
//        : id_attributes(10 + x,x, x,x)
//        , c(0.0f,0.0f,0.0f,10000000.0f)
//        , d(10.0f,10.0f,10.0f,10000.0f)
//    {
//    }

    myStruct(const myStruct& x)
        : id_attributes(x.id_attributes)
        , c(x.c)
        , d(x.d)
    {
    }

    myStruct()
        : id_attributes(0)
        , c(0.0f)
        , d(0.0f)
    {
    }

    boost::compute::int2_    id_attributes;
//    boost::compute::int4_    id_attributes;
    boost::compute::float4_ c;
    boost::compute::float4_ d;
};

BOOST_COMPUTE_ADAPT_STRUCT(myStruct, myStruct, (id_attributes, c, d))

#include "check_macros.hpp"
#include "context_setup.hpp"

#define get_typedef boost::compute::type_definition<myStruct>() + "\n"

BOOST_AUTO_TEST_CASE(atom_type_name)
{

    typedef    std::vector<myStruct> HostData;
    typedef    boost::compute::vector<myStruct>    GPUData;


        HostData data;
        data.push_back(myStruct(1));
        data.push_back(myStruct(2));
        data.push_back(myStruct(3));
        //Print A

        std::cout << "Print A:" << std::endl;
        for(size_t i = 0; i < data.size(); i++) {
            std::cout << data[i].id_attributes << " " << data[i].c << " " << data[i].d << std::endl;
        }
        std::cout << std::endl;

        GPUData gpudata(data.size(), context);
        boost::compute::copy(data.begin(), data.end(), gpudata.begin(), queue);
        queue.finish();
        //Print B

        std::cout << "Print B:" << std::endl;
        for(size_t i = 0; i < gpudata.size(); i++) {
            myStruct j = gpudata[i];
            std::cout << j.id_attributes << " " << j.c << " " << j.d << std::endl;
        }
        std::cout << std::endl;

        //Print in kernel
        std::string source = BOOST_COMPUTE_STRINGIZE_SOURCE(
            __kernel void custom_kernel(__global const myStruct *input, __global myStruct *output)
            {
                const uint i = get_global_id(0);
                const __global myStruct *testy = &input[i];

                output[i].d.xyz = testy->c.xyz;

                float3 newval = testy->c.xyz - (float3)(5.f);

                output[i].c.xyz = newval;

                myStruct s = output[i];
                printf("int2(%d, %d)", s.id_attributes.x, s.id_attributes.y);
//                printf("int4(%d, %d, %d, %d) ", s.id_attributes.x, s.id_attributes.y, s.id_attributes.z, s.id_attributes.w);
                printf("float4(%g, %g, %g, %g) ", s.c.x, s.c.y, s.c.z, s.c.w);
                printf("float4(%g, %g, %g, %g)\n",  s.d.x, s.d.y, s.d.z, s.d.w);
            }
        );

        source = get_typedef + source;

//        boost::compute::program program = boost::compute::program::build_with_source(source, context, "-cl-std=CL1.1");
        boost::compute::program program = boost::compute::program::build_with_source(source, context);

        boost::compute::kernel custom_kernel = program.create_kernel("custom_kernel");
        custom_kernel.set_arg(0, gpudata);
        custom_kernel.set_arg(1, gpudata);

        queue.enqueue_1d_range_kernel(custom_kernel, 0, gpudata.size(), 1);
        queue.finish();

        // Print C
        std::cout << std::endl;
        std::cout << "Print C:" << std::endl;
        for(size_t i = 0; i < gpudata.size(); i++) {
            myStruct j = gpudata[i];
            std::cout << j.id_attributes << " " << j.c << " " << j.d << std::endl;
        }
        std::cout << std::endl;

        boost::compute::copy(gpudata.begin(), gpudata.end(), data.begin(), queue);
        queue.finish();

        // Print D
        std::cout << "Print D:" << std::endl;
        for(size_t i = 0; i < data.size(); i++) {
            std::cout << data[i].id_attributes << " " << data[i].c << " " << data[i].d << std::endl;
        }
        std::cout << std::endl;
}

//BOOST_AUTO_TEST_CASE(atom_struct)
//{
//    std::vector<chemistry::Atom> atoms;
//    atoms.push_back(chemistry::Atom(1.f, 0.f, 0.f, 1));
//    atoms.push_back(chemistry::Atom(0.f, 1.f, 0.f, 1));
//    atoms.push_back(chemistry::Atom(0.f, 0.f, 0.f, 8));
//
//    compute::vector<chemistry::Atom> vec(atoms.size(), context);
//    compute::copy(atoms.begin(), atoms.end(), vec.begin(), queue);
//
//    // find the oxygen atom
//    BOOST_COMPUTE_FUNCTION(bool, is_oxygen, (chemistry::Atom atom),
//    {
//        return atom.number == 8;
//    });
//
//    compute::vector<chemistry::Atom>::iterator iter =
//        compute::find_if(vec.begin(), vec.end(), is_oxygen, queue);
//    BOOST_CHECK(iter == vec.begin() + 2);
//
//    // copy the atomic numbers to another vector
//    compute::vector<int> atomic_numbers(vec.size(), context);
//    compute::transform(
//        vec.begin(), vec.end(),
//        atomic_numbers.begin(),
//        compute::field<int>("number"),
//        queue
//    );
//    CHECK_RANGE_EQUAL(int, 3, atomic_numbers, (1, 1, 8));
//}
//
//BOOST_AUTO_TEST_CASE(custom_kernel)
//{
//    std::vector<chemistry::Atom> data;
//    data.push_back(chemistry::Atom(1.f, 0.f, 0.f, 1));
//    data.push_back(chemistry::Atom(0.f, 1.f, 0.f, 1));
//    data.push_back(chemistry::Atom(0.f, 0.f, 0.f, 8));
//
//    compute::vector<chemistry::Atom> atoms(data.size(), context);
//    compute::copy(data.begin(), data.end(), atoms.begin(), queue);
//
//    std::string source = BOOST_COMPUTE_STRINGIZE_SOURCE(
//        __kernel void custom_kernel(__global const Atom *atoms,
//                                    __global float *distances)
//        {
//            const uint i = get_global_id(0);
//            const __global Atom *atom = &atoms[i];
//
//            const float4 center = { 0, 0, 0, 0 };
//            const float4 position = { atom->x, atom->y, atom->z, 0 };
//
//            distances[i] = distance(position, center);
//        }
//    );
//
//    // add type definition for Atom to the start of the program source
//    source = compute::type_definition<chemistry::Atom>() + "\n" + source;
//
//    compute::program program =
//        compute::program::build_with_source(source, context);
//
//    compute::vector<float> distances(atoms.size(), context);
//
//    compute::kernel custom_kernel = program.create_kernel("custom_kernel");
//    custom_kernel.set_arg(0, atoms);
//    custom_kernel.set_arg(1, distances);
//
//    queue.enqueue_1d_range_kernel(custom_kernel, 0, atoms.size(), 1);
//}
//
//// Creates a StructWithArray containing 'x', 'y', 'z'.
//StructWithArray make_struct_with_array(int x, int y, int z)
//{
//    StructWithArray s;
//    s.value = 0;
//    s.array[0] = x;
//    s.array[1] = y;
//    s.array[2] = z;
//    return s;
//}
//
//BOOST_AUTO_TEST_CASE(struct_with_array)
//{
//    compute::vector<StructWithArray> structs(context);
//
//    structs.push_back(make_struct_with_array(1, 2, 3), queue);
//    structs.push_back(make_struct_with_array(4, 5, 6), queue);
//    structs.push_back(make_struct_with_array(7, 8, 9), queue);
//
//    BOOST_COMPUTE_FUNCTION(int, sum_array, (StructWithArray x),
//    {
//        return x.array[0] + x.array[1] + x.array[2];
//    });
//
//    compute::vector<int> results(structs.size(), context);
//    compute::transform(
//        structs.begin(), structs.end(), results.begin(), sum_array, queue
//    );
//    CHECK_RANGE_EQUAL(int, 3, results, (6, 15, 24));
//}

BOOST_AUTO_TEST_SUITE_END()
