#!/bin/bash
# Memória usada/total. Clique: os 5 processos que mais usam memória.
[ "$BLOCK_BUTTON" = 1 ] && notify-send "Memória" "$(ps axch -o cmd:18,%mem --sort=-%mem | head -5)"
free -h | awk '/^Mem/ { print $3"/"$2 }' | sed s/i//g
