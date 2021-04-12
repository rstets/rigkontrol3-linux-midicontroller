/*
 * This utility converts RigKontrol3 Input Events to ALSA MIDI events.
 *
 * The RigKontrol3 sends key events (from the snd_usb_caiaq ALSA module).  This
 * utility runs as a daemon to grab the events so they don't go anywhere else
 * and sends them as MIDI events thru an ALSA sequencer port.  The expression
 * pedal is capable of sending values with more detail than the range of
 * 0...127.  Output is converted to the range of 0...127 by default, set the
 * HIGHRES setting to change that.
 *
 *
 * AUTHOR: Xendarboh <xendarboh@gmail.com>
 * SOURCE: https://code.google.com/p/rigkontrol3-linux-midicontroller/
 * BIRTHDATE: 2012-12-15 20:40:39
 * References:
 *	http://permalink.gmane.org/gmane.linux.audio.devel/18481
 *	http://stackoverflow.com/questions/929103/convert-a-number-range-to-another-range-maintaining-ratio
 *	http://www.itp.uzh.ch/~dpotter/howto/daemonize
*/


#include <linux/input.h>
#include <alsa/asoundlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <regex.h>

#define CLIENT_NAME "RigKontrol3"
#define PORT_NAME   "Events"
#define CHANNEL 15

/* Buttons output MIDI notes */
#define BUTTON_1 1
#define BUTTON_2 2
#define BUTTON_3 3
#define BUTTON_4 4
#define BUTTON_5 5
#define BUTTON_6 6
#define BUTTON_7 7
#define BUTTON_8 8

/* Pedal MIDI output */
#define BUTTON_9 9
#define EXPRESSION_FINE   43
#define EXPRESSION_COARSE 11

/* return coarse or fine values from the expression pedal, set to 1 to enable
 * higher values than the standard coarse range of 0...127 */
#define HIGHRES 0

#define convertRange(oldMin, oldMax, newMin, newMax, oldVal) ((((oldVal - oldMin) * (newMax - newMin)) / (oldMax - oldMin)) + newMin)

static void daemonize(void)
{
	pid_t pid, sid;

	/* already a daemon */
	if ( getppid() == 1 ) return;

	/* Fork off the parent process */
	pid = fork();
	if (pid < 0)
		exit(1);

	/* If we got a good PID, then we can exit the parent process. */
	if (pid > 0)
		exit(0);

	/* At this point we are executing as the child process */

	/* Change the file mode mask */
	umask(0);

	/* Create a new SID for the child process */
	sid = setsid();
	if (sid < 0)
		exit(1);

	/* Change the current working directory.  This prevents the current
	directory from being locked; hence not being able to remove it. */
	if ((chdir("/")) < 0)
		exit(1);

	/* Redirect standard files to /dev/null */
	freopen( "/dev/null", "r", stdin);
	freopen( "/dev/null", "w", stdout);
	freopen( "/dev/null", "w", stderr);
}

int main(int argc, char *argv[])
{
	struct input_event ie;
	int fd, port, err;
	snd_seq_t *seq;
	snd_seq_event_t ev;
	int val;
	
	/* min and max values to calibrate expression pedal to a range starting from 0.
	actual values usually appear near the range of 60...450 (on my device) */
	int expressionPedalMax = 400;
	int expressionPedalMin = 64;

	/* prevent sending consecutive events of the same value */
	int expressionPedalLastVal = -1;


	if (argc < 2)
	{
		fprintf(stderr, "Input Device Required\nUSAGE: %s </dev/input/eventXX>\n", argv[0]);
		return 1;
	}
	regex_t regex;
	int reti = regcomp(&regex, "^/dev/input/event", 0);
	if (reti)
	{
		fprintf(stderr, "Could not compile regex\n");
		return 1;
	}
        reti = regexec(&regex, argv[1], 0, NULL, 0);
	if (reti)
	{
		fprintf(stderr, "Device must be an input event, exiting.\n");
		return 1;
	}
	regfree(&regex);


	fd = open(argv[1], O_RDONLY);
	if (fd == -1)
	{
		perror(argv[1]);
		return 1;
	}

	/* grab input to avoid leaking key events */
	err = ioctl(fd, EVIOCGRAB, 1);
	if (err != 0)
	{
		fprintf(stderr, "RigKontrol3 Events could not be grabbed.\n");
		return 1;
	}

	err = snd_seq_open(&seq, "default", SND_SEQ_OPEN_OUTPUT, 0);
	if (err < 0)
	{
		fprintf(stderr, "cannot open sequencer: %s\n", snd_strerror(err));
		return 1;
	}
	snd_seq_set_client_name(seq, CLIENT_NAME);
	port = snd_seq_create_simple_port(seq, PORT_NAME,
					  SND_SEQ_PORT_CAP_READ |
					  SND_SEQ_PORT_CAP_SUBS_READ,
					  SND_SEQ_PORT_TYPE_MIDI_GENERIC |
					  SND_SEQ_PORT_TYPE_SOFTWARE);
	if (port < 0)
	{
		fprintf(stderr, "cannot create port: %s\n", snd_strerror(port));
		return 1;
	}

	/* now that startup was sucessful, become a daemon */
	daemonize();

	snd_seq_ev_clear(&ev);
	snd_seq_ev_set_source(&ev, port);
	snd_seq_ev_set_subs(&ev);
	snd_seq_ev_set_direct(&ev);
	for (;;)
	{
		if (read(fd, &ie, sizeof(ie)) != sizeof(ie))
		{
			fprintf(stderr, "exiting\n");
			break;
		}
		ev.type = SND_SEQ_EVENT_NONE;
		switch (ie.type)
		{
		case EV_KEY:
			switch (ie.code)
			{
			case KEY_1:
				if (ie.value)
					snd_seq_ev_set_noteon(&ev, CHANNEL, BUTTON_5, 127);
				else
					snd_seq_ev_set_noteoff(&ev, CHANNEL, BUTTON_5, 64);
				break;
			case KEY_2:
				if (ie.value)
					snd_seq_ev_set_noteon(&ev, CHANNEL, BUTTON_6, 127);
				else
					snd_seq_ev_set_noteoff(&ev, CHANNEL, BUTTON_6, 64);
				break;
			case KEY_3:
				if (ie.value)
					snd_seq_ev_set_noteon(&ev, CHANNEL, BUTTON_7, 127);
				else
					snd_seq_ev_set_noteoff(&ev, CHANNEL, BUTTON_7, 64);
				break;
			case KEY_4:
				if (ie.value)
					snd_seq_ev_set_noteon(&ev, CHANNEL, BUTTON_8, 127);
				else
					snd_seq_ev_set_noteoff(&ev, CHANNEL, BUTTON_8, 64);
				break;
			case KEY_5:
				if (ie.value)
					snd_seq_ev_set_noteon(&ev, CHANNEL, BUTTON_1, 127);
				else
					snd_seq_ev_set_noteoff(&ev, CHANNEL, BUTTON_1, 64);
				break;
			case KEY_6:
				if (ie.value)
					snd_seq_ev_set_noteon(&ev, CHANNEL, BUTTON_2, 127);
				else
					snd_seq_ev_set_noteoff(&ev, CHANNEL, BUTTON_2, 64);
				break;
			case KEY_7:
				if (ie.value)
					snd_seq_ev_set_noteon(&ev, CHANNEL, BUTTON_3, 127);
				else
					snd_seq_ev_set_noteoff(&ev, CHANNEL, BUTTON_3, 64);
				break;
			case KEY_8:
				if (ie.value)
					snd_seq_ev_set_noteon(&ev, CHANNEL, BUTTON_4, 127);
				else
					snd_seq_ev_set_noteoff(&ev, CHANNEL, BUTTON_4, 64);
				break;
			case KEY_9:
				if (ie.value)
					snd_seq_ev_set_noteon(&ev, CHANNEL, BUTTON_9, 127);
				else
					snd_seq_ev_set_noteoff(&ev, CHANNEL, BUTTON_9, 64);
				break;
			}
			break;

		case EV_ABS:
			switch (ie.code)
			{
			case ABS_Z:
				val = ie.value;
				if (val > expressionPedalMax)
					val = expressionPedalMax;

				if (val < expressionPedalMin)
					val = expressionPedalMin;

				if (HIGHRES)
				{
					/* adjust lowest value to 0 */
					val = val - expressionPedalMin;
					if (val != expressionPedalLastVal)
						snd_seq_ev_set_controller(&ev, CHANNEL, EXPRESSION_FINE, val);
				} else
				{
					/* normalize value into the range of 0..127 */
					val = (int) convertRange(expressionPedalMin, expressionPedalMax, 0, 127, val);
					if (val != expressionPedalLastVal)
						snd_seq_ev_set_controller(&ev, CHANNEL, EXPRESSION_COARSE, val);
				}
				expressionPedalLastVal = val;
				break;
			}
			break;

		}
		if (ev.type != SND_SEQ_EVENT_NONE)
			snd_seq_event_output_direct(seq, &ev);
	}
	snd_seq_close(seq);
	close(fd);
	return 0;
}
