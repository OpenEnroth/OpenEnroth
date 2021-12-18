cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(nuclear)

set(NUKLEAR_SOURCES nuklear_9slice.c
                    nuklear_buffer.c
                    nuklear_button.c
                    nuklear_chart.c
                    nuklear_color.c
                    nuklear_color_picker.c
                    nuklear_combo.c
                    nuklear_context.c
                    nuklear_contextual.c
                    nuklear_draw.c
                    nuklear_edit.c
                    nuklear_font.c
                    nuklear_group.c
                    nuklear_image.c
                    nuklear_input.c
                    nuklear_layout.c
                    nuklear_list_view.c
                    nuklear_math.c
                    nuklear_menu.c
                    nuklear_page_element.c
                    nuklear_panel.c
                    nuklear_pool.c
                    nuklear_popup.c
                    nuklear_progress.c
                    nuklear_property.c
                    nuklear_scrollbar.c
                    nuklear_selectable.c
                    nuklear_slider.c
                    nuklear_string.c
                    nuklear_style.c
                    nuklear_table.c
                    nuklear_text.c
                    nuklear_text_editor.c
                    nuklear_toggle.c
                    nuklear_tooltip.c
                    nuklear_tree.c
                    nuklear_utf8.c
                    nuklear_util.c
                    nuklear_vertex.c
                    nuklear_widget.c
                    nuklear_window.c)

set(NUKLEAR_HEADERS nuklear_config.h
                 stb_rect_pack.h
                 stb_truetype.h)

add_library(nuklear STATIC ${NUKLEAR_SOURCES} ${NUKLEAR_HEADERS})
target_check_style(nuklear)
