// Copyright (C) 2015 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef FOONATHAN_MEMORY_TEMPORARY_ALLOCATOR_HPP_INCLUDED
#define FOONATHAN_MEMORY_TEMPORARY_ALLOCATOR_HPP_INCLUDED

#include "allocator_traits.hpp"
#include "stack_allocator.hpp"

namespace foonathan { namespace memory
{    
	/// \brief A memory allocator for temporary allocations.
    /// \detail It is similar to \c alloca() but portable.
    /// It uses a \c thread_local \ref memory_stack<> for the allocation.<br>
    /// It is no \ref concept::RawAllocator, but the \ref allocator_traits are specialized for it.<br>
    /// \ingroup memory
    class temporary_allocator
    {
    public:        
        temporary_allocator(temporary_allocator &&other) noexcept;
        ~temporary_allocator() noexcept;
        
        temporary_allocator& operator=(temporary_allocator &&other) noexcept;
    
        /// \brief Allocates temporary memory of given size and alignment.
        /// \detail It will be deallocated when the allocator goes out of scope.
        void* allocate(std::size_t size, std::size_t alignment);
        
    private:
        temporary_allocator() noexcept;
        
        memory_stack<>::marker marker_;
        bool unwind_;
        
        friend temporary_allocator make_temporary_allocator() noexcept;
    };
    
    /// \brief Creates a new \ref temporary_allocator.
    /// \detail This is the only way to create to avoid accidental creation not on the stack.
    /// \relates temporary_allocator
    inline temporary_allocator make_temporary_allocator() noexcept
    {
        return {};
    }
    
    /// \brief Specialization of the \ref allocator_traits for \ref temporary_allocator.
    /// \detail This allows passing a pool directly as allocator to container types.
    /// \ingroup memory
    template <>
    class allocator_traits<temporary_allocator>
    {
    public:
        using allocator_type = temporary_allocator;
        using is_stateful = std::true_type;
        
        /// @{
        /// \brief Allocation function forward to the temporary allocator for array and node.
        static void* allocate_node(allocator_type &state, std::size_t size, std::size_t alignment)
        {
            assert(size <= max_node_size(state) && "invalid node size");
            return state.allocate(size, alignment);
        }
        
        static void* allocate_array(allocator_type &state, std::size_t count,
                                std::size_t size, std::size_t alignment)
        {
            return allocate_node(state, count * size, alignment);
        }
        /// @}
        
        /// @{
        /// \brief Deallocation functions do nothing, everything is freed on scope exit.
        static void deallocate_node(const allocator_type &,
                    void *, std::size_t, std::size_t) noexcept {}
        
        static void deallocate_array(const allocator_type &,
                    void *, std::size_t, std::size_t, std::size_t) noexcept {}
        /// @}
        
        /// @{
        /// \brief The maximum size is the equivalent of the capacity left in the next block of the internal \ref memory_stack<>.
        static std::size_t max_node_size(const allocator_type &state) noexcept;
        
        static std::size_t max_array_size(const allocator_type &state) noexcept
        {
            return max_node_size(state);
        }
        /// @}
        
        /// \brief There is no maximum alignment (except indirectly through \ref max_node_size()).
        static std::size_t max_alignment(const allocator_type &) noexcept
        {
            return std::size_t(-1);
        }
    };
}} // namespace foonathan::memory

#endif // FOONATHAN_MEMORY_TEMPORARY_ALLOCATOR_HPP_INCLUDED
