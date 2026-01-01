// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#pragma once

#include "rsbl-assert.h"
#include "rsbl-core.h"
#include "rsbl-int-types.h"

// TODO: Method binder helper function
// TODO: Interest in using std::construct_at instead of placement new?
// TODO: Method version of Function constructor
// TODO: analog to std::max_align_t, perhaps __STDCPP_DEFAULT_NEW_ALIGNMENT__ ?
// TODO: do I need to forward the args inside m_invoker lambda?
// TODO: support 'emplacing' into existing Function? I don't think I care

namespace rsbl
{

// I' not entirely sure why I need this forward declaration, but I see this pattern all the time...
// I guess it's because we can't clarify the 'order' of ReturnType and ArgsTypes without the
// specialization that shows the function signature
// Interestingly, I am allowed to have the parameter pack as NOT last, because default arguments can
// be last. I might change this because...it's weird. But default template arg followed by parameter
// pack is also kinda weird
template <typename ReturnType, uint32 BufferSize = 32, typename... ArgsTypes>
class Function;

template <typename ReturnType, uint32 BufferSize, typename... ArgsTypes>
class Function<ReturnType(ArgsTypes...), BufferSize>
{
  public:
    Function() noexcept = default;

    ~Function()
    {
        Reset();
    }

    template <typename FunctorType>
    Function(FunctorType&& functor)
    {
        using StoredType = Decay<FunctorType>;
        static_assert(sizeof(StoredType) <= BufferSize, "Functor too large for Function buffer");

        m_invoker = [](void* functor_buffer, ArgsTypes... args) -> ReturnType {
            return (*static_cast<StoredType*>(functor_buffer))(args...);
        };

        m_destructor = [](void* functor_buffer) {
            static_cast<StoredType*>(functor_buffer)->~StoredType();
        };

        m_mover = [](void* dst, void* src) {
            new (dst) StoredType(rsblMove(*static_cast<StoredType*>(src)));
        };

        // I need to use placement new to manage the functor construction correctly
        new (m_buffer) StoredType(rsblForward(functor));
    }

    // Type for CallArguments is explicitly different from ArgsTypes to support conversion
    // of argument types. We can't use perfect forwarding in this case.
    template <typename... CallArguments>
    ReturnType operator()(CallArguments... call_args) const
    {
        rsblAssert(m_invoker != nullptr);
        return m_invoker(const_cast<void*>(&m_buffer[0]), call_args...);
    }

    // No copies
    Function(const Function&) = delete;
    Function& operator=(const Function&) = delete;

    // Moves allowed
    Function(Function&& rhs) noexcept
    {
        MoveFrom(rhs);
    }
    Function& operator=(Function&& rhs) noexcept
    {
        if (this != &rhs)
        {
            Reset();
            MoveFrom(rhs);
        }
        return *this;
    }

    // I don't love this implicit bool operator, but I see this pattern in other places too
    operator bool() const
    {
        return m_invoker != nullptr;
    }

    bool Valid() const
    {
        return m_invoker != nullptr;
    }

  protected:
    void Reset() noexcept
    {
        if (m_destructor != nullptr)
        {
            m_destructor(m_buffer);
        }
        m_invoker = nullptr;
        m_mover = nullptr;
        m_destructor = nullptr;
    }

    void MoveFrom(Function& rhs) noexcept
    {
        // I can check any of the function pointers, since they are set together
        if (rhs.m_mover == nullptr)
        {
            return;
        }

        m_invoker = rhs.m_invoker;
        m_mover = rhs.m_mover;
        m_destructor = rhs.m_destructor;

        m_mover(m_buffer, rhs.m_buffer);

        rhs.Reset();
    }

    // I think better to have the buffer first
    alignas(8) uint8 m_buffer[BufferSize]{};

    // I could just declare these are regular function pointers, but I think the using syntax
    // makes the member decls easier to read
    using InvokerType = ReturnType (*)(void*, ArgsTypes...);
    using MoverType = void (*)(void*, void*);
    using DestructorType = void (*)(void*);
    InvokerType m_invoker = nullptr;
    MoverType m_mover = nullptr;
    DestructorType m_destructor = nullptr;
};

template <auto MemberFunc>
struct MemberFuncWrapper;

// Non-const member function
template <typename ClassType,
          typename ReturnType,
          typename... ArgsTypes,
          ReturnType (ClassType::*MemberFunc)(ArgsTypes...)>
struct MemberFuncWrapper<MemberFunc>
{
    ClassType* object;

    ReturnType operator()(ArgsTypes... args) const
    {
        return (object->*MemberFunc)(args...);
    }
};

// Const member function
template <typename ClassType,
          typename ReturnType,
          typename... ArgsTypes,
          ReturnType (ClassType::*ConstMemberFunc)(ArgsTypes...) const>
struct MemberFuncWrapper<ConstMemberFunc>
{
    const ClassType* object;

    ReturnType operator()(ArgsTypes... args) const
    {
        return (object->*ConstMemberFunc)(args...);
    }
};

template <auto MemberFunc, typename ClassType>
auto BindMember(ClassType* obj)
{
    return MemberFuncWrapper<MemberFunc>{obj};
}

} // namespace rsbl