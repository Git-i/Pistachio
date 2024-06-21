#pragma once
namespace Pistachio
{
    template <typename T>
    class RHI::Ptr
    {
    public:
        RHI::Ptr()
        {
            ptr = nullptr;
        }
        RHI::Ptr(const RHI::Ptr& other)
        {
            ptr = other.ptr;
            SafeHold();
        }
        RHI::Ptr(T* other)
        {
            ptr = other;
        }
        RHI::Ptr(RHI::Ptr&& other) noexcept
        {
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        ~RHI::Ptr()
        {
            SafeRelease();
            ptr = nullptr;
        }
        void operator=(const RHI::Ptr<T>& other)
        {
            ptr = other.ptr;
            SafeHold();
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
        void SafeHold()
        {
            if (ptr) ptr->Hold();
        }
        T* ptr;
    };
}


