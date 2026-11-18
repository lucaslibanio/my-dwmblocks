//Modify this file to change what commands output to your statusbar, and recompile using the make command.
// O sinal (última coluna) também identifica o bloco nos cliques (statuscmd):
// cada script recebe o botão do mouse na variável BLOCK_BUTTON.
//   pkill -RTMIN+<sinal> dwmblocks  -> atualiza o bloco na hora
#define S "~/.source/dwmblocks/scripts/"
static const Block blocks[] = {
	/*Icon*/	/*Command*/		/*Update Interval*/	/*Update Signal*/
	{"Kbd:",	S "keyboard.sh",	0,	6},
	{"Vol:",	S "volume.sh",		5,	1},
	{"Bri:",	S "brightness.sh",	0,	2},
	{"Bat:",	S "battery_status.sh",	30,	3},
	{"Mem:",	S "memory.sh",		30,	4},
	{"",		S "clock.sh",		10,	5},
};
#undef S

//sets delimiter between status commands. NULL character ('\0') means no delimiter.
static char delim[] = " | ";
static unsigned int delimLen = 5;
