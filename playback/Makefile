all: PlaybackSync.o
	g++ -o playback PlaybackSync.o -lfreeimage -lOpenNI

PlaybackSync.o: PlaybackSync.cpp
	g++ -g -c PlaybackSync.cpp -o PlaybackSync.o -I/usr/include/ni/
