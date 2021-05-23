#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <X11/Xlib.h>

#define INTERVAL 1 // seconds

#define NITEMS(arr) (sizeof(arr) / sizeof(arr[0]))

static volatile sig_atomic_t keep_running = true;
static volatile sig_atomic_t has_ticked = false;

static void handle_sig(int sig)
{
	switch (sig) {
	case SIGALRM:
		has_ticked = true;
		break;
	case SIGINT:
	case SIGTERM:
		keep_running = false;
		break;
	}
}

static int handle_xerror(Display *dpy, XErrorEvent *ev)
{
	char buf[256];

	(void)XGetErrorText(dpy, ev->error_code, buf, sizeof(buf));
	warnx("received X error: %s\n", buf);
	return 0;
}

static void set_time(Display *dpy)
{
	time_t now;
	char *str, *nl;

	errno = 0;
	if (time(&now) == (time_t)-1) {
		warn("unable to get current time");
		return;
	}

	errno = 0;
	str = ctime(&now);
	if (str == NULL) {
		warn("unable to convert time to string");
		return;
	}

	nl = strstr(str, "\n");
	if (nl != NULL) {
		*nl = '\0';
	}

	(void)XStoreName(dpy, DefaultRootWindow(dpy), str);
	(void)XFlush(dpy);
}

int main()
{
	const int sigs[] = { SIGALRM, SIGINT, SIGTERM };
	Display *dpy = NULL;
	struct sigaction sa;
	struct itimerval itv;

	(void)XSetErrorHandler(handle_xerror);

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		errx(EXIT_FAILURE, "unable to open display %s",
		     XDisplayName(NULL));
	}

	sa.sa_handler = handle_sig;
	(void)sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	for (size_t i = 0; i < NITEMS(sigs); i++) {
		errno = 0;
		if (sigaction(sigs[i], &sa, NULL) == -1) {
			err(EXIT_FAILURE, "unable to install signal handler");
		}
	}

	itv.it_interval.tv_sec = INTERVAL;
	itv.it_interval.tv_usec = 0;
	itv.it_value.tv_sec = 1;
	itv.it_value.tv_usec = 0;
	errno = 0;
	if (setitimer(ITIMER_REAL, &itv, NULL) == -1) {
		err(EXIT_FAILURE, "unable to set interval timer");
	}

	while (keep_running) {
		(void)pause();
		if (has_ticked) {
			set_time(dpy);
			has_ticked = false;
		}
	}

	(void)XCloseDisplay(dpy);

	return 0;
}
