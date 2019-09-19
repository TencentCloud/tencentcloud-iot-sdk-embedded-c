#ifdef EVENT_POST_ENABLED

#define EVENT_COUNTS     (3)

static TYPE_DEF_TEMPLATE_BOOL sg_status;
static TYPE_DEF_TEMPLATE_STRING sg_message[64+1];
static DeviceProperty g_propertyEvent_status_report[] = {

   {.key = "status", .data = &sg_status, .type = TYPE_TEMPLATE_BOOL},
   {.key = "message", .data = sg_message, .type = TYPE_TEMPLATE_STRING},
};

static TYPE_DEF_TEMPLATE_FLOAT sg_voltage;
static DeviceProperty g_propertyEvent_low_voltage[] = {

   {.key = "voltage", .data = &sg_voltage, .type = TYPE_TEMPLATE_FLOAT},
};

static TYPE_DEF_TEMPLATE_STRING sg_name[64+1];
static TYPE_DEF_TEMPLATE_INT sg_error_code;
static DeviceProperty g_propertyEvent_hardware_fault[] = {

   {.key = "name", .data = sg_name, .type = TYPE_TEMPLATE_STRING},
   {.key = "error_code", .data = &sg_error_code, .type = TYPE_TEMPLATE_INT},
};


static sEvent g_events[]={

    {
     .event_name = "status_report",
     .type = "info",
     .timestamp = 0,
     .eventDataNum = sizeof(g_propertyEvent_status_report)/sizeof(g_propertyEvent_status_report[0]),
     .pEventData = g_propertyEvent_status_report,
    },
    {
     .event_name = "low_voltage",
     .type = "alert",
     .timestamp = 0,
     .eventDataNum = sizeof(g_propertyEvent_low_voltage)/sizeof(g_propertyEvent_low_voltage[0]),
     .pEventData = g_propertyEvent_low_voltage,
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