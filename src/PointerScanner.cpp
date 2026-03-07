#include "PointerScanner.h"
#include <queue>

PointerScanner::PointerScanner()
{
}

PointerScanner::~PointerScanner()
{
}

bool PointerScanner::ReadPointer(HANDLE hProcess, uintptr_t address, uintptr_t& outValue)
{
    SIZE_T bytesRead = 0;
    return ReadProcessMemory(hProcess,
        reinterpret_cast<LPCVOID>(address),
        &outValue,
        sizeof(uintptr_t),
        &bytesRead) && bytesRead == sizeof(uintptr_t);
}

bool PointerScanner::IsValidPointer(uintptr_t value)
{
#ifdef _WIN64
	return value >= 0x10000 && value <= 0x00007FFFFFFFFFFF && (value % 4 == 0);
#else
	return value >= 0x10000 && value <= 0x7FFFFFFF && (value % 4 == 0);
#endif
}

std::vector<PointerCandidate> PointerScanner::FindPointersToAddress(
	HANDLE hProcess, const MemoryRegions& regions, uintptr_t target, uintptr_t maxOffset)
{
	std::vector<PointerCandidate> candidates;

	const auto& regionList = regions.GetRegions();


    for (const auto& region : regionList)
    {
        std::vector<uint8_t> buffer(region.regionSize);
        SIZE_T bytesRead = 0;

        if (!ReadProcessMemory(hProcess,
            reinterpret_cast<LPCVOID>(region.baseAddress),
            buffer.data(),
            region.regionSize,
            &bytesRead))
        {
            continue;
        }

        // Step by pointer size (8 bytes on x64)
        for (size_t i = 0; i + sizeof(uintptr_t) <= bytesRead; i += sizeof(uintptr_t))
        {
            uintptr_t value = *reinterpret_cast<uintptr_t*>(&buffer[i]);

            if (!IsValidPointer(value))
                continue;

            // Check if this value points into [target - maxOffset, target]
            if (value <= target && (target - value) <= maxOffset)
            {
                uintptr_t candidateAddr = reinterpret_cast<uintptr_t>(region.baseAddress) + i;
                uintptr_t offset = target - value;
                candidates.push_back({ candidateAddr, offset });
            }
        }
    }

    return candidates;
}


uintptr_t PointerScanner::FindOwningModuleBase(HANDLE hProcess, uintptr_t address)
{
	return uintptr_t();
}

void PointerScanner::Scan(
	ProcessHandle& handle, const MemoryRegions& regions, 
	uintptr_t target, uintptr_t maxOffset, int maxDepth)
{
    results.clear();

    auto modules = handle.GetModuleList();

    std::vector<Node> nodes;
    std::unordered_set<uintptr_t> visited;
    std::queue<WorkItem> workQueue;

    workQueue.push({ target, 0, -1 });

    while (!workQueue.empty())
    {
        auto work = workQueue.front();
        workQueue.pop();

        if (visited.count(work.target))
        {
            continue;
        }

        visited.insert(work.target);

        auto candidates = FindPointersToAddress(handle.GetHandle(), regions, work.target, maxOffset);

        for (const auto& candidate : candidates)
        {
            int nodeIdx = (int)nodes.size();
            nodes.push_back({ candidate.address, work.nodeIdx, candidate.offset });

            // Check if this candidate is in a module (static root)
            for (const auto& mod : modules)
            {
                if (candidate.address >= mod.base && candidate.address < mod.base + mod.size)
                {
                    // Found a static root, reconstruct the chain
                    PointerChain chain;
                    chain.moduleOffset = mod.base;
                    chain.moduleName = mod.name;

                    // Walk back up the node tree
                    int idx = nodeIdx;
                    while (idx != -1)
                    {
                        chain.offsets.push_back(nodes[idx].offset);
                        idx = nodes[idx].parent;
                    }

                    // offsets are leaf to root, reverse for root to leaf
                    std::reverse(chain.offsets.begin(), chain.offsets.end());
                    chain.offsets.insert(chain.offsets.begin(), candidate.address - mod.base);

                    results.push_back(chain);
                    break;
                }
            }

            // If not at max depth, keep searching
            if (work.depth < maxDepth)
            {
                workQueue.push({ candidate.address, work.depth + 1, nodeIdx });
            }
        }
    }

    printf("Pointer scan complete. Found %zu chains.\n", results.size());
}
