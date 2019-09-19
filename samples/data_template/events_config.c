#ifdef EVENT_POST_ENABLED

#define EVENT_COUNTS     (3)

static TYPE_DEF_TEMPLATE_BOOL sg_bool;
static TYPE_DEF_TEMPLATE_INT sg_int;
static TYPE_DEF_TEMPLATE_STRING sg_str[64+1];
static TYPE_DEF_TEMPLATE_FLOAT sg_float;
static TYPE_DEF_TEMPLATE_ENUM sg_enum1;
static TYPE_DEF_TEMPLATE_TIME sg_time;
static DeviceProperty g_propertyEvent_all_function[] = {

   {.key = "bool", .data = &sg_bool, .type = TYPE_TEMPLATE_BOOL},
   {.key = "int", .data = &sg_int, .type = TYPE_TEMPLATE_INT},
   {.key = "str", .data = sg_str, .type = TYPE_TEMPLATE_STRING},
   {.key = "float", .data = &sg_float, .type = TYPE_TEMPLATE_FLOAT},
   {.key = "enum1", .data = &sg_enum1, .type = TYPE_TEMPLATEENUM},
   {.key = "time", .data = &sg_time, .type = TYPE_TEMPLATE_TIME},
};

static TYPE_DEF_TEMPLATE_BOOL sg_status;
static TYPE_DEF_TEMPLATE_STRING sg_message[64+1];
static DeviceProperty g_propertyEvent_status_report[] = {

   {.key = "status", .data = &sg_status, .type = TYPE_TEMPLATE_BOOL},
   {.key = "message", .data = sg_message, .type = TYPE_TEMPLATE_STRING},
};

static TYPE_DEF_TEMPLATE_STRING sg_name[64+1];
static TYPE_DEF_TEMPLATE_INT sg_error_code;
static DeviceProperty g_propertyEvent_hardware_fault[] = {

   {.key = "name", .data = sg_name, .type = TYPE_TEMPLATE_STRING},
   {.key = "error_code", .data = &sg_error_code, .type = TYPE_TEMPLATE_INT},
};


static sEvent g_events[]={

    {
     .event_name = "all_function",
     .type = "alert",
     .timestamp = 0,
     .eventDataNum = sizeof(g_propertyEvent_all_function)/sizeof(g_propertyEvent_all_function[0]),
     .pEventData = g_propertyEvent_all_function,
    },
    {
     .event_name = "status_report",
     .type = "info",
     .timestamp = 0,
     .eventDataNum = sizeof(g_propertyEvent_status_report)/sizeof(g_propertyEvent_status_report[0]),
     .pEventData = g_propertyEvent_status_report,
    },
    {
     .event_name = "hardware_fault",
     .type = "fault",
     .timestamp = 0,
     .eventDataNum = sizeof(g_propertyEvent_hardware_fault)/sizeof(g_propertyEvent_hardware_fault[0]),
     .pEventData = g_propertyEvent_hardware_fault,
    },
};

#endif
