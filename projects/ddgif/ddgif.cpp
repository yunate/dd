#include "ddgif/stdafx.h"
#include "ddgif/ddgif.h"
#include "ddgif/string_table.h"
#include <memory.h>

namespace DogGifNSP
{
#define GIF_INTERLACE_PASSES 4
static int g_GifInterlaceOffset[GIF_INTERLACE_PASSES] = {0, 4, 2, 1};
static int g_GifInterlaceIncrement[GIF_INTERLACE_PASSES] = {8, 8, 4, 2};

DogGif::DogGif() :
    m_curFrame(0xffffffff),
    m_pStringTable(nullptr)
{
}

DogGif::~DogGif()
{
    if (m_pStringTable != nullptr)
    {
        delete m_pStringTable;
        m_pStringTable = NULL;
    }
}

bool DogGif::DogGif::Init(u8 * pBuff, u32 buffLen)
{
    m_hasInit = false;

    if (pBuff == nullptr)
    {
        return false;
    }

    if (!ReadHead(&pBuff, buffLen))
    {
        return false;
    }

    m_pStringTable = new(std::nothrow)StringTable();

    if (m_pStringTable == nullptr)
    {
        return false;
    }

    if (!ReadLsd(&pBuff, buffLen))
    {
        return false;
    }

    if (!ReadGolColorTable(&pBuff, buffLen))
    {
        return false;
    }

    DogGifFrame* pFrame = ReadFrameData(&pBuff, buffLen);

    while (pFrame != nullptr)
    {
        m_gifGolInfo.m_frameData.push_back(pFrame);
        pFrame = ReadFrameData(&pBuff, buffLen);
    }

    if (buffLen != 1 || pBuff[0] != 0x3b)
    {
        return false;
    }

    m_hasInit = true;
    m_curFrame = (u32)m_gifGolInfo.m_frameData.size();
    return true;
}

bool DogGif::GetNextFrame(DogGifColor ** ppBuff, u32 & buffLen)
{
    int tmp = m_curFrame++;
    if (m_curFrame >= m_gifGolInfo.m_frameData.size())
    {
        m_curFrame = 0;
    }

    if (!DecodeFrame())
    {
        m_curFrame = tmp;
        return false;
    }

    buffLen = m_gifGolInfo.m_width * m_gifGolInfo.m_height;
    *ppBuff = &m_frameBuff[0];
    return true;
}

bool DogGif::GetCurrentFrame(DogGifColor ** ppBuff, u32 & buffLen)
{
    if (m_frameBuff.size() == 0 ||
        m_curFrame >= m_gifGolInfo.m_frameData.size())
    {
        return false;
    }

    buffLen = m_gifGolInfo.m_width * m_gifGolInfo.m_height;
    *ppBuff = &m_frameBuff[0];
    return true;
}

u32 DogGif::GetGolWidth()
{
    return m_gifGolInfo.m_width;
}

u32 DogGif::GetGolHeight()
{
    return m_gifGolInfo.m_height;
}

u32 DogGif::GetGolTimeDelay()
{
    return GetTimeDelay(0);
}

u32 DogGif::GetCurTimeDelay()
{
    if (m_gifGolInfo.m_frameData.size() >= m_curFrame) {
        return GetGolTimeDelay();
    }
    return GetTimeDelay(m_curFrame);
}

u32 DogGif::GetTimeDelay(u32 frameIdx)
{
    u32 delayTime = 0;

    if (m_gifGolInfo.m_frameData.size() >= frameIdx)
    {
        delayTime = m_gifGolInfo.m_frameData[frameIdx]->m_delayTime * 10;
    } else {
        delayTime = 66;
    }

    return delayTime;
}

bool DogGif::HasInit()
{
    return m_hasInit;
}

u32 DogGif::GetFrameCount()
{
    return (u32)m_gifGolInfo.m_frameData.size();
}

u32 DogGif::GetCurFrameIndex()
{
    return m_curFrame;
}

bool DogGif::ReadHead(u8** ppBuff, u32& buffLen)
{
    // 不要去判pBuff null 了
    u8* pBuff = *ppBuff;
    GifHead head;
    u8 headSize = sizeof(GifHead);

    if (buffLen < (u32)(headSize))
    {
        return false;
    }

    ::memcpy(&head, pBuff, headSize);
    *ppBuff += headSize;
    buffLen -= headSize;

    if (!head.IsGifFile())
    {
        return false;
    }

    m_gifGolInfo.m_gifHeadSignaturl.append((const char*)&head, headSize);
    return true;
}

bool DogGif::ReadLsd(u8 ** ppBuff, u32 & buffLen)
{
    // 不要去判读pBuff null 了
    u8* pBuff = *ppBuff;
    GifLogicalScreenDescriptor lsd;
    u8 lsdSize = sizeof(GifLogicalScreenDescriptor);

    if (buffLen < (u32)(lsdSize))
    {
        return false;
    }

    ::memcpy(&lsd, pBuff, lsdSize);
    *ppBuff += lsdSize;
    buffLen -= lsdSize;
    m_gifGolInfo.m_width = lsd.m_width;
    m_gifGolInfo.m_height = lsd.m_height;
    m_gifGolInfo.m_hasGolColorTable = (lsd.m_colorDes >> 7) & 0x01;
    m_gifGolInfo.m_colorDepth = (lsd.m_colorDes >> 4) & 0x07;
    m_gifGolInfo.m_isGolColorTableSorted = (lsd.m_colorDes >> 3) & 0x01;

    if (m_gifGolInfo.m_hasGolColorTable)
    {
        m_gifGolInfo.m_golColorTableBit = 2 << ((lsd.m_colorDes & 0x07));
    }
    else
    {
        m_gifGolInfo.m_golColorTableBit = 0;
    }

    m_gifGolInfo.m_bgColorIndex = lsd.m_bgColor;
    m_gifGolInfo.m_pixelToWidthHeight = lsd.m_pixelTo;
    return true;
}

bool DogGif::ReadGolColorTable(u8 ** ppBuff, u32 & buffLen)
{
    if (!m_gifGolInfo.m_hasGolColorTable)
    {
        return true;
    }

    u8 * pBuff = *ppBuff;
    m_gifGolInfo.m_golColorTable.resize(m_gifGolInfo.m_golColorTableBit);

    if (buffLen < (u32)m_gifGolInfo.m_golColorTableBit * 3)
    {
        return false;
    }

    for (size_t i = 0; i < m_gifGolInfo.m_golColorTable.size(); ++i)
    {
        m_gifGolInfo.m_golColorTable[i].m_r = pBuff[i * 3];
        m_gifGolInfo.m_golColorTable[i].m_g = pBuff[i * 3 + 1];
        m_gifGolInfo.m_golColorTable[i].m_b = pBuff[i * 3 + 2];
    }

    *ppBuff += m_gifGolInfo.m_golColorTableBit * 3;
    buffLen -= m_gifGolInfo.m_golColorTableBit * 3;
    return true;
}

DogGifFrame * DogGif::ReadFrameData(u8 ** ppBuff, u32 & buffLen)
{
    DogGifFrame * pFrame = new DogGifFrame();
    u8 * pBuff = *ppBuff;
    bool isOk = false;

    for (u32 i = 0; i < buffLen; ++i)
    {
        if (*ppBuff + buffLen <= pBuff + 2)
        {
            isOk = false;
            break;
        }

        if (pBuff[0] == 0x21)
        {
            if (pBuff[1] == 0xf9)
            {
                ExtendBlock extend;
                u8 extendSize = (u8)sizeof(ExtendBlock);

                if (*ppBuff + buffLen <= pBuff + extendSize)
                {
                    isOk = false;
                    break;
                }

                ::memcpy(&extend, pBuff, extendSize);
                pBuff += extendSize;
                pFrame->m_disposalMethod = (extend.m_userFlag >> 2) & 0x07;
                pFrame->m_userInputFlag = (extend.m_userFlag >> 1) & 0x01;
                pFrame->m_tranFlag = extend.m_userFlag & 0x01;
                pFrame->m_delayTime = extend.m_delayTime;
                pFrame->m_tranColorIndex = extend.m_TranColorIndex;
                continue;
            }
            else if (pBuff[1] == 0xfe)
            {
                // 这种情况下一直找到0为止
                pBuff += 2;

                while (*ppBuff + buffLen > pBuff + 1)
                {
                    if (pBuff[0] == 0)
                    {
                        break;
                    }

                    ++pBuff;
                }
            }
            else if (pBuff[1] == 0x01)
            {
                pBuff += 2;

                if (*ppBuff + buffLen <= pBuff + 13)
                {
                    isOk = false;
                    break;
                }

                // 然后一直找到0为止
                while (*ppBuff + buffLen > pBuff + 1)
                {
                    if (pBuff[0] == 0)
                    {
                        break;
                    }

                    ++pBuff;
                }
            }
            else if (pBuff[1] == 0xff)
            {
                pBuff += 2;

                if (*ppBuff + buffLen <= pBuff + 12)
                {
                    isOk = false;
                    break;
                }

                // 然后一直找到0为止
                while (*ppBuff + buffLen > pBuff + 1)
                {
                    if (pBuff[0] == 0)
                    {
                        break;
                    }

                    ++pBuff;
                }
            }
        }

        if (pBuff[0] == 0x2c)
        {
            ImageDescriptor imgDes;
            u8 imgDesSize = (u8)sizeof(imgDes);

            if (*ppBuff + buffLen <= pBuff + imgDesSize)
            {
                isOk = false;
                break;
            }

            ::memcpy(&imgDes, pBuff, imgDesSize);
            pBuff += imgDesSize;
            pFrame->m_left = imgDes.m_left;
            pFrame->m_top = imgDes.m_top;
            pFrame->m_width = imgDes.m_width;
            pFrame->m_height = imgDes.m_height;
            pFrame->m_hasLocalColorTable = (imgDes.m_localColorFlag >> 7) & 0x01;
            pFrame->m_interlaceFlag = (imgDes.m_localColorFlag >> 6) & 0x01;
            pFrame->m_sortFlag = (imgDes.m_localColorFlag >> 5) & 0x01;
            pFrame->m_LocalColorTableBit = 0;

            if (pFrame->m_hasLocalColorTable)
            {
                pFrame->m_LocalColorTableBit = 2 << ((imgDes.m_localColorFlag & 0x07));

                if (*ppBuff + buffLen <= pBuff + pFrame->m_LocalColorTableBit * 3)
                {
                    isOk = false;
                    break;
                }

                pFrame->m_localColorTable.resize(pFrame->m_LocalColorTableBit);

                for (size_t j = 0; j < pFrame->m_LocalColorTableBit; ++j)
                {
                    pFrame->m_localColorTable[j].m_r = pBuff[j * 3];
                    pFrame->m_localColorTable[j].m_g = pBuff[j * 3 + 1];
                    pFrame->m_localColorTable[j].m_b = pBuff[j * 3 + 2];
                }

                pBuff += pFrame->m_LocalColorTableBit * 3;
            }

            if (*ppBuff + buffLen <= pBuff + 2)
            {
                isOk = false;
                break;
            }

            pFrame->m_codeLen = pBuff[0];
            ++pBuff;
            u8* pTmp = pBuff;
            u32 allSize = 0;
            u8 blockSize = pBuff[0];

            while (blockSize != 0)
            {
                allSize += blockSize + 1;
                pBuff += blockSize + 1;

                if (*ppBuff + buffLen <= pBuff)
                {
                    isOk = false;
                    break;
                }

                blockSize = pBuff[0];
            }

            ++allSize;
            ++pBuff;
            pFrame->m_frameData.resize(allSize);
            ::memcpy(&(pFrame->m_frameData[0]), pTmp, allSize);
            isOk = true;
            break;
        }

        ++pBuff;
    }
    
    if (!isOk)
    {
        delete pFrame;
        pFrame = nullptr;
    }
    else
    {
        buffLen -= (u32)(pBuff - *ppBuff);
        *ppBuff = pBuff;
    }
    
    return pFrame;
}

bool DogGif::DecodeFrame()
{
    u32 buffSize = m_gifGolInfo.m_height * m_gifGolInfo.m_width;

    if (m_curFrame >= m_gifGolInfo.m_frameData.size())
    {
        return false;
    }

    DogGifFrame * pFrame = m_gifGolInfo.m_frameData[m_curFrame];

    if (pFrame->m_width + pFrame->m_left > m_gifGolInfo.m_width ||
        pFrame->m_height + pFrame->m_top > m_gifGolInfo.m_height)
    {
        return false;
    }

    // 颜色填充
    std::vector<DogGifColor> colorTable;
    colorTable.resize(256);

    if (pFrame->m_hasLocalColorTable)
    {
        ::memcpy((void*)&colorTable[0],
            (void*)&pFrame->m_localColorTable[0],
            pFrame->m_localColorTable.size() * sizeof(DogGifColor));
    }
    else if (m_gifGolInfo.m_hasGolColorTable)
    {
        ::memcpy((void*)&colorTable[0],
            (void*)&m_gifGolInfo.m_golColorTable[0],
            m_gifGolInfo.m_golColorTable.size() * sizeof(DogGifColor));
    }
    else
    {
        for (int i = 0; i < 256; i++)
        {
            colorTable[i].m_r = (u8)i;
            colorTable[i].m_g = (u8)i;
            colorTable[i].m_b = (u8)i;
        }
    }

//     if (pFrame->m_tranFlag == 1)
//     {
//         colorTable[pFrame->m_tranColorIndex].m_a = 0;
//         colorTable[pFrame->m_tranColorIndex].m_r = 255;
//         colorTable[pFrame->m_tranColorIndex].m_g = 255;
//         colorTable[pFrame->m_tranColorIndex].m_b = 255;
//     }

    if (m_curFrame == 0)
    {
        m_frameBuff.resize(buffSize);
        DogGifColor bgColor = colorTable[m_gifGolInfo.m_bgColorIndex];
        
        for (u32 i = 0; i < buffSize; ++i)
        {
            m_frameBuff[i].m_r = bgColor.m_r;
            m_frameBuff[i].m_g = bgColor.m_g;
            m_frameBuff[i].m_b = bgColor.m_b;
        }
    }
    else
    {
        if (pFrame->m_disposalMethod == 0 ||
            pFrame->m_disposalMethod == 3)
        {
            //TODO: 等于3的时候可能会还要保存在前面一张，这里我就不加了
            return true;
        }
        else if (pFrame->m_disposalMethod == 1)
        {
            // 
            int i = 0;
            ++i;
        }
        else if (pFrame->m_disposalMethod == 2)
        {
            DogGifColor bgColor = colorTable[m_gifGolInfo.m_bgColorIndex];

            for (u32 i = 0; i < buffSize; ++i)
            {
                m_frameBuff[i].m_r = bgColor.m_r;
                m_frameBuff[i].m_g = bgColor.m_g;
                m_frameBuff[i].m_b = bgColor.m_b;
            }
        }
    }

    u32 x = 0;
    u32 y = 0;
    u8 interlacepass = 0;
    u32 frameDataIndex = 0;
    u8 decodeBuff[4096];
    DogGifColor * scanline = &(m_frameBuff)[m_gifGolInfo.m_width * (y + pFrame->m_top) + pFrame->m_left];

    // LZW 解码
    m_pStringTable->Initialize(pFrame->m_codeLen);

    while (1)
    {
        if (frameDataIndex == pFrame->m_frameData.size())
        {
            break;
        }

        u8 blockSize = pFrame->m_frameData[frameDataIndex++];

        if (blockSize == 0 ||
            frameDataIndex + blockSize > pFrame->m_frameData.size())
        {
            break;
        }

        ::memcpy(m_pStringTable->FillInputBuffer(blockSize), (void*)&pFrame->m_frameData[frameDataIndex], blockSize);
        frameDataIndex += blockSize;
        int decodeSize = sizeof(decodeBuff);

        while (m_pStringTable->Decompress(decodeBuff, &decodeSize))
        {
            for (int i = 0; i < decodeSize; i++) 
            {
                if (pFrame->m_tranFlag != 1 ||
                    decodeBuff[i] != pFrame->m_tranColorIndex)
                {
                    scanline[x++] = colorTable[decodeBuff[i]];
                }
                else
                {
                    ++x;
                }

                if (x >= pFrame->m_width)
                {
                    if (pFrame->m_interlaceFlag) 
                    {
                        y += g_GifInterlaceIncrement[interlacepass];

                        if (y >= pFrame->m_height &&
                            ++interlacepass < GIF_INTERLACE_PASSES)
                        {
                            y = g_GifInterlaceOffset[interlacepass];
                        }
                    }
                    else 
                    {
                        y++;
                    }

                    if (y >= pFrame->m_height)
                    {
                        m_pStringTable->Done();
                        break;
                    }

                    x = 0;
                    scanline = &(m_frameBuff)[m_gifGolInfo.m_width * (y + pFrame->m_top) + pFrame->m_left];
                }
            }

            decodeSize = sizeof(decodeBuff);
        }
    }
    return true;
}

}