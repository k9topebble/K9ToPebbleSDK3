
#include "pebble.h"
#include "Inbox.h"
#include "Action.h"
#include "Read.h"
#include "AreYouSure.h"
#include "MessageData.h"
#include "PleaseWait.h"
#include "messageQ.h"
#include "defines.h"

static AppTimer* s_timer;

#if 1
//Window* window;

//TextLayer* textLayer;


void up_single_click_handler(ClickRecognizerRef recognizer, void *window) {
	(void)recognizer;
	(void)window;

}


void down_single_click_handler(ClickRecognizerRef recognizer, void *window) {
	(void)recognizer;
	(void)window;

}


void select_single_click_handler(ClickRecognizerRef recognizer, void *window) {
	(void)recognizer;
	(void)window;

//	text_layer_set_text(&textLayer, "Start!");
	InboxShow(0);
}


void select_long_click_handler(ClickRecognizerRef recognizer, void *window) {
	(void)recognizer;
	(void)window;

}


// This usually won't need to be modified

void click_config_provider(Window *window) {
	(void)window;

	  window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
	  window_long_click_subscribe(BUTTON_ID_SELECT, 700, select_long_click_handler, 0);
	  window_single_repeating_click_subscribe(BUTTON_ID_UP, 100, up_single_click_handler);
	  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 100, down_single_click_handler);

}
#endif

static void m_PebbleAppTimerHandler(void * data)
{
	mq_pop();
	//app_log(1, __FILE__, __LINE__,"Timer fired");
	s_timer = app_timer_register(200,m_PebbleAppTimerHandler,0);

	if (s_timer == 0)
	{
		K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "Failed to reschedule timer");
	}
}


// Standard app initialisation

void handle_init()
{
	mq_create();
	//ActionCreate();
	AreYouSureCreate();
	ReadCreate();
	InboxCreate();
	PleaseWaitCreate();
	md_activate();
	InboxShow(0);
	s_timer = app_timer_register(200,m_PebbleAppTimerHandler,0);
	if (s_timer == 0)
	{
		K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "Invalid timer handle");
	}
}

void handle_deinit() {
	md_deactivate();
}

int main(void)
{
	app_message_register_inbox_received(my_in_rcv_handler);
	app_message_register_inbox_dropped(my_in_drp_handler);
	app_message_register_outbox_sent(my_out_sent_handler);
	app_message_register_outbox_failed(my_out_fail_handler);

	app_message_open(256,APP_MESSAGE_OUTBOX_SIZE_MINIMUM);


	handle_init();
	PleaseWaitShow("Inbox loading");
	app_event_loop();
	K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "deinit all");

	PleaseWaitDestroy();
	AreYouSureDestroy();
	InboxDestroy();
	handle_deinit();

	window_stack_pop_all(false);

	app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
	return 0;
}

void null_log(uint8_t log_level, const char* src_filename, int src_line_number, const char* fmt, ...)
{

}


