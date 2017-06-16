//---------------------------------------------------------------------------//
// Copyright (c) 2017 Jakub Szuppe <j.szuppe@gmail.com>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// See http://boostorg.github.com/compute for more information.
//---------------------------------------------------------------------------//

#ifndef BOOST_COMPUTE_ITERATOR_DISTRIBUTED_ITERATOR_HPP
#define BOOST_COMPUTE_ITERATOR_DISTRIBUTED_ITERATOR_HPP

#include <cstddef>
#include <iterator>
#ifndef BOOST_COMPUTE_NO_HDR_INITIALIZER_LIST
#include <initializer_list>
#endif

#include <boost/config.hpp>
#include <boost/iterator/iterator_adaptor.hpp>

#include <boost/compute/distributed/context.hpp>
#include <boost/compute/distributed/command_queue.hpp>

namespace boost {
namespace compute {

// forward declaration for distributed_iterator<Iterator>
template<class Iterator>
class distributed_iterator;

namespace detail {

// helper class which defines the iterator_adaptor super-class
// type for distributed_iterator
template<class Iterator>
class distributed_iterator_base
{
public:
    typedef ::boost::iterator_adaptor<
        ::boost::compute::distributed_iterator<Iterator>,
        Iterator
    > type;
};

} // end detail namespace

template<class Iterator>
class distributed_iterator
    : public detail::distributed_iterator_base<Iterator>::type
{
public:
    typedef typename
        detail::distributed_iterator_base<Iterator>::type super_type;
    typedef typename super_type::value_type value_type;
    typedef typename super_type::reference reference;
    typedef typename super_type::base_type base_type;
    typedef typename super_type::difference_type difference_type;

    distributed_iterator(std::vector<Iterator> begins,
                         std::vector<Iterator> ends)
        : m_begins(begins),
          m_ends(ends),
          m_current_iterator_id(0),
          m_current_iterator(begins[0])
    {

    }

    #ifndef BOOST_COMPUTE_NO_HDR_INITIALIZER_LIST
    #endif


    distributed_iterator<Iterator>&
    operator=(const distributed_iterator<Iterator> &other)
    {
        m_current_iterator = other.m_current_iterator;
        m_current_iterator_id = other.m_current_iterator_id;
        m_begins = other.m_begins;
        m_ends = other.m_ends;
    }

    ~distributed_iterator()
    {
    }

    const size_t parts() const
    {
        return m_begins.size();
    }

    Iterator get()
    {
        return m_current_iterator;
    }

    Iterator get_begin(const size_t part_index) const
    {
        if(part_index < m_current_iterator_id)
        {
            return m_ends[part_index];
        }
        else if(part_index == m_current_iterator_id)
        {
            return m_current_iterator;
        }
        return m_begins[part_index];
    }

    Iterator get_end(const size_t part_index) const
    {
        return m_ends[part_index];
    }

    value_type read(distributed::command_queue &queue) const
    {
        return m_current_iterator.read(queue.get(m_current_iterator_id));
    }

    void write(const value_type &value, distributed::command_queue &queue)
    {
        m_current_iterator.write(value, queue.get(m_current_iterator_id));
    }

private:
    friend class ::boost::iterator_core_access;

    /// \internal_
    reference dereference() const
    {
        return *m_current_iterator;
    }

    /// \internal_
    bool equal(const distributed_iterator<Iterator> &other) const
    {
        return
            parts() == other.parts()
                && m_current_iterator == other.m_current_iterator
                && m_begins == other.m_begins
                && m_ends == other.m_ends;
    }

    /// \internal_
    void increment()
    {
        if(m_current_iterator != m_ends[m_current_iterator_id])
            m_current_iterator++;

        if(m_current_iterator == m_ends[m_current_iterator_id])
        {
            // If it's NOT the end of the last iterator,
            // then go to the next iterator
            if(m_current_iterator_id != (parts() - 1))
            {
                m_current_iterator_id++;
                m_current_iterator = m_begins[m_current_iterator_id];
            }
        }
    }

    /// \internal_
    void decrement()
    {
        if(m_current_iterator != m_begins[m_current_iterator_id])
            m_current_iterator--;

        if(m_current_iterator == m_begins[m_current_iterator_id])
        {
            // If it's NOT the begin of the first iterator, then
            // go to the previous iterator
            if(m_current_iterator_id != 0)
            {
                m_current_iterator_id--;
                m_current_iterator = (m_ends[m_current_iterator_id]--);
            }
        }
    }

    /// \internal_
    void advance(difference_type n)
    {
        // TODO
    }

    /// \internal_
    difference_type distance_to(const distributed_iterator<Iterator> &other) const
    {
        // TODO
        return difference_type(0);
    }

private:
    std::vector<Iterator> m_begins;
    std::vector<Iterator> m_ends;

    unsigned int m_current_iterator_id;
    Iterator m_current_iterator;
};

// template<class T>
// inline distributed_iterator<T>
// make_distributed_iterator(const buffer &buffer, size_t index = 0)
// {
//     return distributed_iterator<T>(buffer, index);
// }

} // end compute namespace
} // end boost namespace

#endif // BOOST_COMPUTE_ITERATOR_DISTRIBUTED_ITERATOR_HPP
