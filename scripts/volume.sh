#!/bin/bash
# Volume da saída padrão.
# Clique esquerdo: mudo | direito: pavucontrol | rolar: +/- volume
case $BLOCK_BUTTON in
	1) controle.sh vol_mute ;;
	3) setsid -f pavucontrol >/dev/null 2>&1 ;;
	4) controle.sh vol_up ;;
	5) controle.sh vol_down ;;
esac

# logo depois do login o PipeWire pode ainda não estar pronto: mostra "--"
# e o bloco tenta de novo no próximo intervalo (5 s, em blocks.h)
mute=$(pactl get-sink-mute @DEFAULT_SINK@ 2>/dev/null) || { echo "--"; exit 0; }
case "$mute" in
	*yes*) echo "mudo" ;;
	*)     pactl get-sink-volume @DEFAULT_SINK@ 2>/dev/null | grep -o '[0-9]*%' | head -1 ;;
esac
