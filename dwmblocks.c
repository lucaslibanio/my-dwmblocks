#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>
#include<fcntl.h>
#include<sys/wait.h>
#ifndef NO_X
#include<X11/Xlib.h>
#endif
#ifdef __OpenBSD__
#define SIGPLUS			SIGUSR1+1
#define SIGMINUS		SIGUSR1-1
#else
#define SIGPLUS			SIGRTMIN
#define SIGMINUS		SIGRTMIN
#endif
#define LENGTH(X)               (sizeof(X) / sizeof (X[0]))
#define CMDLENGTH		60
#define MIN( a, b ) ( ( a < b) ? a : b )
#define STATUSLENGTH (LENGTH(blocks) * CMDLENGTH + 1)

typedef struct {
	char* icon;
	char* command;
	unsigned int interval;
	unsigned int signal;
} Block;

#ifndef __OpenBSD__
void dummysighandler(int num);
#endif
void sighandler(int num);
void buttonhandler(int sig, siginfo_t *si, void *ucontext);
void getcmds(int time);
void getsigcmds(unsigned int signal);
void setupsignals();
void sighandler(int signum);
int getstatus(char *str, char *last);
void statusloop();
void termhandler();
void pstdout();
#ifndef NO_X
void setroot();
static void (*writestatus) () = setroot;
static int setupX();
static Display *dpy;
static int screen;
static Window root;
#else
static void (*writestatus) () = pstdout;
#endif


#include "blocks.h"

static char statusbar[LENGTH(blocks)][CMDLENGTH] = {0};
static char statusstr[2][STATUSLENGTH];
static int statusContinue = 1;
static int returnStatus = 0;

//opens process *cmd and stores output in *output
void getcmd(const Block *block, char *output)
{
	//make sure status is same until output is ready
	char tempstatus[CMDLENGTH] = {0};
	int i = 0;
	/* statuscmd: o número do sinal vai como um byte invisível antes do bloco,
	 * para o dwm saber em qual bloco o clique caiu */
	if (block->signal)
		tempstatus[i++] = block->signal;
	strcpy(tempstatus + i, block->icon);
	FILE *cmdf = popen(block->command, "r");
	if (!cmdf)
		return;
	i = strlen(tempstatus);
	fgets(tempstatus+i, CMDLENGTH-i-delimLen, cmdf);
	i = strlen(tempstatus);
	//if block and command output are both not empty
	if (i != 0) {
		//only chop off newline if one is present at the end
		i = tempstatus[i-1] == '\n' ? i-1 : i;
		if (delim[0] != '\0') {
			strncpy(tempstatus+i, delim, delimLen);
		}
		else
			tempstatus[i++] = '\0';
	}
	strcpy(output, tempstatus);
	pclose(cmdf);
}

void getcmds(int time)
{
	const Block* current;
	for (unsigned int i = 0; i < LENGTH(blocks); i++) {
		current = blocks + i;
		if ((current->interval != 0 && time % current->interval == 0) || time == -1)
			getcmd(current,statusbar[i]);
	}
}

void getsigcmds(unsigned int signal)
{
	const Block *current;
	for (unsigned int i = 0; i < LENGTH(blocks); i++) {
		current = blocks + i;
		if (current->signal == signal)
			getcmd(current,statusbar[i]);
	}
}

void setupsignals()
{
#ifndef __OpenBSD__
	    /* initialize all real time signals with dummy handler */
    for (int i = SIGRTMIN; i <= SIGRTMAX; i++)
        signal(i, dummysighandler);
#endif

	for (unsigned int i = 0; i < LENGTH(blocks); i++) {
		if (blocks[i].signal > 0)
			signal(SIGMINUS+blocks[i].signal, sighandler);
	}

	/* cliques vindos do dwm (statuscmd) chegam por SIGUSR1 */
	struct sigaction sa = {0};
	sa.sa_sigaction = buttonhandler;
	sa.sa_flags = SA_SIGINFO | SA_RESTART;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGUSR1, &sa, NULL);

}

int getstatus(char *str, char *last)
{
	strcpy(last, str);
	str[0] = '\0';
	for (unsigned int i = 0; i < LENGTH(blocks); i++)
		strcat(str, statusbar[i]);
	str[strlen(str)-strlen(delim)] = '\0';
	return strcmp(str, last);//0 if they are the same
}

#ifndef NO_X
void setroot()
{
	if (!getstatus(statusstr[0], statusstr[1]))//Only set root if text has changed.
		return;
	XStoreName(dpy, root, statusstr[0]);
	XFlush(dpy);
}

int setupX()
{
	dpy = XOpenDisplay(NULL);
	if (!dpy) {
		fprintf(stderr, "dwmblocks: Failed to open display\n");
		return 0;
	}
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);
	return 1;
}
#endif

void pstdout()
{
	if (!getstatus(statusstr[0], statusstr[1]))//Only write out if text has changed.
		return;
	printf("%s\n",statusstr[0]);
	fflush(stdout);
}


void statusloop()
{
	setupsignals();
	int i = 0;
	getcmds(-1);
	while (1) {
		getcmds(i++);
		writestatus();
		while (waitpid(-1, NULL, WNOHANG) > 0); /* recolhe processos de cliques */
		if (!statusContinue)
			break;
		sleep(1.0);
	}
}

#ifndef __OpenBSD__
/* this signal handler should do nothing */
void dummysighandler(int signum)
{
    return;
}
#endif

void sighandler(int signum)
{
	getsigcmds(signum-SIGPLUS);
	writestatus();
}

/* Executa o comando do bloco clicado com BLOCK_BUTTON=<botão> e depois
 * pede para o dwmblocks atualizar aquele bloco. */
void buttonhandler(int sig, siginfo_t *si, void *ucontext)
{
	int button = si->si_value.sival_int & 0xff;
	unsigned int bsig = si->si_value.sival_int >> 8;
	pid_t parent = getpid();
	const Block *current = NULL;

	for (unsigned int i = 0; i < LENGTH(blocks); i++)
		if (blocks[i].signal == bsig)
			current = blocks + i;
	if (!current)
		return;
	if (fork() == 0) {
		char b[4];
		pid_t pid;
		/* Este processo também se chama "dwmblocks": sem isto, um
		 * "pkill -RTMIN+n dwmblocks" feito pelo próprio script do bloco
		 * chegaria aqui, rodaria o bloco de novo (ainda com BLOCK_BUTTON)
		 * e entraria em ciclo infinito. */
		for (int s = SIGRTMIN; s <= SIGRTMAX; s++)
			signal(s, SIG_IGN);
		signal(SIGUSR1, SIG_IGN);
		snprintf(b, sizeof(b), "%d", button);
		setsid();
		signal(SIGCHLD, SIG_DFL);
		setenv("BLOCK_BUTTON", b, 1);
		if ((pid = fork()) == 0) {
			int fd = open("/dev/null", O_WRONLY);
			if (fd >= 0)
				dup2(fd, STDOUT_FILENO);
			execl("/bin/sh", "sh", "-c", current->command, (char *)NULL);
			_exit(127);
		}
		if (pid > 0)
			waitpid(pid, NULL, 0);
		kill(parent, SIGPLUS + bsig);
		_exit(0);
	}
}

void termhandler()
{
	statusContinue = 0;
}

int main(int argc, char** argv)
{
	for (int i = 0; i < argc; i++) {//Handle command line arguments
		if (!strcmp("-d",argv[i]))
			strncpy(delim, argv[++i], delimLen);
		else if (!strcmp("-p",argv[i]))
			writestatus = pstdout;
	}
#ifndef NO_X
	if (!setupX())
		return 1;
#endif
	delimLen = MIN(delimLen, strlen(delim));
	delim[delimLen++] = '\0';
	signal(SIGTERM, termhandler);
	signal(SIGINT, termhandler);
	statusloop();
#ifndef NO_X
	XCloseDisplay(dpy);
#endif
	return 0;
}
