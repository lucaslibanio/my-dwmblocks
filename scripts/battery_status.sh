#!/bin/bash
# Bateria: mostra ⚡ quando está carregando. Clique: notificação com detalhes.
bat=/sys/class/power_supply/BAT0
capacity=$(cat "$bat/capacity")
status=$(cat "$bat/status")

[ "$BLOCK_BUTTON" = 1 ] && notify-send "Bateria" "$status — $capacity%"

case $status in
	Charging) echo "⚡$capacity%" ;;
	*)        echo "$capacity%" ;;
esac
