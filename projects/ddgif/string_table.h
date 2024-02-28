#ifndef ddgif_string_table_h_
#define ddgif_string_table_h_

#include "ddgif/ddgifdef.h"
#include <string>

#define MAX_LZW_CODE 4096

namespace DogGifNSP
{

class StringTable
{
public:
    StringTable();
    ~StringTable();
    void Initialize(int minCodeSize);
    u8 *FillInputBuffer(int len);
    void CompressStart(int bpp, int width);
    int CompressEnd(u8 *buf); //0-4 bytes
    bool Compress(u8 *buf, int *len);
    bool Decompress(u8 *buf, int *len);
    void Done(void);

protected:
    bool m_done;

    int m_minCodeSize, m_clearCode, m_endCode, m_nextCode;

    int m_bpp, m_slack; //Compressor information

    int m_prefix; //Compressor state variable
    int m_codeSize, m_codeMask; //Compressor/Decompressor state variables
    int m_oldCode; //Decompressor state variable
    int m_partial, m_partialSize; //Compressor/Decompressor bit buffer

    int firstPixelPassed; // A specific flag that indicates if the first pixel
                            // of the whole image had already been read

    std::string m_strings[MAX_LZW_CODE]; //This is what is really the "string table" data for the Decompressor
    int* m_strmap;

    //input buffer
    u8 *m_buffer;
    int m_bufferSize, m_bufferRealSize, m_bufferPos, m_bufferShift;

    void ClearCompressorTable(void);
    void ClearDecompressorTable(void);
};

}
#endif // ddgif_string_table_h_
