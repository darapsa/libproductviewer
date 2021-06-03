#ifndef PRODUCTVIEWER_H
#define PRODUCTVIEWER_H

#ifdef __cplusplus
extern "C" {
#endif

void pv_init(const char *model, void (*file_reader)(const char *path, char **data, size_t *len),
		const char *color, unsigned int width, unsigned int height);
void pv_draw(unsigned int width, unsigned int height);
void pv_rotate(float angle);
void pv_quit();

#ifdef __cplusplus
}
#endif

#endif
