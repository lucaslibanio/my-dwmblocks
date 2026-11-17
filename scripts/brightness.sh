#!/bin/bash
# Brilho da tela em %. Rolar: +/- brilho
case $BLOCK_BUTTON in
	4) controle.sh bright_up ;;
	5) controle.sh bright_down ;;
esac

dev=$(ls -d /sys/class/backlight/* 2>/dev/null | head -1)
[ -n "$dev" ] || exit 0
echo "$(( $(cat "$dev/brightness") * 100 / $(cat "$dev/max_brightness") ))%"
