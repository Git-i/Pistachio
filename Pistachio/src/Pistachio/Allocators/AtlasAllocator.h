#pragma once
#include "Pistachio\Core\Math.h"
namespace Pistachio
{
    enum class AllocatorFlags
    {
        None = 0,
        DownscaleOnFail = 1,
    };
    DEFINE_ENUM_FLAG_OPERATORS(AllocatorFlags);
    struct Region
    {
        iVector2 offset;
        iVector2 size;
    };
    struct hRegion
    {
        hiVector2 offset;
        hiVector2 size;
        hRegion() = default;
        hRegion(Region r) : offset(r.offset), size(r.size) {}
    };
    struct Entry
    {
        iVector2 val;
        bool used;
        Entry() : val(0, 0), used(false) {}
        Entry(iVector2 vec) :val(vec), used(false) {}
        Entry(iVector2 vec, bool _used) : val(vec), used(_used) {}
    };
    typedef std::vector<Entry> Row;
    class AtlasAllocator
    {
    public:
        AtlasAllocator(iVector2 max_size, iVector2 portion_size);
        uint32_t Allocate(iVector2 size, AllocatorFlags flags);
        void DeAllocate(uint32_t id);
        void DeFragment();
        Region GetRegion(std::uint32_t id);
        inline iVector2 GetPortionSize()const { return m_portion_size; }
        inline uint32_t GetTotalArea() const { return ((m_portion_size.x * m_portions[0].size()) * (m_portion_size.y * m_portions.size())); }
        inline iVector2 GetMaxSize() const { return iVector2(m_portions[0].size(), m_portions.size()); }
    private:
        uint32_t id = 0;
        std::unordered_map<uint32_t, Region> indexRegionMap;
        iVector2 m_portion_size = {};
        std::vector<Row> m_portions = {};
    };
}
