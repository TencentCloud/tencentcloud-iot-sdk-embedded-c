##  数据模板业务逻辑开发
数据模板示例data_template_sample.c已实现数据、事件收发及响应的通用处理框架。可以基于此示例开发业务逻辑，上下行业务逻辑添加的入口函数分别为 deal_up_stream_user_logic 、deal_down_stream_user_logic。可以参考智能灯的场景示例 light_data_template_sample.c 添加业务处理逻辑。

### 下行业务逻辑实现
- 服务端下行的数据，SDK已按数据模板协议完成了json数据的解析。ProductDataDefine是第三步中根据在平台定义的产品数据模板生成的模板结构体，由定义的各属性构成成员变量。入参pData所指向的属性数据，从服务端下行数据中，SDK已经按数据模板协议完成了基础数据类型(整型、浮点、布尔等)的解析，如果有属性是定义为字符串或 JSON 类型，用户需要在函数 update_self_define_value 中完成字符串数据的解析，因为字符串数据是用户自定义的。
- 用户根据数据模板数据pData进行相应的业务逻辑处理。
```
/*用户需要实现的下行数据的业务逻辑,pData除字符串变量已实现用户定义的所有其他变量值解析赋值，待用户实现业务逻辑*/
static void deal_down_stream_user_logic(void *client, ProductDataDefine   * pData)
{
	Log_d("someting about your own product logic wait to be done");
}
```

- 示例用法:

```
/*智能灯属性数据模板*/
typedef struct _ProductDataDefine {
    TYPE_DEF_TEMPLATE_BOOL m_light_switch; 
    TYPE_DEF_TEMPLATE_ENUM m_color;
    TYPE_DEF_TEMPLATE_INT  m_brightness;
    TYPE_DEF_TEMPLATE_STRING m_name[MAX_STR_NAME_LEN+1];
} ProductDataDefine; 

/*示例灯光控制处理逻辑*/
static void deal_down_stream_user_logic(void *client,ProductDataDefine *light)
{
	int i;
    const char * ansi_color = NULL;
    const char * ansi_color_name = NULL;
    char brightness_bar[]      = "||||||||||||||||||||";
    int brightness_bar_len = strlen(brightness_bar);

	/*灯光颜色*/
	switch(light->m_color) {
	    case eCOLOR_RED:
	        ansi_color = ANSI_COLOR_RED;
	        ansi_color_name = " RED ";
	        break;
	    case eCOLOR_GREEN:
	        ansi_color = ANSI_COLOR_GREEN;
	        ansi_color_name = "GREEN";
	        break;
	    case eCOLOR_BLUE:
	        ansi_color = ANSI_COLOR_BLUE;
	        ansi_color_name = " BLUE";
	        break;
	    default:
	        ansi_color = ANSI_COLOR_YELLOW;
	        ansi_color_name = "UNKNOWN";
	        break;
	}


	/* 灯光亮度显示条 */		    
    brightness_bar_len = (light->m_brightness >= 100)?brightness_bar_len:(int)((light->m_brightness * brightness_bar_len)/100);
    for (i = brightness_bar_len; i < strlen(brightness_bar); i++) {
        brightness_bar[i] = '-';
    }

	if(light->m_light_switch){
        /* 灯光开启式，按照控制参数展示 */
		HAL_Printf( "%s[  lighting  ]|[color:%s]|[brightness:%s]|[%s]\n" ANSI_COLOR_RESET,\
					ansi_color,ansi_color_name,brightness_bar,light->m_name);
	}else{
		/* 灯光关闭展示 */
		HAL_Printf( ANSI_COLOR_YELLOW"[  light is off ]|[color:%s]|[brightness:%s]|[%s]\n" ANSI_COLOR_RESET,\
					ansi_color_name,brightness_bar,light->m_name);	
	}
}

```

- 字符串属性处理

```
//*如果有自定义的字符串或者json，需要在这里解析*/
static int update_self_define_value(const char *pJsonDoc, DeviceProperty *pProperty) 
{
    int rc = QCLOUD_ERR_SUCCESS;
		
	if((NULL == pJsonDoc)||(NULL == pProperty)){
		return QCLOUD_ERR_INVAL;
	}
	
	/*convert const char* to char * */
	char *pTemJsonDoc =HAL_Malloc(strlen(pJsonDoc) + 1);
	strcpy(pTemJsonDoc, pJsonDoc);

	char* property_data = LITE_json_value_of(pProperty->key, pTemJsonDoc);	
	
    if(property_data != NULL){
		if(pProperty->type == TYPE_TEMPLATE_STRING){
			/*如果多个字符串属性,根据pProperty->key值匹配，处理字符串*/					
			if(0 == strcmp("name", pProperty->key)){/*智能灯有定义一个name属性的字符串*/
				memset(sg_ProductData.m_name, 0, MAX_STR_NAME_LEN);
				strncpy(sg_ProductData.m_name, pProperty->data, MAX_STR_NAME_LEN);
				sg_ProductData.m_name[MAX_STR_NAME_LEN-1] = '\0';
			}
		}else if(pProperty->type == TYPE_TEMPLATE_JOBJECT){
			Log_d("Json type wait to be deal,%s",property_data);	
		}
		
		HAL_Free(property_data);
    }else{
		
		rc = QCLOUD_ERR_FAILURE;
		Log_d("Property:%s no matched",pProperty->key);	
	}
	
	HAL_Free(pTemJsonDoc);
		
    return rc;
}
```

###  上行业务逻辑实现

- 设备端根据业务场景需要，对设备端数据属性采取一定策略进行监测处理，用户可以在***deal_up_stream_user_logic***中将需要上报的属性更新给入参pReportDataList属性上报列表及需要上报的属性个数，数据模板的示例处理框架，在***IOT_Template_JSON_ConstructReportArray***中会将属性数据列表处理为数据模板的协议格式，***IOT_Template_Report***将数据发送给服务端。

```
/*用户需要实现的上行数据的业务逻辑,此处仅供示例*/
static int deal_up_stream_user_logic(DeviceProperty *pReportDataList[], int *pCount)
{
	int i, j;
	
		/*监测是否需要更新本地数据*/
		_local_property_refresh(); 

		/*将变化的属性更新到上报列表*/
     for (i = 0, j = 0; i < TOTAL_PROPERTY_COUNT; i++) {       
        if(eCHANGED == sg_DataTemplate[i].state) {
            pReportDataList[j++] = &(sg_DataTemplate[i].data_property);
			sg_DataTemplate[i].state = eNOCHANGE;
        }
    }
	*pCount = j;

	return (*pCount > 0)?QCLOUD_ERR_SUCCESS:QCLOUD_ERR_FAILURE;
}

```