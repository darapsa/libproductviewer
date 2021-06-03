#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include <tinyobj_loader_c.h>

static tinyobj_attrib_t attrib;
static tinyobj_material_t *materials = NULL;
static size_t num_materials;

static void read_file(void *ctx, const char *filename, const int is_mtl, const char *obj_filename,
		char **data, size_t *len)
{
	char *last_delim = strrchr(obj_filename,
#ifdef _WIN32
			'\\'
#else
			'/'
#endif
			);
	char path[strlen(obj_filename) - (last_delim ? strlen(last_delim) + 1 : 0) + 1];
	if (is_mtl && last_delim) {
		last_delim[0] = '\0';
		sprintf(path, "%s/%s", obj_filename, filename);
	} else
		strcpy(path, filename);
	((void (*)(const char *, char **, size_t *))ctx)(path, data, len);
}

void pv_parse(unsigned int *num_triangles, size_t *tex_name_len, const char *model, void *file_reader)
{
	tinyobj_shape_t *shapes = NULL;
	size_t num_shapes;
	tinyobj_parse_obj(&attrib, &shapes, &num_shapes, &materials, &num_materials, model, read_file, file_reader,
			TINYOBJ_FLAG_TRIANGULATE);
	tinyobj_shapes_free(shapes, num_shapes);
	*num_triangles = attrib.num_face_num_verts;
	*tex_name_len = strlen(materials[0].diffuse_texname);
}

void pv_convert(float *data, char *texture)
{
	static const size_t stride = 5;
	for (unsigned int i = 0; i < attrib.num_face_num_verts; i++) {
		tinyobj_vertex_index_t idx0 = attrib.faces[i * 3];
		tinyobj_vertex_index_t idx1 = attrib.faces[i * 3 + 1];
		tinyobj_vertex_index_t idx2 = attrib.faces[i * 3 + 2];
		float v[3][3];
		for (size_t j = 0; j < 3; j++) {
			v[0][j] = attrib.vertices[idx0.v_idx * 3 + j];
			v[1][j] = attrib.vertices[idx1.v_idx * 3 + j];
			v[2][j] = attrib.vertices[idx2.v_idx * 3 + j];
		}
		float t[][2] = {
			attrib.texcoords[idx0.vt_idx * 2], 1.0f - attrib.texcoords[idx0.vt_idx * 2 + 1],
			attrib.texcoords[idx1.vt_idx * 2], 1.0f - attrib.texcoords[idx1.vt_idx * 2 + 1],
			attrib.texcoords[idx2.vt_idx * 2], 1.0f - attrib.texcoords[idx2.vt_idx * 2 + 1]
		};
		for (size_t j = 0; j < 3; j++) {
			data[(i * 3 + j) * stride + 0] = v[j][0];
			data[(i * 3 + j) * stride + 1] = v[j][1];
			data[(i * 3 + j) * stride + 2] = v[j][2];
			data[(i * 3 + j) * stride + 3] = t[j][0];
			data[(i * 3 + j) * stride + 4] = t[j][1];
		}
	}
	strcpy(texture, materials[0].diffuse_texname);
	tinyobj_attrib_free(&attrib);
	tinyobj_materials_free(materials, num_materials);
}
