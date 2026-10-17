# Setup VNC server for gui interface
vncserver -kill :1 2>/dev/null || true
vncserver :1 -geometry 1280x720 -depth 24 -localhost no