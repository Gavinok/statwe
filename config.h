/* set static variables */
static const char bataddress[] = "BAT0"; // battery to get status from 
static int sleeptime = 1; // time to sleep in secounds

/* brightness settings */
static const char *cur_brightness = "/sys/class/backlight/intel_backlight/brightness";
static const char *max_brightness = "/sys/class/backlight/intel_backlight/max_brightness";
