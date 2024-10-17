#pragma once

struct FrameTableTxtLine {  // 7C
    int uPropCount;
    const char *pProperties[30];
};

FrameTableTxtLine *txt_file_frametable_parser(const char *str_to_parse,
                                              FrameTableTxtLine *tokens_table);

FrameTableTxtLine *frame_table_txt_parser(const char *pString,
                                          FrameTableTxtLine *a2);
