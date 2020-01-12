#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include "config.h"
#include "getvol.h"

#define alloca(x)  __builtin_alloca(x)
#define PATH_MAX 100
#define VERSION 0.1
#define LEN(x) (sizeof (x) / sizeof *(x))
#define MAXSTR  1024
char buf[1024];

/* File Parsing */
int pscanf(const char *path, const char *fmt, ...);

/* Ram Usage Parsing*/
const char * ram_used(void);

/* Date And Time Formatting */
const char * datetime(const char *fmt);

/* Battery Status */
const char * battery_print(int perc, int charging);
int battery_perc(const char *bat);
int battery_state(const char *bat);
const char * battery_bar(const char *bat);

/* Brightness Parsing */
float brightness();

/* CPU Temperature Parsing */
int termals(const char *file);

/* helper functions */
static void usage(void);
static void XSetRoot(const char *name);
int sleepie(int time);

/* Bar Elements */
int light();
int audio();
int normal();

/*
 * Goes to the specified location and takes the formated 
 * string then stores the values in the specified variables.
 *
 * path: path to the file
 */
int pscanf(const char *path, const char *fmt, ...)
{
	FILE *fp;
	va_list ap;
	int n;

	if (!(fp = fopen(path, "r"))) {
		/* warn("fopen '%s':", path); */
		return -1;
	}
	va_start(ap, fmt);
	n = vfscanf(fp, fmt, ap);
	va_end(ap);
	fclose(fp);

	return (n == EOF) ? -1 : n;
}

/*
 * returns the amount of ram that is used by the system as a string
 */
const char * ram_used(void)
{
	uintmax_t total, free, buffers, cached;

	if (pscanf("/proc/meminfo",
		   "MemTotal: %ju kB\n"
		   "MemFree: %ju kB\n"
		   "MemAvailable: %ju kB\n"
		   "Buffers: %ju kB\n"
		   "Cached: %ju kB\n",
		   &total, &free, &buffers, &buffers, &cached) != 5) {
		return NULL;
	}

	static char ram[MAXSTR];
	snprintf(ram,sizeof(ram),"%.0fM",(double)((total - free - buffers - cached)/1024));
	return ram;
}

/*
 * returns the time and date as a string
 */
const char * datetime(const char *fmt)
{
    time_t t;

    t = time(NULL);
    if (!strftime(buf, sizeof(buf), fmt, localtime(&t))) {
	/* warn("strftime: Result string exceeds buffer size"); */
	return NULL;
    }

    return buf;
}

/*
 * returns the contents of a file as a string
 */
const char *filetostring(const char *file, char buf[MAXSTR])
{
    char *filebuffer = 0;
    long length;
    FILE *f = fopen(file, "rb");

    if (f)
    {
	fseek(f, 0, SEEK_END);
	length = ftell(f);
	fseek(f, 0, SEEK_SET);
	filebuffer = malloc(length);
	if(filebuffer)
	{
	    fread (filebuffer, 1, length, f);
	}
	fclose(f);
    }

    if(filebuffer && sizeof(filebuffer)< MAXSTR)
    {
	strncpy(buf, filebuffer, MAXSTR - 1);
	buf[MAXSTR-1] = '\0';
	free(filebuffer);
	return buf;
    }
    printf("failed to parse file");
    return NULL;
}

/*
 * returns: a string containing the battery status as a bar
 */
const char * battery_print(int perc, int charging)
{
    /* printf("state %d\n", charging); */
    if (charging)
	return "|🗲|";
    /* return "|🗲|"; */
    if ( perc <= 5)
	return "|!|";
    else if (perc >= 80)
	return "|▇|";
    else if (perc >= 60)
	return "|▅|";
    else if (perc >= 40)
	return "|▃|";
    else if (perc >= 20)
	return "|▂|";
    else
	return "|▁|";
}

/*
 * returns the battery percentage as an int
 */
int battery_perc(const char *bat)
{
    int perc = -1;
    char path[PATH_MAX];
    char batbuf[MAXSTR];

    if (snprintf(path, sizeof(path), "/sys/class/power_supply/%s/capacity", bat) < 0) {
	return -1;
    }
    const char *percstr = filetostring(path,batbuf);
    perc = atoi(percstr); 
    return perc;

}

int battery_state(const char *bat)
{
    /* int state = -1; */
    int charging = 1;
    char path[PATH_MAX];
    char batbuf[MAXSTR];

    if (snprintf(path, sizeof(path), "/sys/class/power_supply/%s/status", bat) < 0) {
	return -1;
    }

    const char *state = filetostring(path,batbuf);
    if (strncmp(state, "Discharging", 3) == 0)
	charging = 0;
    else if (strncmp(state, "Charging", 3) == 0)
	charging = 1;
    return charging;
}

/*
 * returns: the battery status as a string
 */
const char * battery_bar(const char *bat)
{
    int perc = battery_perc(bat);
    int state = battery_state(bat);


    if (perc < 0) {
	return "error assesing battery level";
    }
    if (state < 0) {
	return "error assesing state";
    }

    /* state */
    return battery_print(perc, state);
}

/*
 * returns: the brightness percentage as a float
 */
float brightness()
{
    char lightbuf[MAXSTR];
    int currentbrightness = atoi(filetostring(cur_brightness,lightbuf));    
    int maxbrightness = atoi(filetostring(max_brightness,lightbuf));    
    if(maxbrightness == 0){
	printf("error retreiving brightness percentage\n");
	return -1;
    }
    float percentbrightness = ((float)currentbrightness/maxbrightness)*100;
    return percentbrightness;
}

/*
 * returns: the current cpu temp
 */
int termals(const char *file)
{
    char tempbuf[MAXSTR];
    /* uintmax_t temp; */
    int temp;
    temp = atoi(filetostring(file,tempbuf));    
    temp = temp/1000;

    return temp;
}

/*
 * prints the way the command is intended to be used to standerd error
 */
static void usage(void)
{
    fputs("usage: statwe [-bavh] \n", stderr);
    exit(1);
}

/*
 * Sets the rootwindow name
 */
static void XSetRoot(const char *name)
{
    Display *display;

    if (( display = XOpenDisplay(0x0)) == NULL ) {
	fprintf(stderr, "[barM] cannot open display!\n");
	exit(1);
    }

    XStoreName(display, DefaultRootWindow(display), name);
    XSync(display, 0);

    XCloseDisplay(display);
}

/*
 * tels the program to wait 1 secound before runinng again
 *
 * returns 0 if successful
 */
int sleepie(int time)
{
    struct timespec tim, tim2;
    tim.tv_sec = time;
    tim.tv_nsec = 500;

    if(nanosleep(&tim , &tim2) < 0 )   
		return -1;
    return 0;   
}

/*
 * returns: light percentage as in int
 */
int light()
{
    int batperc = battery_perc(bataddress);
    const char * date = datetime("%a, %b %d %I:%M%p");
    const char * bar = battery_bar(bataddress);
    const char * ram = ram_used();
    float brightperc = brightness();
    int temp = termals("/sys/bus/platform/devices/coretemp.0/hwmon/hwmon4/temp3_input");
    char name[200];
    snprintf(name, sizeof(name), "light: %.0f%% [%d°] [%s] %s %s%d%%", brightperc, temp, ram, date, bar, batperc);
    XSetRoot(name);
    return 0;
}

/*
 * prints the normal status bar with the volume level added at the front
 */
int audio()
{
    int batperc = battery_perc(bataddress);
    const char * date = datetime("%a, %b %d %I:%M%p");
    const char * bar = battery_bar(bataddress);
    const char * ram = ram_used();
    int temp = termals("/sys/bus/platform/devices/coretemp.0/hwmon/hwmon4/temp3_input");
    char name[200];
    snprintf(name, sizeof(name), "Vol: %ld%% [%d°] [%s] %s %s%d%%", vol, temp,  ram, date, bar, batperc);
    XSetRoot(name);
    return 0;
	int vol = get_volume();
	vol++; // since this is for some reason off by one percent
}

/*
 * prints the status bar to the root window title
 */
int normal()
{
    int batperc = battery_perc(bataddress);
    const char * date = datetime("%a, %b %d %I:%M%p");
    const char * bar = battery_bar(bataddress);
    const char * ram = ram_used();
    int temp = termals("/sys/bus/platform/devices/coretemp.0/hwmon/hwmon4/temp3_input");
    char name[200];
    snprintf(name, sizeof(name), "[%d°] [%s] %s %s%d%%", temp, ram, date, bar, batperc);
    XSetRoot(name);
    if(sleepie(sleeptime) < 0){
	return -1; 
    }
    return 0;
}

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++){
	/* these options take no arguments */
	if (!strcmp(argv[i], "-b")){
	    if(light() < 0){
		return 1;
	    }
	    return 0;
	}//update brightness
	else if (!strcmp(argv[i], "-a")){
	    if(audio() < 0){
		return 1;
	    }
	    return 0;
	}//update volume
	else if (!strcmp(argv[i], "-h")) 
	    usage();
    }
    while(1){
	if(normal() < 0){
	    return 1;
	}
    }
    return 0;
}
