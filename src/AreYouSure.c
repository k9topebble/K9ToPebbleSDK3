/*
 * AreYouSure.c
 *
 *  Created on: 25 Jul 2013
 *      Author: andrew
 */


/*
 * show 'Delete <subject>?
 * Select = yes, back = no.
 *
 */

#include "pebble.h"
#include "AreYouSure.h"
#include "stdbool.h"
#include "defines.h"
#include "ScreenCapture.h"

static Window* s_window;
static TextLayer* s_title;
static TextLayer* s_body;
static TextLayer* s_instruction;
static AYS_CALLBACK s_cb;

static void handle_unloaded(Window *window);
static bool s_AreYouSureConfirm = false;
static void mClickHandler(ClickRecognizerRef recognizer, void *context)
{
	s_AreYouSureConfirm = true;

	s_cb(0);
	AreYouSureHide();
}

static void mClickConfigProvider(void *context)
{
	 // single click / repeat-on-hold config:
	  window_single_click_subscribe(BUTTON_ID_SELECT, mClickHandler);
	  window_long_click_subscribe(BUTTON_ID_SELECT, 700, mClickHandler, 0);
}

void AreYouSureCreate()
{
	s_window = window_create();

	s_title = text_layer_create(GRect(10,10,134, 30));
	s_body  = text_layer_create(GRect(10,40,134, 60));
	s_instruction = text_layer_create(GRect(10,100,134, 60));

 	text_layer_set_text	(s_title, "Delete email?");
    text_layer_set_font(s_title, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_overflow_mode(s_title, GTextOverflowModeWordWrap );

    text_layer_set_font(s_body, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_overflow_mode(s_body, GTextOverflowModeWordWrap );

    text_layer_set_text	(s_instruction, "Select to confirm, back to cancel");
    text_layer_set_font(s_instruction, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_overflow_mode(s_instruction, GTextOverflowModeWordWrap );

	layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_title));
	layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_body));
	layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_instruction));

	window_set_click_config_provider(s_window,mClickConfigProvider);
}

void AreYouSureDestroy(void)
{
	K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "AreYouSure  destroy>");
    text_layer_destroy(s_title);
	text_layer_destroy(s_instruction);
    text_layer_destroy(s_body);
	window_destroy(s_window);
}


bool AreYouSureConfirm()
{
	return s_AreYouSureConfirm;
}

void AreYouSureShow(const char const *body, AYS_CALLBACK cb)
{
	s_AreYouSureConfirm = false;
	text_layer_set_text	(s_body, body);
 	window_stack_remove(s_window, false);
 	window_stack_push(s_window, true);
 	s_cb = cb;
}

void AreYouSureHide()
{
 	window_stack_pop(true);
}
