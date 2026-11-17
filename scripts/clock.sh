#!/bin/bash
# Data e hora (24h, em português). Clique: calendário do mês.
[ "$BLOCK_BUTTON" = 1 ] && notify-send "Calendário" "$(cal --color=never)"
LC_TIME=pt_BR.UTF-8 date '+%a %d %b %H:%M'
