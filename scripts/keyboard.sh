#!/bin/bash
# Layout do teclado: BR (ABNT2) ou US (internacional).
# Clique esquerdo: alterna (o mesmo que Super+Alt+K).
[ "$BLOCK_BUTTON" = 1 ] && layout_change.sh >/dev/null

setxkbmap -query 2>/dev/null | awk '/^layout/ { print toupper($2) }'
