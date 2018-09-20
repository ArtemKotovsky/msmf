#include "stdafx.h"
#include "MFAttributes.h"

#ifndef IF_GUID_EQUAL_RETURN
#define IF_GUID_EQUAL_RETURN(val) if(val == guid) return L#val
#endif

namespace mf
{
    MFAttributes::MFAttributes(ComPtr<IMFAttributes> ptr)
        : m_ptr(ptr)
    {
    }

    std::wstring MFAttributes::GetString(const GUID& key) const
    {
        UINT32 cchName = 0;
        utils::TaskMemFreeTraits<LPWSTR>::Handle str;

        HRESULT hr = m_ptr->GetAllocatedString(
            key,
            str.ReleaseAndGetAddressOf(),
            &cchName);

        std::wstring result;

        if (SUCCEEDED(hr))
        {
            result = str.Get();
        }

        return result;
    }

    std::wstring MFAttributes::GetGuidName(const GUID& key) const
    {
        GUID value;
        HRESULT hr = m_ptr->GetGUID(key, &value);

        if (SUCCEEDED(hr))
        {
            return GuidToName(value);
        }

        return L"<not found>";
    }

    uint32_t MFAttributes::GetUint32(const GUID& key, uint32_t def) const
    {
        UINT32 value = 0;
        HRESULT hr = m_ptr->GetUINT32(key, &value);

        if (SUCCEEDED(hr))
        {
            return value;
        }

        return def;
    }

    HRESULT MFAttributes::GetBlob(const GUID& key, std::vector<UINT8>& data) const
    {
        UINT32 cbBlob = 0;
        HRCHK(m_ptr->GetBlobSize(key, &cbBlob));

        data.resize(cbBlob);
        HRCHK(m_ptr->GetBlob(key, data.data(), cbBlob, &cbBlob));

        return S_OK;
    }

    std::wstring MFAttributes::GuidToName(const GUID& guid)
    {
        IF_GUID_EQUAL_RETURN(MF_MT_MAJOR_TYPE);
        IF_GUID_EQUAL_RETURN(MF_MT_SUBTYPE);
        IF_GUID_EQUAL_RETURN(MF_MT_ALL_SAMPLES_INDEPENDENT);
        IF_GUID_EQUAL_RETURN(MF_MT_FIXED_SIZE_SAMPLES);
        IF_GUID_EQUAL_RETURN(MF_MT_COMPRESSED);
        IF_GUID_EQUAL_RETURN(MF_MT_SAMPLE_SIZE);
        IF_GUID_EQUAL_RETURN(MF_MT_WRAPPED_TYPE);
        IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_NUM_CHANNELS);
        IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_SAMPLES_PER_SECOND);
        IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_FLOAT_SAMPLES_PER_SECOND);
        IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_AVG_BYTES_PER_SECOND);
        IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_BLOCK_ALIGNMENT);
        IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_BITS_PER_SAMPLE);
        IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_VALID_BITS_PER_SAMPLE);
        IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_SAMPLES_PER_BLOCK);
        IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_CHANNEL_MASK);
        IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_FOLDDOWN_MATRIX);
        IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_WMADRC_PEAKREF);
        IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_WMADRC_PEAKTARGET);
        IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_WMADRC_AVGREF);
        IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_WMADRC_AVGTARGET);
        IF_GUID_EQUAL_RETURN(MF_MT_AUDIO_PREFER_WAVEFORMATEX);
        IF_GUID_EQUAL_RETURN(MF_MT_AAC_PAYLOAD_TYPE);
        IF_GUID_EQUAL_RETURN(MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION);
        IF_GUID_EQUAL_RETURN(MF_MT_FRAME_SIZE);
        IF_GUID_EQUAL_RETURN(MF_MT_FRAME_RATE);
        IF_GUID_EQUAL_RETURN(MF_MT_FRAME_RATE_RANGE_MAX);
        IF_GUID_EQUAL_RETURN(MF_MT_FRAME_RATE_RANGE_MIN);
        IF_GUID_EQUAL_RETURN(MF_MT_PIXEL_ASPECT_RATIO);
        IF_GUID_EQUAL_RETURN(MF_MT_DRM_FLAGS);
        IF_GUID_EQUAL_RETURN(MF_MT_PAD_CONTROL_FLAGS);
        IF_GUID_EQUAL_RETURN(MF_MT_SOURCE_CONTENT_HINT);
        IF_GUID_EQUAL_RETURN(MF_MT_VIDEO_CHROMA_SITING);
        IF_GUID_EQUAL_RETURN(MF_MT_INTERLACE_MODE);
        IF_GUID_EQUAL_RETURN(MF_MT_TRANSFER_FUNCTION);
        IF_GUID_EQUAL_RETURN(MF_MT_VIDEO_PRIMARIES);
        IF_GUID_EQUAL_RETURN(MF_MT_CUSTOM_VIDEO_PRIMARIES);
        IF_GUID_EQUAL_RETURN(MF_MT_YUV_MATRIX);
        IF_GUID_EQUAL_RETURN(MF_MT_VIDEO_LIGHTING);
        IF_GUID_EQUAL_RETURN(MF_MT_VIDEO_NOMINAL_RANGE);
        IF_GUID_EQUAL_RETURN(MF_MT_GEOMETRIC_APERTURE);
        IF_GUID_EQUAL_RETURN(MF_MT_MINIMUM_DISPLAY_APERTURE);
        IF_GUID_EQUAL_RETURN(MF_MT_PAN_SCAN_APERTURE);
        IF_GUID_EQUAL_RETURN(MF_MT_PAN_SCAN_ENABLED);
        IF_GUID_EQUAL_RETURN(MF_MT_AVG_BITRATE);
        IF_GUID_EQUAL_RETURN(MF_MT_AVG_BIT_ERROR_RATE);
        IF_GUID_EQUAL_RETURN(MF_MT_MAX_KEYFRAME_SPACING);
        IF_GUID_EQUAL_RETURN(MF_MT_DEFAULT_STRIDE);
        IF_GUID_EQUAL_RETURN(MF_MT_PALETTE);
        IF_GUID_EQUAL_RETURN(MF_MT_USER_DATA);
        IF_GUID_EQUAL_RETURN(MF_MT_AM_FORMAT_TYPE);
        IF_GUID_EQUAL_RETURN(MF_MT_MPEG_START_TIME_CODE);
        IF_GUID_EQUAL_RETURN(MF_MT_MPEG2_PROFILE);
        IF_GUID_EQUAL_RETURN(MF_MT_MPEG2_LEVEL);
        IF_GUID_EQUAL_RETURN(MF_MT_MPEG2_FLAGS);
        IF_GUID_EQUAL_RETURN(MF_MT_MPEG_SEQUENCE_HEADER);
        IF_GUID_EQUAL_RETURN(MF_MT_DV_AAUX_SRC_PACK_0);
        IF_GUID_EQUAL_RETURN(MF_MT_DV_AAUX_CTRL_PACK_0);
        IF_GUID_EQUAL_RETURN(MF_MT_DV_AAUX_SRC_PACK_1);
        IF_GUID_EQUAL_RETURN(MF_MT_DV_AAUX_CTRL_PACK_1);
        IF_GUID_EQUAL_RETURN(MF_MT_DV_VAUX_SRC_PACK);
        IF_GUID_EQUAL_RETURN(MF_MT_DV_VAUX_CTRL_PACK);
        IF_GUID_EQUAL_RETURN(MF_MT_ARBITRARY_HEADER);
        IF_GUID_EQUAL_RETURN(MF_MT_ARBITRARY_FORMAT);
        IF_GUID_EQUAL_RETURN(MF_MT_IMAGE_LOSS_TOLERANT);
        IF_GUID_EQUAL_RETURN(MF_MT_MPEG4_SAMPLE_DESCRIPTION);
        IF_GUID_EQUAL_RETURN(MF_MT_MPEG4_CURRENT_SAMPLE_ENTRY);
        IF_GUID_EQUAL_RETURN(MF_MT_ORIGINAL_4CC);
        IF_GUID_EQUAL_RETURN(MF_MT_ORIGINAL_WAVE_FORMAT_TAG);
        // Media types
        IF_GUID_EQUAL_RETURN(MFMediaType_Audio);
        IF_GUID_EQUAL_RETURN(MFMediaType_Video);
        IF_GUID_EQUAL_RETURN(MFMediaType_Protected);
        //IF_GUID_EQUAL_RETURN(MFMediaType_Perception);
        IF_GUID_EQUAL_RETURN(MFMediaType_Stream);
        IF_GUID_EQUAL_RETURN(MFMediaType_SAMI);
        IF_GUID_EQUAL_RETURN(MFMediaType_Script);
        IF_GUID_EQUAL_RETURN(MFMediaType_Image);
        IF_GUID_EQUAL_RETURN(MFMediaType_HTML);
        IF_GUID_EQUAL_RETURN(MFMediaType_Binary);
        IF_GUID_EQUAL_RETURN(MFMediaType_FileTransfer);
        IF_GUID_EQUAL_RETURN(MFVideoFormat_AI44); //     FCC('AI44')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_ARGB32); //   D3DFMT_A8R8G8B8
        IF_GUID_EQUAL_RETURN(MFVideoFormat_AYUV); //     FCC('AYUV')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_DV25); //     FCC('dv25')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_DV50); //     FCC('dv50')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_DVH1); //     FCC('dvh1')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_DVC);
        IF_GUID_EQUAL_RETURN(MFVideoFormat_DVHD);
        IF_GUID_EQUAL_RETURN(MFVideoFormat_DVSD); //     FCC('dvsd')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_DVSL); //     FCC('dvsl')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_H264); //     FCC('H264')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_I420); //     FCC('I420')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_IYUV); //     FCC('IYUV')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_M4S2); //     FCC('M4S2')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_MJPG);
        IF_GUID_EQUAL_RETURN(MFVideoFormat_MP43); //     FCC('MP43')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_MP4S); //     FCC('MP4S')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_MP4V); //     FCC('MP4V')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_MPG1); //     FCC('MPG1')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_MSS1); //     FCC('MSS1')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_MSS2); //     FCC('MSS2')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_NV11); //     FCC('NV11')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_NV12); //     FCC('NV12')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_P010); //     FCC('P010')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_P016); //     FCC('P016')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_P210); //     FCC('P210')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_P216); //     FCC('P216')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_RGB24); //    D3DFMT_R8G8B8
        IF_GUID_EQUAL_RETURN(MFVideoFormat_RGB32); //    D3DFMT_X8R8G8B8
        IF_GUID_EQUAL_RETURN(MFVideoFormat_RGB555); //   D3DFMT_X1R5G5B5
        IF_GUID_EQUAL_RETURN(MFVideoFormat_RGB565); //   D3DFMT_R5G6B5
        IF_GUID_EQUAL_RETURN(MFVideoFormat_RGB8);
        IF_GUID_EQUAL_RETURN(MFVideoFormat_UYVY); //     FCC('UYVY')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_v210); //     FCC('v210')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_v410); //     FCC('v410')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_WMV1); //     FCC('WMV1')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_WMV2); //     FCC('WMV2')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_WMV3); //     FCC('WMV3')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_WVC1); //     FCC('WVC1')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_Y210); //     FCC('Y210')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_Y216); //     FCC('Y216')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_Y410); //     FCC('Y410')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_Y416); //     FCC('Y416')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_Y41P);
        IF_GUID_EQUAL_RETURN(MFVideoFormat_Y41T);
        IF_GUID_EQUAL_RETURN(MFVideoFormat_YUY2); //     FCC('YUY2')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_YV12); //     FCC('YV12')
        IF_GUID_EQUAL_RETURN(MFVideoFormat_YVYU);
        //IF_GUID_EQUAL_RETURN(MFVideoFormat_H263);
        IF_GUID_EQUAL_RETURN(MFVideoFormat_H265);
        IF_GUID_EQUAL_RETURN(MFVideoFormat_H264_ES);
        IF_GUID_EQUAL_RETURN(MFVideoFormat_HEVC);
        IF_GUID_EQUAL_RETURN(MFVideoFormat_HEVC_ES);
        IF_GUID_EQUAL_RETURN(MFVideoFormat_MPEG2);
        IF_GUID_EQUAL_RETURN(MFVideoFormat_VP80);
        IF_GUID_EQUAL_RETURN(MFVideoFormat_VP90);
        IF_GUID_EQUAL_RETURN(MFVideoFormat_420O);
        IF_GUID_EQUAL_RETURN(MFVideoFormat_Y42T);
        IF_GUID_EQUAL_RETURN(MFVideoFormat_YVU9);
        IF_GUID_EQUAL_RETURN(MFVideoFormat_v216);
        IF_GUID_EQUAL_RETURN(MFVideoFormat_L8);
        IF_GUID_EQUAL_RETURN(MFVideoFormat_L16);
        IF_GUID_EQUAL_RETURN(MFVideoFormat_D16);
        // IF_GUID_EQUAL_RETURN(D3DFMT_X8R8G8B8);
        // IF_GUID_EQUAL_RETURN(D3DFMT_A8R8G8B8);
        // IF_GUID_EQUAL_RETURN(D3DFMT_R8G8B8);
        // IF_GUID_EQUAL_RETURN(D3DFMT_X1R5G5B5);
        // IF_GUID_EQUAL_RETURN(D3DFMT_A4R4G4B4);
        // IF_GUID_EQUAL_RETURN(D3DFMT_R5G6B5);
        // IF_GUID_EQUAL_RETURN(D3DFMT_P8);
        // IF_GUID_EQUAL_RETURN(D3DFMT_A2R10G10B10);
        // IF_GUID_EQUAL_RETURN(D3DFMT_A2B10G10R10);
        // IF_GUID_EQUAL_RETURN(D3DFMT_L8);
        // IF_GUID_EQUAL_RETURN(D3DFMT_L16);
        // IF_GUID_EQUAL_RETURN(D3DFMT_D16);
        IF_GUID_EQUAL_RETURN(MFVideoFormat_A2R10G10B10);
        IF_GUID_EQUAL_RETURN(MFVideoFormat_A16B16G16R16F);
        IF_GUID_EQUAL_RETURN(MFAudioFormat_PCM); //              WAVE_FORMAT_PCM
        IF_GUID_EQUAL_RETURN(MFAudioFormat_Float); //            WAVE_FORMAT_IEEE_FLOAT
        IF_GUID_EQUAL_RETURN(MFAudioFormat_DTS); //              WAVE_FORMAT_DTS
        IF_GUID_EQUAL_RETURN(MFAudioFormat_Dolby_AC3_SPDIF); //  WAVE_FORMAT_DOLBY_AC3_SPDIF
        IF_GUID_EQUAL_RETURN(MFAudioFormat_DRM); //              WAVE_FORMAT_DRM
        IF_GUID_EQUAL_RETURN(MFAudioFormat_WMAudioV8); //        WAVE_FORMAT_WMAUDIO2
        IF_GUID_EQUAL_RETURN(MFAudioFormat_WMAudioV9); //        WAVE_FORMAT_WMAUDIO3
        IF_GUID_EQUAL_RETURN(MFAudioFormat_WMAudio_Lossless); // WAVE_FORMAT_WMAUDIO_LOSSLESS
        IF_GUID_EQUAL_RETURN(MFAudioFormat_WMASPDIF); //         WAVE_FORMAT_WMASPDIF
        IF_GUID_EQUAL_RETURN(MFAudioFormat_MSP1); //             WAVE_FORMAT_WMAVOICE9
        IF_GUID_EQUAL_RETURN(MFAudioFormat_MP3); //              WAVE_FORMAT_MPEGLAYER3
        IF_GUID_EQUAL_RETURN(MFAudioFormat_MPEG); //             WAVE_FORMAT_MPEG
        IF_GUID_EQUAL_RETURN(MFAudioFormat_AAC); //              WAVE_FORMAT_MPEG_HEAAC
        IF_GUID_EQUAL_RETURN(MFAudioFormat_ADTS); //             WAVE_FORMAT_MPEG_ADTS_AAC
        IF_GUID_EQUAL_RETURN(MFAudioFormat_ALAC);
        IF_GUID_EQUAL_RETURN(MFAudioFormat_AMR_NB);
        IF_GUID_EQUAL_RETURN(MFAudioFormat_AMR_WB);
        IF_GUID_EQUAL_RETURN(MFAudioFormat_AMR_WP);
        IF_GUID_EQUAL_RETURN(MFAudioFormat_Dolby_AC3);
        IF_GUID_EQUAL_RETURN(MFAudioFormat_Dolby_DDPlus);
        IF_GUID_EQUAL_RETURN(MFAudioFormat_FLAC);
        IF_GUID_EQUAL_RETURN(MFAudioFormat_Opus);
        // IF_GUID_EQUAL_RETURN(MEDIASUBTYPE_RAW_AAC1);
        // IF_GUID_EQUAL_RETURN(MFAudioFormat_Float_SpatialObjects);
        // IF_GUID_EQUAL_RETURN(MFAudioFormat_QCELP);

        return L"<unknown>";
    }
}