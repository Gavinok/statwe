/* set static variables */
static const char bataddress[] = "BAT0"; // battery to get status from 
static int sleeptime = 2; // time to sleep in secounds

/* brightness settings */
static const char *cur_brightness = "/sys/class/backlight/intel_backlight/brightness";
static const char *max_brightness = "/sys/class/backlight/intel_backlight/max_brightness";

static const char *temperature_file = "/sys/class/thermal/thermal_zone0/temp";
static const char *maildir = "/home/gavinok/.local/share/mail/personal/INBOX/new/";
static int mail_enable = 1;

static const char *lemonfmt = "%%{r}%s\n";
