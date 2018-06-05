/*
* Copyright c                  Realtek Semiconductor Corporation, 2008  
* All rights reserved.
* Abstract : 
* Author : hyking (hyking_liu@realsil.com.cn)  
*/
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
#include "rtl_errno.h"
//#include "rtl_utils.h"
#include "rtl865x_eventMgr.h"
#include "rtl865x_vlan.h"
#include <linux/seq_file.h>

/*record the allocated memory pool pointer, 
it can be used when event management module destroyed*/
static char eventMgrInitFlag=FALSE;
static rtl865x_event_t *eventPool;
static rtl865x_eventLayerList_t *eventLayerListPool;
static struct rtl865x_eventTables_s eventsTables;
static rtl865x_eventMgr_param_t eventMgrParam;

int32 rtl865x_initEventMgr(rtl865x_eventMgr_param_t *param)
{
	int32 i;
	memset(&eventMgrParam,0,sizeof(rtl865x_eventMgr_param_t));
	
	if(param==NULL)
	{
		eventMgrParam.eventCnt=DEFAULT_EVENT_CNT;
		eventMgrParam.eventListCnt=DEFAULT_EVENT_LIST_CNT;
	}
	else
	{
		if(param->eventCnt>MAX_EVENT_CNT)
		{
			eventMgrParam.eventCnt=MAX_EVENT_CNT;
		}
		else if(param->eventCnt==0)
		{
			eventMgrParam.eventCnt=DEFAULT_EVENT_CNT;
		}
		else
		{
			eventMgrParam.eventCnt=param->eventCnt;
		}
		
		
		if(param->eventListCnt>MAX_EVENT_LIST_CNT)
		{
			eventMgrParam.eventListCnt=MAX_EVENT_LIST_CNT;
		}
		else if(param->eventListCnt==0)
		{
			eventMgrParam.eventListCnt=DEFAULT_EVENT_LIST_CNT;
		}
		else
		{
			eventMgrParam.eventListCnt=param->eventListCnt;
		}	
	}
	
	CTAILQ_INIT(&eventsTables.freeList.eventHead);
	CTAILQ_INIT(&eventsTables.freeList.eventLayerListHead);
	CTAILQ_INIT(&eventsTables.inuseList.eventLayerListHead);

	//#ifdef __KERNEL__
	
	TBL_MEM_ALLOC(eventPool, rtl865x_event_t, eventMgrParam.eventCnt);
	//#else
	//eventPool= (rtl865x_event_t *)malloc(eventMgrParam.eventCnt * sizeof(rtl865x_event_t)); 
	//#endif
	if(eventPool!=NULL)
	{
		memset( eventPool, 0, eventMgrParam.eventCnt * sizeof(rtl865x_event_t));	
	}
	else
	{
		return FAILED;
	}
	
	
	//#ifdef __KERNEL__
	TBL_MEM_ALLOC(eventLayerListPool, rtl865x_eventLayerList_t, eventMgrParam.eventListCnt);
	//#else
	//eventLayerListPool=(rtl865x_eventLayerList_t *)malloc(eventMgrParam.eventListCnt * sizeof(rtl865x_eventLayerList_t)); 
	//#endif	
	if(eventLayerListPool!=NULL)
	{
		memset( eventLayerListPool, 0, eventMgrParam.eventListCnt * sizeof(rtl865x_eventLayerList_t));	
	}
	else
	{
		return FAILED;
	}

	for(i = 0; i<eventMgrParam.eventCnt;i++)
	{
		CTAILQ_INSERT_HEAD(&eventsTables.freeList.eventHead, &eventPool[i], next);
	}
	
	for(i = 0; i<eventMgrParam.eventListCnt;i++)
	{
		CTAILQ_INSERT_HEAD(&eventsTables.freeList.eventLayerListHead, &eventLayerListPool[i], next);
	}
	
	eventMgrInitFlag=TRUE;
	return SUCCESS;
}


static int32 rtl865x_flushEventLayerList(rtl865x_eventLayerList_t  *eventLayerList)
{
	rtl865x_event_t *event;
	rtl865x_event_t *nextEvent;

	if(eventLayerList==NULL)
	{
		return SUCCESS;
	}
	
	event = CTAILQ_FIRST(&eventLayerList->eventHead);
	while(event)
	{
		nextEvent = CTAILQ_NEXT( event, next );
		event->eventId=0;
		event->eventPriority=0;
		event->event_action_fn=NULL;
		CTAILQ_REMOVE(&eventLayerList->eventHead, event, next);
		CTAILQ_INSERT_HEAD(&eventsTables.freeList.eventHead, event, next);
		event = nextEvent;
	}
	return SUCCESS;
}

int32 rtl865x_reInitEventMgr(void)
{
	rtl865x_eventLayerList_t *eventLayerList;
	rtl865x_eventLayerList_t *nextEventLayerList;

	if(eventMgrInitFlag==FALSE)
	{
		return FAILED;
	}
	
	eventLayerList = TAILQ_FIRST(&eventsTables.inuseList.eventLayerListHead);
	
	for (;eventLayerList!=NULL;eventLayerList=nextEventLayerList)
	{
		nextEventLayerList=TAILQ_NEXT(eventLayerList, next);
		rtl865x_flushEventLayerList(eventLayerList);
		
		CTAILQ_REMOVE(&eventsTables.inuseList.eventLayerListHead, eventLayerList, next);
		eventLayerList->eventLayerId=0;
		CTAILQ_INIT(&eventLayerList->eventHead);
		CTAILQ_INSERT_HEAD(&eventsTables.freeList.eventLayerListHead, eventLayerList, next);
	}
	return SUCCESS;
}

int32 rtl865x_destroyEventMgr(void)
{

	if(eventMgrInitFlag==FALSE)
	{
		return FAILED;
	}
	
	rtl865x_reInitEventMgr();
	#ifdef __KERNEL__
	if(eventPool!=NULL)
	{
		kfree(eventPool);
		eventPool=NULL;
	}

	if(eventLayerListPool!=NULL)
	{
		kfree(eventLayerListPool);
		eventLayerListPool=NULL;
	}
	#else
	if(eventPool!=NULL)
	{
		free(eventPool); 
		eventPool=NULL;
	}

	if(eventLayerListPool!=NULL)
	{
		free(eventLayerListPool); 
		eventLayerListPool=NULL;
	}
	#endif
	
	eventMgrInitFlag=FALSE;
	
	return SUCCESS;
}

static rtl865x_eventLayerList_t * rtl865x_searchEventLayerList(rtl865x_event_Param_t *eventParam)
{
	rtl865x_eventLayerList_t *eventLayerList;
	CTAILQ_FOREACH(eventLayerList, &eventsTables.inuseList.eventLayerListHead, next)
	{
		if(eventLayerList->eventLayerId==eventParam->eventLayerId)
		{
			return eventLayerList;
		}
	}

	return NULL;
}

static rtl865x_event_t * rtl865x_searchEvent(rtl865x_eventLayerList_t *eventList,rtl865x_event_Param_t *eventParam)
{
	rtl865x_event_t *event=NULL;
	
	if(eventParam==NULL)
	{
		return NULL;
	}
	
	CTAILQ_FOREACH(event, &eventList->eventHead, next)
	{
		if(	(event->eventId==eventParam->eventId) 
			&&(event->eventPriority==eventParam->eventPriority)
			&&(event->event_action_fn== eventParam->event_action_fn))
		{
			return event;
		}	
	}
	
	return NULL;
}

static void rtl865x_insertEventLayerList(rtl865x_eventLayerList_t *eventLayerList)
{
	rtl865x_eventLayerList_t *tmpEventLayerList=NULL;
	if(eventLayerList==NULL)
	{
		return;
	}
	
	CTAILQ_FOREACH(tmpEventLayerList, &eventsTables.inuseList.eventLayerListHead, next)
	{
		if(tmpEventLayerList->eventLayerId>=eventLayerList->eventLayerId)
		{
			CTAILQ_INSERT_BEFORE(&eventsTables.inuseList.eventLayerListHead,tmpEventLayerList,eventLayerList,next);
			break;	
		}
	}
	
	if(tmpEventLayerList==NULL)
	{
		CTAILQ_INSERT_TAIL(&eventsTables.inuseList.eventLayerListHead,eventLayerList,next);
	}
	return;
	
}

static void rtl8651_insertEvent(rtl865x_eventLayerList_t *eventLayerList,rtl865x_event_t *event)
{
	rtl865x_event_t *tmpEvent=NULL;
	if( (eventLayerList==NULL) ||(event==NULL))
	{
		return ; 
	}
	
	CTAILQ_FOREACH(tmpEvent, &eventLayerList->eventHead, next)
	{
	
		if((tmpEvent->eventId>event->eventId)||
			((tmpEvent->eventId== event->eventId) && (tmpEvent->eventPriority>=event->eventPriority)))
		{
			CTAILQ_INSERT_BEFORE(&eventLayerList->eventHead,tmpEvent,event,next);
			break;	
		}
	}

	if(tmpEvent==NULL)
	{
		CTAILQ_INSERT_TAIL(&eventLayerList->eventHead,event,next);
	}
	return ;
	
}

int32 rtl865x_registerEvent(rtl865x_event_Param_t *eventParam)
{
	rtl865x_eventLayerList_t *eventLayerList=NULL;
	rtl865x_event_t *event=NULL;
	
	if(eventMgrInitFlag==FALSE)
	{
		return FAILED;
	}
	
	if(eventParam==NULL)
	{
		return RTL_EINVALIDINPUT;
	}

	if(eventParam->eventLayerId==0)
	{
		return RTL_EINVALIDINPUT;
	}
	
	eventLayerList=rtl865x_searchEventLayerList(eventParam);
	if(eventLayerList==NULL)
	{
		eventLayerList = CTAILQ_FIRST(&eventsTables.freeList.eventLayerListHead);
		if(eventLayerList==NULL)
		{
			return RTL_ENOFREEBUFFER;
		}
		CTAILQ_REMOVE(&eventsTables.freeList.eventLayerListHead, eventLayerList, next);
		CTAILQ_INIT(&eventLayerList->eventHead);
		eventLayerList->eventLayerId=eventParam->eventLayerId;
		rtl865x_insertEventLayerList(eventLayerList);
	}

	if(	(eventParam->eventId< 0) || 
		(eventParam->eventId >MAX_SYSTEM_EVENT_ID) ||
		(eventParam->event_action_fn == NULL))
		return RTL_EINVALIDINPUT;

	
	/*check duplicate entry*/
	event=rtl865x_searchEvent(eventLayerList,eventParam);
	if(event!=NULL)
	{
		return RTL_EENTRYALREADYEXIST;
	}
	else
	{

		event = CTAILQ_FIRST(&eventsTables.freeList.eventHead);
		if(event==NULL)
		{
			return RTL_ENOFREEBUFFER;
		}
		CTAILQ_REMOVE( &eventsTables.freeList.eventHead, event, next);
		event->eventId= eventParam->eventId;
		event->eventPriority = eventParam->eventPriority;
		event->event_action_fn = eventParam->event_action_fn;
		
		rtl8651_insertEvent(eventLayerList,event);
	}

	return SUCCESS;
		
}

int32 rtl865x_unRegisterEvent(rtl865x_event_Param_t *eventParam)
{
	rtl865x_eventLayerList_t *eventLayerList=NULL;
	rtl865x_event_t *event=NULL;
	
	if(eventMgrInitFlag==FALSE)
	{
		return FAILED;
	}
		
	if(eventParam==NULL)
	{
		return RTL_EINVALIDINPUT;
	}
	
	eventLayerList=rtl865x_searchEventLayerList(eventParam);
	if(eventLayerList==NULL)
	{
		return RTL_EINVALIDINPUT;
	}

	if(	(eventParam->eventId< 0) || 
		(eventParam->eventId >MAX_SYSTEM_EVENT_ID) ||
		(eventParam->event_action_fn == NULL))
		return RTL_EINVALIDINPUT;

	
	event=rtl865x_searchEvent(eventLayerList,eventParam);
	if(event!=NULL)
	{
		event->eventId = 0;
		event->eventPriority = 0;
		event->event_action_fn = NULL;
		CTAILQ_REMOVE(&eventLayerList->eventHead, event, next);
		CTAILQ_INSERT_HEAD(&eventsTables.freeList.eventHead, event, next);
		
		if(CTAILQ_EMPTY(&eventLayerList->eventHead))
		{
			CTAILQ_REMOVE(&eventsTables.inuseList.eventLayerListHead, eventLayerList, next);
			eventLayerList->eventLayerId=0;
			CTAILQ_INIT(&eventLayerList->eventHead);
			CTAILQ_INSERT_HEAD(&eventsTables.freeList.eventLayerListHead, eventLayerList, next);
		}
		
		return SUCCESS;
	}
	else
	{
		return RTL_EINVALIDINPUT;
	}

	return SUCCESS;
		
}	


int32 rtl865x_raiseEvent(int32 eventId,void *actionParam)
{
	rtl865x_eventLayerList_t *eventLayerList;
	rtl865x_event_t *event;
	int retValue;

	if(eventMgrInitFlag==FALSE)
	{
		return FAILED;
	}
	
	CTAILQ_FOREACH(eventLayerList, &eventsTables.inuseList.eventLayerListHead, next)
	{
		CTAILQ_FOREACH(event, &eventLayerList->eventHead, next)
		{
			if((event->eventId==eventId))
			{	
				if(event->event_action_fn!=NULL)
				{
					do
					{
						retValue=event->event_action_fn(actionParam);
					}while(retValue==EVENT_RE_EXECUTE);
					
					switch(retValue)
					{
						
						case EVENT_STOP_EXECUTE:
							return SUCCESS;

						case EVENT_CONTINUE_EXECUTE:
							break;
							
						default:
							break;
					}
				}
			}	
		}
	
	}
	
	return SUCCESS;
}

void rtl865x_dumpAllEventLayerListInfo(void)
{
	
	rtl865x_eventLayerList_t *eventLayerList;
	rtl865x_event_t *event;
	int eventListCnt=0;
	int eventCnt=0;
	CTAILQ_FOREACH(eventLayerList, &eventsTables.inuseList.eventLayerListHead, next)
	{
		eventListCnt++;
		eventCnt=0;
		rtl865x_debug_printf(" No.%d eventLayerList:eventLayerId:%d \n",eventListCnt,eventLayerList->eventLayerId);
		CTAILQ_FOREACH(event, &eventLayerList->eventHead, next)
		{
			eventCnt++;
			rtl865x_debug_printf(" \tNo.%d event:eventId:%d,priority:%d,action_fn is 0x%x\n",eventCnt,event->eventId, event->eventPriority,(unsigned int)(event->event_action_fn));
		}
	
	}
	
	return;
}


#ifdef CONFIG_RTL865X_EVENT_PROC_DEBUG
#ifdef CONFIG_RTL_PROC_NEW
int32 rtl865x_event_proc_read(struct seq_file *s, void *v)
{
	rtl865x_eventLayerList_t *eventLayerList;
	rtl865x_event_t *event;
	int eventListCnt=0;
	int eventCnt=0;
	
	seq_printf(s, "%s\n", "realtek event management Table:");

	CTAILQ_FOREACH(eventLayerList, &eventsTables.inuseList.eventLayerListHead, next)
	{
		eventListCnt++;
		eventCnt=0;
		seq_printf(s, " No.%d eventLayerList:eventLayerId:0x%x \n",eventListCnt,eventLayerList->eventLayerId);
		CTAILQ_FOREACH(event, &eventLayerList->eventHead, next)
		{
			eventCnt++;
			seq_printf(s, " \tNo.%d event:eventId:0x%x,priority:%d,action_fn is 0x%x\n",eventCnt,event->eventId, event->eventPriority,(unsigned int)(event->event_action_fn));
		}
	
	}
	
	return 0;
}
#else
int32 rtl865x_event_proc_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len=0;
	rtl865x_eventLayerList_t *eventLayerList;
	rtl865x_event_t *event;
	int eventListCnt=0;
	int eventCnt=0;
	
	len = sprintf(page, "%s\n", "realtek event management Table:");

	CTAILQ_FOREACH(eventLayerList, &eventsTables.inuseList.eventLayerListHead, next)
	{
		eventListCnt++;
		eventCnt=0;
		len += sprintf(page+len, " No.%d eventLayerList:eventLayerId:0x%x \n",eventListCnt,eventLayerList->eventLayerId);
		CTAILQ_FOREACH(event, &eventLayerList->eventHead, next)
		{
			eventCnt++;
			len += sprintf(page+len, " \tNo.%d event:eventId:0x%x,priority:%d,action_fn is 0x%x\n",eventCnt,event->eventId, event->eventPriority,(unsigned int)(event->event_action_fn));
		}
	
	}
	
	if (len <= off+count) 
	{
		*eof = 1;
	}
	
	*start = page + off;
	len -= off;
	
	if (len>count)
	{
		len = count;
	}
	
	if (len<0)
	{
	  	len = 0;
	}

	return len;
}
#endif

int32  rtl865x_event_proc_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}
#endif


