#include <bit>  
#include <compare> 
#include <cstdint>
#include <iostream>

template <typename T>
class CompressedSharedPtr
{
public:
    CompressedSharedPtr() = default;

    explicit CompressedSharedPtr(T* ptr)
    {
        if (ptr)
        {
            uintptr_t initial = (PtrToInt(ptr) & PTR_MASK) | (static_cast<uintptr_t>(1) << PTR_BITS);
            handle = initial;
        }
    }

    CompressedSharedPtr(CompressedSharedPtr const& other)
    {
        handle = other.handle;
        if (GetPointer())
        {
            IncrementRefCount();
        }
    }

    CompressedSharedPtr(CompressedSharedPtr&& other) noexcept
    {
        handle = other.handle;
        other.handle = 0;
    }

    CompressedSharedPtr& operator=(CompressedSharedPtr const& other)
    {
        if (this != &other)
        {
            ReleaseCurrent();
            handle = other.handle;
            if (GetPointer())
            {
                IncrementRefCount();
            }
        }
        return *this;
    }

    CompressedSharedPtr& operator=(CompressedSharedPtr&& other) noexcept
    {
        if (this != &other)
        {
            ReleaseCurrent();
            handle = other.handle;
            other.handle = 0;
        }
        return *this;
    }

    ~CompressedSharedPtr()
    {
        ReleaseCurrent();
    }

    T* Get() const
    {
        return GetPointer();
    }

    T& operator*() const
    {
        return *Get();
    }

    T* operator->() const
    {
        return Get();
    }

    uint32_t UseCount() const
    {
        return GetRefCount();
    }

    auto operator<=>(CompressedSharedPtr const& rhs) const noexcept
    {
        return GetPointer() <=> rhs.GetPointer();
    }

    bool operator==(CompressedSharedPtr const& rhs) const noexcept
    {
        return GetPointer() == rhs.GetPointer();
    }

private:
    void ReleaseCurrent()
    {
        if (T* ptr = GetPointer())
        {
            DecrementRefCount();
            if (GetRefCount() == 0)
            {
                delete ptr;
                handle = 0;
            }
        }
    }

private:
    static constexpr int PTR_BITS = 42;
    static constexpr uintptr_t PTR_MASK = (1ULL << PTR_BITS) - 1;

    static uintptr_t PtrToInt(T* ptr) noexcept
    {
        return std::bit_cast<uintptr_t>(ptr);
    }

    static T* IntToPtr(uintptr_t val) noexcept
    {
        return std::bit_cast<T*>(val);
    }

    T* GetPointer() const
    {
        uintptr_t ptrPart = handle & PTR_MASK;
        return IntToPtr(ptrPart);
    }

    uint32_t GetRefCount() const
    {
        return static_cast<uint32_t>(handle >> PTR_BITS);
    }

    void SetPointer(T* ptr)
    {
        uint32_t currentRef = GetRefCount();
        uintptr_t newHandle = (ptrToInt(ptr) & PTR_MASK) | (static_cast<uintptr_t>(currentRef) << PTR_BITS);
        handle = newHandle;
    }

    void SetRefCount(uint32_t refCount)
    {
        uintptr_t ptrPart = handle & PTR_MASK;
        uintptr_t newHandle = ptrPart | (static_cast<uintptr_t>(refCount) << PTR_BITS);
        handle = newHandle;
    }

    void IncrementRefCount()
    {
        SetRefCount(GetRefCount() + 1);
    }

    void DecrementRefCount()
    {
        SetRefCount(GetRefCount() - 1);
    }

private:
    uintptr_t handle = 0;
};

struct Foo
{
    Foo()  { std::cout << "Foo constructed\n"; }
    ~Foo() { std::cout << "Foo destructed\n"; }
    void hello() const { std::cout << "Foo hello\n"; }
};

int main()
{
    {
        CompressedSharedPtr<Foo> sp1(new Foo());
        std::cout << "sp1.UseCount() = " << sp1.UseCount() << std::endl;

        {
            CompressedSharedPtr<Foo> sp2 = sp1;
            std::cout << "sp1.UseCount() = " << sp1.UseCount() << std::endl;
            std::cout << "sp2.UseCount() = " << sp2.UseCount() << std::endl;

            sp2->hello();

            {
                CompressedSharedPtr<Foo> sp3(sp2);
                std::cout << "sp2.UseCount() = " << sp2.UseCount() << std::endl;
                std::cout << "sp3.UseCount() = " << sp3.UseCount() << std::endl;
            }
            std::cout << "sp2.UseCount() after sp3 is out of scope = " << sp2.UseCount() << std::endl;
        }
        std::cout << "sp1.UseCount() after sp2 is out of scope = " << sp1.UseCount() << std::endl;
    }

    return 0;
}