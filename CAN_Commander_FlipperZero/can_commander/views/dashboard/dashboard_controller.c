#include "dashboard_i.h"

#include <stdio.h>

bool dashboard_controller_draw(Canvas* canvas, const AppDashboardModel* dashboard) {
    if(!dashboard || (dashboard->mode != AppDashboardGameController)) {
        return false;
    }

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "Game Controller");

    if (!controller_is_enabled()) {
        canvas_draw_str_aligned(canvas, 64, 24, AlignCenter, AlignCenter, "USB mode changed.");
        canvas_draw_str_aligned(canvas, 64, 34, AlignCenter, AlignCenter, "Connect to host");
        canvas_draw_str_aligned(canvas, 64, 45, AlignCenter, AlignCenter, "then press OK to");
        canvas_draw_str_aligned(canvas, 64, 63, AlignCenter, AlignBottom, "start.");
        return true;
    }

    char fps_text[24] = {0};
    snprintf(fps_text, sizeof(fps_text), "Rate: %lu fps", (unsigned long)dashboard->read_rate_fps);

    // TODO: Should have a more useful UI, maybe visualize the controller and the current inputs?
    canvas_draw_str_aligned(canvas, 64, 24, AlignCenter, AlignCenter, "Running.");
    canvas_draw_str_aligned(canvas, 64, 34, AlignCenter, AlignCenter, fps_text);
    return true;
}

bool dashboard_controller_input(App* app, const InputEvent* event) {
    if(!app || !event) {
        return false;
    }

    if (event->type == InputTypeShort && event->key == InputKeyOk) {
        controller_enable();
        return true;
    }

    return false;
}

void dashboard_controller_update(App* app, const CcEvent* event) {
    if(!event || event->type != CcEventTypeCanFrame) {
        return;
    }

    controller_handle(event);

    with_view_model(
        app->dashboard_view,
        AppDashboardModel * model,
        {
            model->mode = AppDashboardGameController;
            strncpy(model->title, "Game Controller", sizeof(model->title) - 1U);
            model->title[sizeof(model->title) - 1U] = '\0';

            if(model->mode == AppDashboardGameController) {
                const uint32_t ts = event->data.can_frame.ts_ms;
                if(model->read_rate_window_start_ms == 0U) {
                    model->read_rate_window_start_ms = ts;
                    model->read_rate_window_count = 0U;
                    model->read_rate_fps = 0U;
                }

                const uint32_t elapsed = ts - model->read_rate_window_start_ms;
                if(elapsed >= 1000U) {
                    const uint32_t sample_ms = (elapsed == 0U) ? 1U : elapsed;
                    model->read_rate_fps =
                        ((uint32_t)model->read_rate_window_count * 1000U) / sample_ms;
                    model->read_rate_window_start_ms = ts;
                    model->read_rate_window_count = 0U;
                }

                if(model->read_rate_window_count < 0xFFFFU) {
                    model->read_rate_window_count++;
                }
            } else {
                model->read_rate_window_start_ms = 0U;
                model->read_rate_window_count = 0U;
                model->read_rate_fps = 0U;
            }
        },
        false);
}
