#pragma once

/*  322 */
enum FRAME_TABLE_FLAGS {
    FRAME_TABLE_MORE_FRAMES = 0x1,
    FRAME_TABLE_FIRST = 0x4,
};

/*  359 */
#pragma pack(push, 1)
struct FrameTableTxtLine {  // 7C
    int uPropCount;
    const char *pProperties[30];
};
#pragma pack(pop)

FrameTableTxtLine *txt_file_frametable_parser(const char *str_to_parse,
                                              FrameTableTxtLine *tokens_table);

FrameTableTxtLine *frame_table_txt_parser(const char *pString,
                                          FrameTableTxtLine *a2);
