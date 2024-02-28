
#include "ddwin/stdafx.h"
#include "ddwin/graphics/d2d/ddd2d_factory.h"
#include "ddbase/dderror_code.h"

namespace NSP_DD {
static ComPtr<IDWriteFactory> s_dwrite_factory;

bool init_dwrite_factory()
{
    if (s_dwrite_factory != NULL) {
        return true;
    }

    HRESULT hr = ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &s_dwrite_factory);
    if (FAILED(hr) || s_dwrite_factory == NULL) {
        dderror_code::set_last_error(hr);
        return false;
    }

    return true;
}

void uninit_dwrite_factory()
{
    s_dwrite_factory.Reset();
}

ComPtr<IDWriteFactory> ddd2d_factory::get_dwrite_factory()
{
    return s_dwrite_factory;
}

ComPtr<IDWriteTextFormat> ddd2d_factory::CreateTextFormat(const std::wstring& fontFamilyName, FLOAT fontSize, DWRITE_FONT_WEIGHT fontWeight, DWRITE_FONT_STYLE fontStyle)
{
    if (s_dwrite_factory == NULL) {
        return NULL;
    }

    ComPtr<IDWriteTextFormat> text_format;
    HRESULT hr = s_dwrite_factory->CreateTextFormat(
        fontFamilyName.c_str(),
        nullptr,
        fontWeight,
        fontStyle,
        DWRITE_FONT_STRETCH_NORMAL,
        fontSize,
        L"en-us",
        text_format.ReleaseAndGetAddressOf()
    );

    if (FAILED(hr)) {
        dderror_code::set_last_error(hr);
    }
    return text_format;
}
} // namespace NSP_DD
