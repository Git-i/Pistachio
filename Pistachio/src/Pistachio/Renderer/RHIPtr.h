#pragma once
namespace Pistachio
{
    template <typename T>
    class RHIPtr
    {
    public:
        RHIPtr()
        {
            ptr = nullptr;
        }
        RHIPtr(const RHIPtr& other)
        {
            ptr = other.ptr;
            ptr->Hold();
        }
        RHIPtr(T* other)
        {
            ptr = other;
        }
        RHIPtr(RHIPtr&& other) noexcept
        {
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        ~RHIPtr()
        {
            SafeRelease();
            ptr = nullptr;
        }
        void operator=(T* other)
        {
            SafeRelease();
            ptr = other;
        }
        void operator=(std::nullptr_t _ptr)
        {
            SafeRelease();
            ptr = _ptr;
        }
        T* operator->() const
        {
            return ptr;
        }
        T* Get() const
        {
            return ptr;
        }
        T** GetAddressOf()
        {
            return &ptr;
        }
        T** operator&()
        {
            SafeRelease();
            return &ptr;
        }
    private:
        void SafeRelease()
        {
            if(ptr && ptr->GetRefCount())
                ptr->Release();
        }
        T* ptr;
    };
}


