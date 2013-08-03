#include <SDL/sdl.h>
#include <stdio.h>
#include <stdlib.h>

#define WORLD_FILE_MAGIC_NUMBER 274263364
#define CHUNK_SIZE 16

void
usage(void){
	fprintf(stderr, "Usage: highmap2wrl <highmap.bmp> <outfile.wrl>\n");
	exit(1);
}

void write_chunk(int x, int y, int z, SDL_Surface *image, FILE *out);

int 
main(int argc, char *argv[]){
	SDL_Surface *image;
	FILE *out;
	if(argc != 3)
		usage();

	image = SDL_LoadBMP(argv[1]);
	if(image == NULL){
		fprintf(stderr, "Could not open file %s\n", argv[1]);
		exit(1);
	}

	out = fopen(argv[2], "w");
	if(out == NULL){
		fprintf(stderr, "Could not open output file %s for writing\n", argv[2]);
		exit(1);
	}

	if(image->format->BitsPerPixel != 8 || image->format->BytesPerPixel != 1){
		fprintf(stderr, "BMP Format error. BitsPerPixel must be 8 and BytesPerPixel must be 1.\n");
		printf("bitsperpixel = %d, bytesperpixel = %d\n", image->format->BitsPerPixel, image->format->BytesPerPixel);
		exit(1);
	}

	/* Magic number */
	Uint32 magic = WORLD_FILE_MAGIC_NUMBER;
	fwrite(&magic, sizeof(Uint32), 1, out);

	/* Size */
	Uint32 size[3];
	size[0] = image->w;
	size[1] = 256;
	size[2] = image->h;
	printf("%d %d %d\n", size[0], size[1], size[2]);
	fwrite(&size, sizeof(Uint32), 3, out);

	for(int i = 0; i < size[0] / CHUNK_SIZE; i++)
		for(int j = 0; j < size[1] / CHUNK_SIZE; j++)
			for(int k = 0; k < size[2] / CHUNK_SIZE; k++)
				write_chunk(i, j, k, image, out);


	return 0;
}

void
write_chunk(int x, int y, int z, SDL_Surface *image, FILE *out){
	for(int i = 0; i < CHUNK_SIZE; i++){
		for(int j = 0; j < CHUNK_SIZE; j++){
			for(int k = 0; k < CHUNK_SIZE; k++){
				int offset = x * CHUNK_SIZE + i + 
					    (z * CHUNK_SIZE + k) * image->w; 
				Uint8 *height_value = image->pixels + offset;
				Uint32 block = 0;
				//printf("a = %d b = %d c = %d\n", y * CHUNK_SIZE +j, *height_value, y * CHUNK_SIZE +j < *height_value);
				if(y * CHUNK_SIZE + j < *height_value){
					block = 0x80000000;
				}
				fwrite(&block, sizeof(Uint32), 1, out);
			}
		}
	}
}

