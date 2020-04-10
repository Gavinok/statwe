#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <dirent.h>
#include "config.h"
#include "getvol.h"

/* #define alloca(x)  __builtin_alloca(x) */
/* #define PATH_MAX 100 */
#define VERSION 0.1
#define MAXSTR  1024

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
void die(const char *errstr, ...);

/* Bar Elements */
char *base(char* base, int len);
int light();
int audio();
int normal();

/*
 * returns the amount of ram that is used by the system as a string
 */
const char *
ram_used(void)
{
	static char ram[MAXSTR];
	uintmax_t total, free, buffers, cached;

	FILE *infile = fopen("/proc/meminfo","r");
	if (infile == NULL)
		die("ram_used fopen: %s", strerror(errno));

	fscanf(infile,"MemTotal: %ju kB\nMemFree: %ju kB\nMemAvailable: %ju kB\nBuffers: %ju kB\nCached: %ju kB\n",
			&total,&free,&buffers,&buffers,&cached);
	fclose(infile);

	

	int len_needed = snprintf(ram, sizeof(ram), "%.0fM",(double)((total - free - buffers - cached)/1024));
	if (len_needed < 0 || (unsigned) len_needed >= sizeof(ram))
		die("ram_used snprintf len_needed = %d", len_needed);
	return ram;
}

/*
 * returns the time and date as a string
 */
const char *
datetime(const char *fmt)
{
	static char buf[MAXSTR];
	time_t t;

	t = time(NULL);
	if (!strftime(buf, MAXSTR, fmt, localtime(&t))) {
		die("strftime: %s", strerror(errno));
	}

	return buf;
}

/*
 * returns the contents of a file as a string
 */
const char *
filetostring(const char *file, char buf[MAXSTR])
{
	char *filebuffer = 0;
	long length;
	FILE *f = fopen(file, "rb");
	if (f == NULL)
		die("filetostring fopen: %s", strerror(errno));

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
const char *
battery_print(int perc, int charging)
{
	/* printf("state %d\n", charging); */
	if (charging)
		return "|🗲|";
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
int
battery_perc(const char *bat)
{
	int perc = -1;
	char path[PATH_MAX];
	char batbuf[MAXSTR];

	int len_needed = snprintf(path, PATH_MAX, "/sys/class/power_supply/%s/capacity", bat);
	if (len_needed < 0 || (unsigned) len_needed >= sizeof(path))
		die("battery_perc snprintf len_needed = %d", len_needed);

	const char *percstr = filetostring(path, batbuf);
	perc = atoi(percstr); 
	return perc;

}

int
battery_state(const char *bat)
{
	/* int state = -1; */
	int charging = 1;
	char path[PATH_MAX];
	char batbuf[MAXSTR];

	int len_needed = snprintf(path, sizeof(path), "/sys/class/power_supply/%s/status", bat);
	if (len_needed < 0 || (unsigned) len_needed >= sizeof(path))
		die("battery_state snprintf len_needed = %d", len_needed);

	const char *state = filetostring(path, batbuf);
	/* assume it is discharging if it is not full or charging*/
	if ((strncmp(state, "Charging", 3) == 0) || (strncmp(state, "Full", 3) == 0))
		charging = 1;
	else /*work around for broken charging apci*/
		charging = 0;

	return charging;
}

/*
 * returns: the battery status as a string
 */
const char *
battery_bar(const char *bat)
{
	int perc = battery_perc(bat);
	int state = battery_state(bat);


	if (perc < 0) {
		die("battery perc is %d", perc);
	}
	if (state < 0) {
		die("battery state is %d", state);
	}

	/* state */
	return battery_print(perc, state);
}

/*
 * returns: the brightness percentage as a float
 */
float 
brightness()
{
	char lightbuf[MAXSTR];
	int currentbrightness = atoi(filetostring(cur_brightness,lightbuf));    
	int maxbrightness = atoi(filetostring(max_brightness,lightbuf));    
	if(maxbrightness <= 0){
		die("error max brightness = %d\n", maxbrightness);
	}
	float percentbrightness = ((float)currentbrightness/maxbrightness)*100;
	return percentbrightness;
}

/*
 * returns: the current cpu temp
 */
int 
termals(const char *file)
{
	char tempbuf[MAXSTR];
	/* uintmax_t temp; */
	int temp;
	temp = atoi(filetostring(file,tempbuf));    
	temp = temp/1000;

	return temp;
}

int 
countmail(const char *dir)
{
	int file_count = 0;
	DIR * dirp;
	struct dirent * entry;

	dirp = opendir(dir); /* There should be error handling after this */
	while ((entry = readdir(dirp)) != NULL) {
		if (entry->d_type == DT_REG) { /* If the entry is a regular file */
			file_count++;
		}
	}
	closedir(dirp);
	return file_count;
}

/*
 * prints the way the command is intended to be used to standerd error
 */
void 
usage(void)
{
	fputs("usage: statwe [-bavh] \n", stderr);
	exit(1);
}

/*
 * Sets the rootwindow name
 */
void
XSetRoot(const char *name)
{
	Display *display;

	if (( display = XOpenDisplay(0x0)) == NULL ) {
		die("cannot open display! %s\n", strerror(errno));
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
int 
sleepie(int time)
{
	struct timespec tim, tim2;
	tim.tv_sec = time;
	tim.tv_nsec = 500;

	if(nanosleep(&tim , &tim2) < 0 )   
		die("nanosleep %s", strerror(errno));
	return 0;   
}

/* 
 * The base of the status bar
 */
char *
base(char* base, int len)
{
	int batperc = battery_perc(bataddress);
	const char * date = datetime("%a, %b %d %I:%M%p");
	const char * bar = battery_bar(bataddress);
	const char * ram = ram_used();
	int temp = termals(temperature_file);
	int mail;
 	if (mail_enable)
		mail = countmail(maildir);
	else
		mail = 0;

	int len_needed = -2;
	if (mail > 0)
		len_needed = snprintf(base, len, "[💌 %d] [%d°] [%s] %s %s%d%%", mail, temp, ram, date, bar, batperc);
	else
		len_needed = snprintf(base, len, "[%d°] [%s] %s %s%d%%", temp, ram, date, bar, batperc);

	if (len_needed < 0 || len_needed >= len)
			die("base snprintf len_needed = %d", len_needed);

	return base;
}

/*
 * returns: light percentage as in int
 */
int 
light()
{
	float brightperc = brightness();
	char start[MAXSTR], status[250];
	base(start, MAXSTR);

	int len_needed = snprintf(status, sizeof(status), " %.0f%%  %s", brightperc, start);
	if (len_needed < 0 || (unsigned) len_needed >= sizeof(status))
		die("light snprintf len_needed = %d", len_needed);
	
	XSetRoot(status);
	return 0;
}

/*
 * prints the normal status bar with the volume level added at the front
 */
int
audio()
{
	int vol = get_volume();
	vol++; // since this is for some reason off by one percent
	char start[200], status[250];
	base(start, 200);

	int len_needed = snprintf(status, sizeof(status), " %d%%  %s", vol, start);
	if (len_needed < 0 || (unsigned) len_needed >= sizeof(status))
		die("audio snprintf len_needed = %d", len_needed);

	XSetRoot(status);
	return 0;
}

/*
 * prints the status bar to the root window title
 */
int 
normal()
{
	char status[200];
	base(status, 200);
	XSetRoot(status);
	if(sleepie(sleeptime) < 0){
		return -1; 
	}
	return 0;
}

/*
 * kill the program and print error
 */
void
die(const char *errstr, ...)
{
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	XSetRoot(errstr);
	exit(1);
}

int
main(int argc, char *argv[])
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
