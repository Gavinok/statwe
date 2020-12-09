/**
 * @author      : Gavin Jaeger-Freeborn (gavinfreeborn@gmail.com)
 * @file        : getvol2.h
 * @created     : Tue 14 Apr 2020 01:03:49 AM
 */
#include <sys/soundcard.h>

int
get_volume(const char *card)
{
	size_t i;
	int v, afd, devmask;
	char *vnames[] = SOUND_DEVICE_NAMES;

	if ((afd = open(card, O_RDONLY | O_NONBLOCK)) < 0) {
		warn("open '%s':", card);
		return NULL;
	}

	if (ioctl(afd, (int)SOUND_MIXER_READ_DEVMASK, &devmask) < 0) {
		warn("ioctl 'SOUND_MIXER_READ_DEVMASK':");
		close(afd);
		return NULL;
	}
	for (i = 0; i < LEN(vnames); i++) {
		if (devmask & (1 << i) && !strcmp("vol", vnames[i])) {
			if (ioctl(afd, MIXER_READ(i), &v) < 0) {
				warn("ioctl 'MIXER_READ(%ld)':", i);
				close(afd);
				return NULL;
			}
		}
	}

	close(afd);

	return v & 0xff;
}
