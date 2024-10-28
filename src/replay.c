#include "simple_logger.h"
#include "replay.h"


FILE* open_replay(int id) {
	FILE* file;
	char filename[30];
	sprintf(filename, "replays/replay%i.bin", id);
	file = fopen(filename, "rb");
	if (!file) {
		//slog("no replay dir");
		sprintf(filename, "replay%i.bin", id);
		file = fopen(filename, "rb");
		if (!file) {
			slog("can't open replay");
			return NULL;
		}
	}
	return file;
}
int get_replay_size(FILE* file) {
	//int size;
	fseek(file, 0L, SEEK_END);
	//size = ftell(file);
	return ftell(file);
}


int read_replay(FILE* file) {
	int i;
	if (fread(&i, sizeof(int), 1, file)) {
		//slog("in: %i", i);
		return i;
	}
	
	return 0;
}

FILE* refresh_temp_replay(FILE* file) {
	if (file) {
		fclose(file);
	}

	char filename[30];
	sprintf(filename, "replays/tempreplay.bin");
	file = fopen(filename, "wb");
	if (!file) {
		//slog("no replay dir");
		sprintf(filename, "tempreplay.bin");
		file = fopen(filename, "wb");
		if (!file) {
			slog("can't open replay");
			return NULL;
		}
	}
	return file;
}

void append_to_temp_replay(int inputs, FILE* file) {
	fwrite(&inputs, sizeof(int), 1, file);
}

void save_temp_replay(int id, FILE* file) {
	int i;
	FILE* newfile;
	char filename[30];

	sprintf(filename, "replays/replay%i.bin", id);
	newfile = fopen(filename, "wb");
	if (!newfile) {
		//slog("no replay dir");
		sprintf(filename, "replay%i.bin", id);
		newfile = fopen(filename, "wb");
		if (!newfile) {
			slog("can't open replay");
			return;
		}
	}
	fclose(file);

	sprintf(filename, "replays/tempreplay.bin");
	file = fopen(filename, "rb");
	if (!file) {
		//slog("no replay dir");
		sprintf(filename, "tempreplay.bin");
		file = fopen(filename, "rb");
		if (!file) {
			slog("can't open replay");
			return;
		}
	}
	while (fread(&i, sizeof(int), 1, file)) {
		fwrite(&i, sizeof(int), 1, newfile);
		//slog("in: %i", i);
	}
	fclose(newfile);
}