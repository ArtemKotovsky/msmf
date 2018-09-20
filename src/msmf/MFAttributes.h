#pragma once

#include "ComUtils.h"

namespace mf
{
    class MFAttributes
    {
    public:
        explicit MFAttributes(ComPtr<IMFAttributes> ptr);

        std::wstring GetString(const GUID& key) const;
        std::wstring GetGuidName(const GUID& key) const;

        uint32_t GetUint32(const GUID& key, uint32_t def = 0) const;
        HRESULT GetBlob(const GUID& key, std::vector<UINT8>& data) const;

        static std::wstring GuidToName(const GUID& guid);

    private:
        ComPtr<IMFAttributes> m_ptr;
    };
}
