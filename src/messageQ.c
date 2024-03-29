#include "pebble.h"
#include "pebble_app_info.h"
#include "stdbool.h"
#include "PleaseWait.h"
#include "messageQ.h"
#include "defines.h"

static int s_top;
static int s_tail;
static int s_count;

static uint8_t s_buffers[MAX_OUT_MESSAGE][MAX_OUT_MESSAGE_SIZE];

typedef struct _msgQEntry
{
	DictionaryIterator m_dict;
	MQ_CALLBACK m_callback;
	void *cb_val;
} msgQEntry;

static msgQEntry s_queue[MAX_OUT_MESSAGE];
//static DictionaryIterator s_queue[MAX_OUT_MESSAGE];

void mq_create()
{
	s_top = 0;
	s_tail = 0;
	s_count = 0;
}


void dictupdatecb(const uint32_t key, const Tuple *new_tuple, const Tuple *old_tuple, void *context)
{

}

void mq_post_cb(DictionaryIterator* source, MQ_CALLBACK cb, void *context)
{

	if (s_count == MAX_OUT_MESSAGE)
	{
		//PleaseWaitDebug("MQ full");

		if (s_count == MAX_OUT_MESSAGE)
		{
			return;  // full
		}

	}
	int pos = s_tail;

	DictionaryResult res = dict_write_begin(&s_queue[pos].m_dict, s_buffers[pos], MAX_OUT_MESSAGE_SIZE);
	if (res != DICT_OK)
	{
		PleaseWaitDebug("MQ can't write");
		return;
	}

	// copy the supplied dictionary in
	uint32_t size = MAX_OUT_MESSAGE_SIZE;

	res = dict_merge(&s_queue[pos].m_dict, &size, source, false, dictupdatecb, 0);
	if (res != DICT_OK)
	{
		return;
	}

	dict_write_end(&s_queue[pos].m_dict);
	s_queue[pos].m_callback = cb;
	s_queue[pos].cb_val = context;

	// message ok, so mark acordingly
	s_count++;
	s_tail++;
	if (s_tail >= MAX_OUT_MESSAGE)
	{
		s_tail = 0; // wrap around
	}

	mq_pop();
}

void mq_post(DictionaryIterator* source)
{
	mq_post_cb(source, 0, 0);
}

static void mq_complete(DictionaryIterator* source, AppMessageResult reason)
{
	if (s_queue[s_top].m_callback != 0)
	{
		s_queue[s_top].m_callback(source, reason, s_queue[s_top].cb_val);
	}
	s_top++;
	s_count--;
	K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "Send complete, Messages to send %d", s_count);
	if (s_top >= MAX_OUT_MESSAGE)
	{
		// wraparound
		s_top = 0;
	}
}

void mq_pop()
{
	if (s_count == 0)
	{
		return;
	}
	while (s_count > 0)
	{
		DictionaryIterator *command;

		K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "Messages to send %d", s_count);
		AppMessageResult appRes = app_message_outbox_begin(&command);
		if (appRes != APP_MSG_OK)
		{
			K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "Can't get %d", appRes);
			return;
		}

		Tuple* tupe = dict_read_first(&s_queue[s_top].m_dict);
		while (tupe)
		{
			switch (tupe->type)
			{
			case TUPLE_BYTE_ARRAY:
				dict_write_data(command, tupe->key, tupe->value->data, tupe->length);
				break;
			case TUPLE_CSTRING:
				dict_write_cstring(command, tupe->key, tupe->value->cstring);
				break;
			case TUPLE_UINT:
				switch (tupe->length)
				{
				case 1:
					dict_write_int(command, tupe->key, &tupe->value->uint8, 1, false);
					break;
				case 2:
					dict_write_int(command, tupe->key, &tupe->value->uint16, 2, false);
					break;
				case 4:
					dict_write_int(command, tupe->key, &tupe->value->uint32, 4, false);
					break;
				default:
					// skip
					break;
				}

				break;
				case TUPLE_INT:
					PleaseWaitDebug("Got int");
				
					switch (tupe->length)
					{
					case 1:
						dict_write_int(command, tupe->key, &tupe->value->int8, 1, true);
						break;
					case 2:
						dict_write_int(command, tupe->key, &tupe->value->int16, 2, true);
						break;
					case 4:
						dict_write_int(command, tupe->key, &tupe->value->int32, 4, true);
						break;
					default:
						// skip
						break;
					}
					break;
			}
			tupe = dict_read_next(&s_queue[s_top].m_dict);
		}

		dict_write_int(command, 50, &s_count, 4, true);
		AppMessageResult sres = app_message_outbox_send();
		K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "Send result %d", sres);

	}
	K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "Messages to send now %d", s_count);

}

void my_out_sent_handler(DictionaryIterator *sent, void *context) {
	mq_complete(sent, APP_MSG_OK);
	K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "Send completed ok");
	mq_pop();
}

void my_out_fail_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {

	K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "MQ sent failed %d", reason);
	mq_complete(failed, reason);
	mq_pop();
}

void my_in_drp_handler(AppMessageResult reason, void *context) {
	K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "in failed %d", reason);
	//LOGGER_SI("in failed ", reason);
	//LOGGER_S(_appResult(reason));
	mq_pop();
}
