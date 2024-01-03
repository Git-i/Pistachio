#include "ptpch.h"
#include "AtlasAllocator.h"
namespace Pistachio
{
    float CalculateFragmentationMetric(const AtlasAllocator& allocator, const iVector2& slotPosition, const iVector2& allocationSize) {
        int numEmptyBlocks = 0;
        int totalEmptyArea = 0;

        // Check for empty blocks to the left
        if (slotPosition.x > 0) {
            numEmptyBlocks++;
            totalEmptyArea += slotPosition.x * allocator.GetPortionSize().y;
        }

        // Check for empty blocks above
        if (slotPosition.y > 0) {
            numEmptyBlocks++;
            totalEmptyArea += slotPosition.y * allocator.GetPortionSize().x;
        }

        // Check for empty blocks to the right
        int potentialRightBlockWidth = allocator.GetMaxSize().x - slotPosition.x - allocationSize.x;
        if (potentialRightBlockWidth > 0) {
            numEmptyBlocks++;
            totalEmptyArea += potentialRightBlockWidth * allocator.GetPortionSize().y;
        }

        // Check for empty blocks below
        int potentialBottomBlockHeight = allocator.GetMaxSize().y - slotPosition.y - allocationSize.y;
        if (potentialBottomBlockHeight > 0) {
            numEmptyBlocks++;
            totalEmptyArea += potentialBottomBlockHeight * allocator.GetPortionSize().x;
        }

        // Calculate a weighted average of the number of blocks and empty area
        // (Adjust weights as needed based on your priorities)
        return 0.4f * numEmptyBlocks + 0.6f * totalEmptyArea / allocator.GetTotalArea();
    }
    AtlasAllocator::AtlasAllocator(iVector2 max_size, iVector2 portion_size)
    {
        PT_CORE_ASSERT(!(max_size.x % portion_size.x));
        PT_CORE_ASSERT(!(max_size.y % portion_size.y));
        uint32_t numRowElements = max_size.x / portion_size.x;
        uint32_t numRows = max_size.y / portion_size.y;
        uint32_t numPortions = 0;
        m_portion_size = portion_size;
        for (uint32_t i = 0; i < numRows; i++)
        {
            m_portions.emplace_back();
            Row& current_row = m_portions[m_portions.size() - 1];
            for (uint32_t j = 0; j < numRowElements; j++)
            {
                current_row.emplace_back(iVector2{ j * portion_size.y, i * portion_size.x });
                numPortions++;
                //PrintVec2({i * portion_size.x, j * portion_size.y});
            }
        }
        //std::cout << numPortions << std::endl;
    }
    bool IsRowAvailable(Row& row, uint32_t offsetFromRowStart, uint32_t span)
    {
        bool retVal = true;

        for (int k = 0; k < span; k++)
        {
            if (row[k + offsetFromRowStart].used)
            {
                retVal = false;
                break;
            }
        }
        return retVal;
    }
    Region AtlasAllocator::Allocate(iVector2 size, AllocatorFlags flags)
    {
        std::vector<iVector2> valid_positions;
        iVector2 bestPos = { 0,0 };
        float bestScore = FLT_MAX;
        iVector2 numPortions = size / m_portion_size;
        for (int i = 0; i < m_portions.size(); i++)
        {
            Row& row = m_portions[i];
            //Per entry
            for (int j = 0; j < row.size(); j++)
            {
                if ((row.size() - j) < numPortions.x)
                    continue;
                bool Avail = true;
                for (int k = 0; k < numPortions.y; k++)
                {
                    if (i + k >= m_portions.size())
                        break;
                    Avail = IsRowAvailable(m_portions[i + k], j, numPortions.x);
                    if (Avail == false)
                        break;
                }
                if (Avail == false) continue;
                //if we dont continue set all entries to used
                valid_positions.push_back(row[j].val);

            }
        }
        bool allocated = false;
        iVector2 portion_size = { 0,0 };
        //compare all allocations and pick best
        for (auto& position : valid_positions)
        {
            float score = CalculateFragmentationMetric(*this, position, size);
            if (score < bestScore)
            {
                bestScore = score;
                bestPos = position;
                allocated = true;
            }
        }
        if (allocated == true)
        {
            for (int k = 0; k < numPortions.y; k++)
            {
                for (int l = 0; l < numPortions.x; l++)
                {
                    m_portions[bestPos.y / m_portion_size.y + k][bestPos.x / m_portion_size.x + l].used = true;
                }
            }
            portion_size = size;
        }
        else if ((flags & AllocatorFlags::DownscaleOnFail) != AllocatorFlags::None)
        {
            if (size.x == m_portion_size.x && size.y == m_portion_size.y)
                return { bestPos , portion_size };
            return Allocate(size / iVector2{ 2,2 }, AllocatorFlags::DownscaleOnFail);
        }
        return { bestPos , portion_size };
    }

    void AtlasAllocator::DeAllocate(Region region)
    {
        for (int k = 0; k < region.size.y / m_portion_size.y; k++)
        {
            for (int l = 0; l < region.size.x / m_portion_size.x; l++)
            {
                m_portions[region.offset.y / m_portion_size.y + k][region.offset.x / m_portion_size.x + l].used = false;
            }
        }
    }

}
