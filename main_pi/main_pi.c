#define PY_SSIZE_T_CLEAN

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>

#include <Python.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>

#include "../includes/net_packet.h"
#include "./driver/lcd/lcd.h"

#define TT_DEV_PATH_NAME "/dev/lcd_dev"
#define TT_MAJOR_NUMBER 510
#define TT_MINOR_NUMBER 100
#define TT_DEV_NAME   "lcd_display"

#define IOCTL_MAGIC_NUMBER 'i'
#define IOCTL_CMD_LCD_INIT _IOWR(IOCTL_MAGIC_NUMBER, 0, int)
#define IOCTL_CMD_LCD_SET_STRING _IOWR(IOCTL_MAGIC_NUMBER, 1, struct str_info)

#define DELAY_WEATHER 600
#define DELAY_WINDOW 1
#define DELAY_SOUND 1
#define DELAY_LIGHT 1
#define DELAY_DISPLAY 1

#define STRING_LINE_NUM 7

struct status
{
    int window;
    int sound;
    int light;
};

pthread_t pthread_client, pthread_server;
struct net_options opt_client, opt_server;

int pm10, pm25, weather_id;

struct status status;
float window_rate_weather;

dev_t lcd_dev;
int lcd;

int timer = 1;
int string_line = 0;

char strings[STRING_LINE_NUM * 2][STRING_LEN_MAX] = 
{
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    ""
};

int get_py_info(char* py_path, char* py_func, int* rtn_value)
{
	PyObject *pName, *pModule, *pFunc;
    PyObject *pArgs, *pValue;
    int i;

    Py_Initialize();
    
    pName = PyUnicode_DecodeFSDefault(py_path);
    // Error checking of pName left out

    pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    if (pModule != NULL) {
        pFunc = PyObject_GetAttrString(pModule, py_func);
        //pFunc is a new reference

        if (pFunc && PyCallable_Check(pFunc)) {
            pArgs = PyTuple_New(0);
            pValue = PyObject_CallObject(pFunc, pArgs);
            Py_DECREF(pArgs);
            if (pValue != NULL) {
				*rtn_value = PyLong_AsLong(pValue);
                Py_DECREF(pValue);
            }
            else {
                Py_DECREF(pFunc);
                Py_DECREF(pModule);
                PyErr_Print();
                fprintf(stderr,"Call failed\n");
                return 1;
            }
        }
        else {
            if (PyErr_Occurred())
                PyErr_Print();
            fprintf(stderr, "Cannot find function \"%s\"\n", py_func);
        }
        Py_XDECREF(pFunc);
        Py_DECREF(pModule);
    }
    else {
        PyErr_Print();
        fprintf(stderr, "Failed to load \"%s\"\n", py_path);
        return 1;
    }
    
    if (Py_FinalizeEx() < 0) {
        return 120;
    }
    return 0;
}

void get_weather_info()
{
	char* py_path = "get_weather_info";

	get_py_info(py_path, "get_pm10", &pm10);
	get_py_info(py_path, "get_pm25", &pm25);
	get_py_info(py_path, "get_weather", &weather_id);

}

int net_init()
{
    int rtnval;
    opt_client.serverIP = "192.168.100.5";
    opt_client.isMain = 0;
    
    opt_server.serverIP = NULL;
    opt_server.isMain = 1;

    rtnval = pthread_create(&pthread_client, NULL, net_process, (void *)&opt_client);
    if(rtnval > 0)
    {
        printf("pthread error!\n");
        return -1;
    }

    sleep(1);

    rtnval = pthread_create(&pthread_server, NULL, net_process, (void *)&opt_server);
    if(rtnval > 0)
    {
        printf("pthread error!\n");
        return -1;
    }
    
    return 0;
}

int lcd_init()
{
    //lcd_dev =makedev(TT_MAJOR_NUMBER, TT_MINOR_NUMBER);
    //mknod(TT_DEV_PATH_NAME, S_IFCHR|0666, lcd_dev);
   
    lcd = open(TT_DEV_PATH_NAME, O_RDWR);
  
    if (lcd < 0) {
	   printf("fail to open lcd\n");
	   return -1;
    }

	ioctl(lcd, IOCTL_CMD_LCD_INIT, NULL);

    return 0;
}

float get_window_rate_weather(int id)
{
	int i=id/100;
	float rate = 1;
	if(i==2 || i==5 || (i==3 && id > 304) || (i==6 && id > 604) || (i==7 && id>701))
		rate = 0;
	else if((i==3 && id<=304) || (i==6 || id<=604) || id==701)
		rate = 0.2;
	else if(id>801)
		rate = 0.5;

	if(pm10 > 50 || pm25 > 100)
		rate *=0.25;
	return rate;
}

char* get_weather_condition(int id)
{
	int i = id/100;

	if(i==2)
	{
		return "Thunderstorm";
	}
	else if(i==3)
	{
		return "Drizzle";
	}
	else if(i==5)
		return "Rain";
	else if(i==6)
		return "Snow";
	else if(i==7)
	{
		if(id==701)
			return "Mist";
		return "Bad Atmosphere";
	}
	else if(i==8)
	{
		if(id!=800)
			return "Clouds";
	}
	return "Clear";
}
void display_lcd(char* line1, char* line2)
{
    struct str_info s;
	
    s.line = 1;
    strcpy(s.string,line1);
	ioctl(lcd, IOCTL_CMD_LCD_SET_STRING, &s);
            
    s.line = 0;
    strcpy(s.string,line2);
	ioctl(lcd, IOCTL_CMD_LCD_SET_STRING, &s);
}

void control_window()
{
    if(timer % DELAY_WINDOW != 0)
        return;

    if(sensor_value.gas > 1000)
	    control_value.window = (307*get_window_rate_weather(weather_id) + 77);
    else
	    control_value.window = 77;

    printf("window %d\n",control_value.window);
}
void control_light()
{
    if(timer % DELAY_LIGHT != 0)
        return;
    if(sensor_value.light < 400)
	    control_value.led = 1024;
    else if(sensor_value.light > 600)
	    control_value.led = 0;

    printf("led : %d\n", control_value.led);
}
void control_sound()
{
    if(timer % DELAY_SOUND != 0)
        return;
    if(sensor_value.sound < 30)
	    return;
    if(sensor_value.sound <=120)
    {
	    if(control_value.sound >= 25)
		    return;
	    control_value.sound ++;
    }
    else if(sensor_value.sound > 400)
    {
	    if(control_value.sound <= 5)
		    return;
	    control_value.sound --;
    }
    printf("sound : %d\n",control_value.sound);
}
void get_weather_timer()
{
    if(timer % DELAY_WEATHER != 0)
        return;
    
    get_weather_info();
}

void refresh_display()
{

    if(timer % DELAY_DISPLAY != 0)
        return;
        

    //LCD표시 -> 미세먼지 / 날씨 / 온도 / 습도 / 창문 개폐상태 / 조명 온오프상태 / 마이크 소리 %

    //미세먼지
    sprintf(strings[0],"PM25 : %d",pm25);
    sprintf(strings[1],"PM10 : %d",pm10);

    //날씨
    sprintf(strings[2],"Weahter");
    sprintf(strings[3],"%s",get_weather_condition(weather_id));

    //온도 / 습도
    sprintf(strings[4],"Temp : %d",sensor_value.temp);
    sprintf(strings[5],"Humi : %d%%",sensor_value.humi);

    //창문 개폐상태
    sprintf(strings[6],"Window : %s",(control_value.window > 77 ? "Open":"Closed"));
    sprintf(strings[7],"%d%%",(int)(((float)control_value.window - 77.0) / 307.0 * 100.0));

    //조명
    sprintf(strings[8],"Light : %s",(control_value.led > 0) ? "On":"Off");
    sprintf(strings[9],"%d%%",(int)(((float)control_value.led / 1024.0) * 100.0));

    //마이크
    sprintf(strings[10],"Sound : %d%%",(int)(((float)control_value.sound - 5.0)/25.0 * 100.0));
    sprintf(strings[11],"");

    //CO2 ppm
    sprintf(strings[12],"CO2(ppm)");
    sprintf(strings[13],"%d",sensor_value.gas);

    display_lcd(strings[string_line],strings[string_line+1]);

    string_line += 2;
    if(string_line >= STRING_LINE_NUM * 2)
        string_line = 0;
}

int main(int argc, char* argv[])
{
    sensor_value.gas = -1;
    control_value.sound = 5;
    timer = 0;

    net_init();
    printf("net init\n");

    lcd_init();
    printf("lcd_init\n");

    get_weather_info();

	//날씨 구분
	//날씨 맑을떄 -> 창문 활짝
	//날씨 눈올때, 흐릴때, 미세먼지 심할때 -> 창문 쬐금만
	//날씨 찔끔 비올때 -> 창문 아주아주 쪼금만
	//날씨 태풍 와장창 -> 창문 ㄴㄴ
	//가스 -> 1000ppm 기준으로 열고 닫고
	//빛 -> 밝으면 LED 끄고 어두우면 LED 키고
	//소리 -> 3개 기준값으로 조절
	
    while(1)
    {
        sleep(1);
        timer ++;

        get_weather_timer();
        refresh_display();
        if(sensor_value.gas == -1)
            continue;
	
        control_window();
        control_light();
        control_sound();

    }
    
    close(lcd);
    return 0;
}
