#include "Engine/Tables/FrameTableInc.h"

#include <string.h>

// identical function to work in differnt threads

//----- (004BE3BF) --------------------------------------------------------
FrameTableTxtLine *txt_file_frametable_parser(const char *str_to_parse,
                                              FrameTableTxtLine *tokens_table) {
    bool new_token_flag;  // edx@3
    bool in_quotes;       // [sp+Ch] [bp-4h]@3
    char *parse_pos;
    unsigned char test_char;
    int i;

    static char tokens_buff1[1000];
    static struct FrameTableTxtLine temp_tokens_table1;

    temp_tokens_table1.uPropCount = 0;

    if (str_to_parse && *str_to_parse) {
        parse_pos = (char *)str_to_parse;
        new_token_flag = true;
        in_quotes = false;
        for (i = 0; (i < 1000) && (temp_tokens_table1.uPropCount < 30); ++i) {
            test_char = *parse_pos;
            tokens_buff1[i] = test_char;
            if (!test_char) break;
            if ((test_char != ' ') && (test_char != ',') &&
                    (test_char != '\t') ||
                in_quotes) {
                if (test_char == '"') {
                    tokens_buff1[i] = '\0';
                    new_token_flag = true;
                    if (in_quotes) {
                        in_quotes = false;
                    } else {
                        in_quotes = true;
                        if (*(parse_pos + 1) == '"') {
                            temp_tokens_table1
                                .pProperties[temp_tokens_table1.uPropCount] =
                                &tokens_buff1[i];
                            ++temp_tokens_table1.uPropCount;
                        }
                    }
                } else {
                    if (new_token_flag) {
                        temp_tokens_table1
                            .pProperties[temp_tokens_table1.uPropCount] =
                            &tokens_buff1[i];
                        ++temp_tokens_table1.uPropCount;
                    }
                    new_token_flag = false;
                }
            } else {
                tokens_buff1[i] = '\0';
                new_token_flag = true;
            }
            ++parse_pos;
        }
        tokens_buff1[i] = '\0';
    }
    memcpy(tokens_table, &temp_tokens_table1, sizeof(FrameTableTxtLine));
    return tokens_table;
}
// F8BA58: using guessed type FrameTableTxtLine static_stru_F8BA58;

//----- (004BE485) --------------------------------------------------------
FrameTableTxtLine *frame_table_txt_parser(const char *str_to_parse,
                                          FrameTableTxtLine *tokens_table) {
    bool new_token_flag;  // edx@3
    bool in_quotes;       // [sp+Ch] [bp-4h]@3
    char *parse_pos;
    unsigned char test_char;
    int i;

    static char tokens_buff2[1000];
    static struct FrameTableTxtLine temp_tokens_table2;
    temp_tokens_table2.uPropCount = 0;

    if (str_to_parse && *str_to_parse) {
        parse_pos = (char *)str_to_parse;
        new_token_flag = true;
        in_quotes = false;
        for (i = 0; (i < 1000) && (temp_tokens_table2.uPropCount < 30); ++i) {
            test_char = *parse_pos;
            tokens_buff2[i] = test_char;
            if (!test_char) break;
            if ((test_char != ' ') && (test_char != ',') &&
                    (test_char != '\t') ||
                in_quotes) {
                if (test_char == '"') {
                    tokens_buff2[i] = '\0';
                    new_token_flag = true;
                    if (in_quotes) {
                        in_quotes = false;
                    } else {
                        in_quotes = true;
                        if (*(parse_pos + 1) == '"') {
                            temp_tokens_table2
                                .pProperties[temp_tokens_table2.uPropCount] =
                                &tokens_buff2[i];
                            ++temp_tokens_table2.uPropCount;
                        }
                    }
                } else {
                    if (new_token_flag) {
                        temp_tokens_table2
                            .pProperties[temp_tokens_table2.uPropCount] =
                            &tokens_buff2[i];
                        ++temp_tokens_table2.uPropCount;
                    }
                    new_token_flag = false;
                }
            } else {
                tokens_buff2[i] = '\0';
                new_token_flag = true;
            }
            ++parse_pos;
        }

        tokens_buff2[i] = '\0';
    }
    memcpy(tokens_table, &temp_tokens_table2, sizeof(FrameTableTxtLine));
    return tokens_table;
}
