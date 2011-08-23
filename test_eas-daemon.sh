killall lt-activesyncd
./eas-daemon/src/activesyncd &
if ps aux | grep '[/]eas-daemon/src/activesyncd'
then
    echo Running
else
    echo Not running
fi
make check

