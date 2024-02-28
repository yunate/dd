#include "ddgif/stdafx.h"
#include "ddgif/string_table.h"
namespace DogGifNSP
{

    StringTable::StringTable()
    {
        m_buffer = NULL;
        firstPixelPassed = 0; // Still no pixel read
                              // Maximum number of entries in the map is MAX_LZW_CODE * 256 
                              // (aka 2**12 * 2**8 => a 20 bits key)
                              // This Map could be optmized to only handle MAX_LZW_CODE * 2**(m_bpp)
        m_strmap = new(std::nothrow) int[1 << 20];
    }

    StringTable::~StringTable()
    {
        if (m_buffer != NULL) {
            delete[] m_buffer;
            m_buffer = NULL;
        }
        if (m_strmap != NULL) {
            delete[] m_strmap;
            m_strmap = NULL;
        }
    }

    void StringTable::Initialize(int minCodeSize)
    {
        m_done = false;

        m_bpp = 8;
        m_minCodeSize = minCodeSize;
        m_clearCode = 1 << m_minCodeSize;
        if (m_clearCode > MAX_LZW_CODE) {
            m_clearCode = MAX_LZW_CODE;
        }
        m_endCode = m_clearCode + 1;

        m_partial = 0;
        m_partialSize = 0;

        m_bufferSize = 0;
        ClearCompressorTable();
        ClearDecompressorTable();
    }

    u8 *StringTable::FillInputBuffer(int len)
    {
        if (m_buffer == NULL) {
            m_buffer = new(std::nothrow) u8[len];
            m_bufferRealSize = len;
        }
        else if (len > m_bufferRealSize) {
            delete[] m_buffer;
            m_buffer = new(std::nothrow) u8[len];
            m_bufferRealSize = len;
        }
        m_bufferSize = len;
        m_bufferPos = 0;
        m_bufferShift = 8 - m_bpp;
        return m_buffer;
    }

    void StringTable::CompressStart(int bpp, int width)
    {
        m_bpp = bpp;
        m_slack = (8 - ((width * bpp) % 8)) % 8;

        m_partial |= m_clearCode << m_partialSize;
        m_partialSize += m_codeSize;
        ClearCompressorTable();
    }

    int StringTable::CompressEnd(u8 *buf)
    {
        int len = 0;

        //output code for remaining prefix
        m_partial |= m_prefix << m_partialSize;
        m_partialSize += m_codeSize;
        while (m_partialSize >= 8) {
            *buf++ = (u8)m_partial;
            m_partial >>= 8;
            m_partialSize -= 8;
            len++;
        }

        //add the end of information code and flush the entire buffer out
        m_partial |= m_endCode << m_partialSize;
        m_partialSize += m_codeSize;
        while (m_partialSize > 0) {
            *buf++ = (u8)m_partial;
            m_partial >>= 8;
            m_partialSize -= 8;
            len++;
        }

        //most this can be is 4 u8s.  7 bits in m_partial to start + 12 for the
        //last code + 12 for the end code = 31 bits total.
        return len;
    }

    bool StringTable::Compress(u8 *buf, int *len)
    {
        if (m_bufferSize == 0 || m_done) {
            return false;
        }

        int mask = (1 << m_bpp) - 1;
        u8 *bufpos = buf;
        while (m_bufferPos < m_bufferSize) {
            //get the current pixel value
            char ch = (char)((m_buffer[m_bufferPos] >> m_bufferShift) & mask);

            // The next prefix is : 
            // <the previous LZW code (on 12 bits << 8)> | <the code of the current pixel (on 8 bits)>
            int nextprefix = (((m_prefix) << 8) & 0xFFF00) + (ch & 0x000FF);
            if (firstPixelPassed) {

                if (m_strmap[nextprefix] > 0) {
                    m_prefix = m_strmap[nextprefix];
                }
                else {
                    m_partial |= m_prefix << m_partialSize;
                    m_partialSize += m_codeSize;
                    //grab full u8s for the output buffer
                    while (m_partialSize >= 8 && bufpos - buf < *len) {
                        *bufpos++ = (u8)m_partial;
                        m_partial >>= 8;
                        m_partialSize -= 8;
                    }

                    //add the code to the "table map"
                    m_strmap[nextprefix] = m_nextCode;

                    //increment the next highest valid code, increase the code size
                    if (m_nextCode == (1 << m_codeSize)) {
                        m_codeSize++;
                    }
                    m_nextCode++;

                    //if we're out of codes, restart the string table
                    if (m_nextCode == MAX_LZW_CODE) {
                        m_partial |= m_clearCode << m_partialSize;
                        m_partialSize += m_codeSize;
                        ClearCompressorTable();
                    }

                    // Only keep the 8 lowest bits (prevent problems with "negative chars")
                    m_prefix = ch & 0x000FF;
                }

                //increment to the next pixel
                if (m_bufferShift > 0 && !(m_bufferPos + 1 == m_bufferSize && m_bufferShift <= m_slack)) {
                    m_bufferShift -= m_bpp;
                }
                else {
                    m_bufferPos++;
                    m_bufferShift = 8 - m_bpp;
                }

                //jump out here if the output buffer is full
                if (bufpos - buf == *len) {
                    return true;
                }

            }
            else {
                // Specific behavior for the first pixel of the whole image

                firstPixelPassed = 1;
                // Only keep the 8 lowest bits (prevent problems with "negative chars")
                m_prefix = ch & 0x000FF;

                //increment to the next pixel
                if (m_bufferShift > 0 && !(m_bufferPos + 1 == m_bufferSize && m_bufferShift <= m_slack)) {
                    m_bufferShift -= m_bpp;
                }
                else {
                    m_bufferPos++;
                    m_bufferShift = 8 - m_bpp;
                }

                //jump out here if the output buffer is full
                if (bufpos - buf == *len) {
                    return true;
                }
            }
        }

        m_bufferSize = 0;
        *len = (int)(bufpos - buf);

        return true;
    }

    bool StringTable::Decompress(u8 *buf, int *len)
    {
        if (m_bufferSize == 0 || m_done) {
            return false;
        }

        u8 *bufpos = buf;
        for (; m_bufferPos < m_bufferSize; m_bufferPos++) {
            m_partial |= (int)m_buffer[m_bufferPos] << m_partialSize;
            m_partialSize += 8;
            while (m_partialSize >= m_codeSize) {
                int code = m_partial & m_codeMask;
                m_partial >>= m_codeSize;
                m_partialSize -= m_codeSize;

                if (code > m_nextCode || /*(m_nextCode == MAX_LZW_CODE && code != m_clearCode) || */code == m_endCode) {
                    m_done = true;
                    *len = (int)(bufpos - buf);
                    return true;
                }
                if (code == m_clearCode) {
                    ClearDecompressorTable();
                    continue;
                }

                //add new string to string table, if not the first pass since a clear code
                if (m_oldCode != MAX_LZW_CODE && m_nextCode < MAX_LZW_CODE) {
                    m_strings[m_nextCode] = m_strings[m_oldCode] + m_strings[code == m_nextCode ? m_oldCode : code][0];
                }

                if ((int)m_strings[code].size() > *len - (bufpos - buf)) {
                    //out of space, stuff the code back in for next time
                    m_partial <<= m_codeSize;
                    m_partialSize += m_codeSize;
                    m_partial |= code;
                    m_bufferPos++;
                    *len = (int)(bufpos - buf);
                    return true;
                }

                //output the string into the buffer
                memcpy(bufpos, m_strings[code].data(), m_strings[code].size());
                bufpos += m_strings[code].size();

                //increment the next highest valid code, add a bit to the mask if we need to increase the code size
                if (m_oldCode != MAX_LZW_CODE && m_nextCode < MAX_LZW_CODE) {
                    if (++m_nextCode < MAX_LZW_CODE) {
                        if ((m_nextCode & m_codeMask) == 0) {
                            m_codeSize++;
                            m_codeMask |= m_nextCode;
                        }
                    }
                }

                m_oldCode = code;
            }
        }

        m_bufferSize = 0;
        *len = (int)(bufpos - buf);

        return true;
    }

    void StringTable::Done(void)
    {
        m_done = true;
    }

    void StringTable::ClearCompressorTable(void)
    {
        if (m_strmap) {
            memset(m_strmap, 0xFF, sizeof(unsigned int)*(1 << 20));
        }
        m_nextCode = m_endCode + 1;

        m_prefix = 0;
        m_codeSize = m_minCodeSize + 1;
    }

    void StringTable::ClearDecompressorTable(void)
    {
        for (int i = 0; i < m_clearCode; i++) {
            m_strings[i].resize(1);
            m_strings[i][0] = (char)i;
        }
        m_nextCode = m_endCode + 1;

        m_codeSize = m_minCodeSize + 1;
        m_codeMask = (1 << m_codeSize) - 1;
        m_oldCode = MAX_LZW_CODE;
    }

}
