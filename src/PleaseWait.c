/*
 * PleaseWait.c
 *
 *  Created on: 6 Jul 2013
 *      Author: andrew
 */

#include "pebble.h"
#include "PleaseWait.h"
#include "stdbool.h"
#include "defines.h"
#include "ScreenCapture.h"

static Window* s_window;
static TextLayer* s_title;
static TextLayer* s_body;
static TextLayer* s_debug;
static TextLayer* s_version;

static void handle_unloaded(Window *window);
static void handle_load(Window *window);
static void handle_appear(Window *window);

#ifdef SCREEN_CAPTURE_ENABLED
static void mClickHandler(ClickRecognizerRef recognizer, void *context)
{
	pbl_capture_send();
}
#endif //SCREEN_CAPTURE_ENABLED

static void mClickConfigProvider(void *context)
{
#ifdef SCREEN_CAPTURE_ENABLED
	array_of_ptrs_to_click_configs_to_setup[BUTTON_ID_SELECT]->long_click.handler = mClickHandler;
#endif //SCREEN_CAPTURE_ENABLED
}


static bool s_pleaseWaitOnTop = false;
void PleaseWaitCreate()
{
	s_window = window_create();

	s_title   = text_layer_create(GRect(10,30,134, 60));
	s_body    = text_layer_create(GRect(10,60,134, 60));
	s_debug   = text_layer_create(GRect(10,100,134, 80));
	s_version = text_layer_create(GRect(10,135,134, 19));

 	text_layer_set_text	(s_title, "Please Wait");
    text_layer_set_font(s_title, fonts_get_system_font(FONT_KEY_GOTHIC_28));
    text_layer_set_overflow_mode(s_title, GTextOverflowModeWordWrap );

    text_layer_set_font(s_body, fonts_get_system_font(FONT_KEY_GOTHIC_28));
    text_layer_set_overflow_mode(s_body, GTextOverflowModeWordWrap );

    text_layer_set_font(s_debug, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    text_layer_set_overflow_mode(s_debug, GTextOverflowModeWordWrap );

    text_layer_set_font(s_version, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    text_layer_set_overflow_mode(s_version, GTextOverflowModeWordWrap );

	layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_title));
	layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_body));
	layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_debug));
	layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_version));

 	text_layer_set_text	(s_version, VERSION_STRING);
    s_pleaseWaitOnTop = false;


    window_set_click_config_provider(s_window,mClickConfigProvider);

	K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "PleaseWait created");
}

void PleaseWaitDestroy()
{
	K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "PleaseWaitDestroy");
	text_layer_destroy(s_title);
	text_layer_destroy(s_debug);
	text_layer_destroy(s_version);
    text_layer_destroy(s_body);
	window_destroy(s_window);

}

bool PleaseWaitActive()
{
	return s_pleaseWaitOnTop;
}

void PleaseWaitShow(const char const * body)
{
	K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "PleaseWait >show");

 	text_layer_set_text	(s_body, body);
// 	window_stack_remove(s_window, false);
 	window_stack_push(s_window, true);
}

void PleaseWaitDebug(const char const *debug)
{
 	text_layer_set_text	(s_debug, debug);
}

void PleaseWaitHide()
{
	K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "PleaseWait hide");
	s_pleaseWaitOnTop = false;
 	window_stack_pop(true);
}
