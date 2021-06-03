#ifdef __APPLE__
#include <ES2/gl.h>
#else
#include <GLES2/gl2.h>
#endif
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#if defined __ANDROID__ && defined DEBUG
#include <android/log.h>
#endif

static struct {
	void (*file_reader)(const char *, char **, size_t *);
	GLfloat matrix[4][4];
	GLfloat angle;
	GLsizei num_triangles, num_textures;
	GLuint program, buffers[1], textures[];
} model = {
	.angle = 0.0f,
	.num_textures = 0
};
static const GLfloat pi = 3.1415926535897932384626433832795f;

void pv_parse(GLsizei *num_triangles, size_t *tex_name_len, const char *model, void *file_reader);
void pv_convert(float *data, char *texture);

static GLuint compile(GLenum type, const char *shader)
{
	GLchar *data;
	size_t len;
	model.file_reader(shader, &data, &len);
	GLuint id = glCreateShader(type);
	glShaderSource(id, 1, &(const GLchar *){data}, NULL);
	glCompileShader(id);
	GLint compiled;
	glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
#ifdef DEBUG
		GLint len = 0;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
		if (len > 1) {
			char log[len + 1];
			glGetShaderInfoLog(id, len, NULL, log);
#ifdef __ANDROID__
			__android_log_print(ANDROID_LOG_ERROR, "libproductviewer", "%s", log);
#else
			printf("%s\n", log);
#endif
		}
#endif
		glDeleteShader(id);
		return 0;
	}
	return id;
}

void pv_texturize(const char *texture)
{
	GLsizei w, h;
	int comp;
	unsigned char *image = stbi_load(texture, &w, &h, &comp, STBI_default);
	GLint format = GL_RGBA;
	if (comp == 3)
		format = GL_RGB;
	glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, image);
	stbi_image_free(image);
}

void pv_init(const char *obj, void (*file_reader)(const char *path, char **data, size_t *len),
		const char *color, unsigned int width, unsigned int height)
{
	size_t tex_name_len;
	model.file_reader = file_reader;
	pv_parse(&model.num_triangles, &tex_name_len, obj, file_reader);
	GLfloat data[sizeof(GLfloat) * model.num_triangles * 5];
	char texture[tex_name_len + 1];
	pv_convert(data, texture);
	GLfloat rgba[4] = { [3] = 1.0f };
	for (size_t i = 0; i < strlen(color) / 2; i++) {
		GLuint value;
		sscanf((char[]){ color[i * 2], color[i * 2 + 1], '\0' }, "%x", &value);
		rgba[i] = (GLfloat)value / 255.0f;
	}
	glClearColor(rgba[0], rgba[1], rgba[2], rgba[3]);
	glViewport(0, 0, width, height);
	glEnable(GL_CULL_FACE);
	model.program = glCreateProgram();
	GLuint vert_id = compile(GL_VERTEX_SHADER, "product.vert");
	glAttachShader(model.program, vert_id);
	glDeleteShader(vert_id);
	GLuint frag_id = compile(GL_FRAGMENT_SHADER, "product.frag");
	glAttachShader(model.program, frag_id);
	glDeleteShader(frag_id);
	glLinkProgram(model.program);
	glUseProgram(model.program);
	static const GLsizei stride = sizeof(GLfloat) * 5;
	GLuint data_id;
	glGenBuffers(1, &data_id);
	glBindBuffer(GL_ARRAY_BUFFER, data_id);
	model.buffers[0] = data_id;
	glBufferData(GL_ARRAY_BUFFER, stride * model.num_triangles * 3, data, GL_STATIC_DRAW);
	GLint position = glGetAttribLocation(model.program, "a_position");
	glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(position);
	GLint texcoord = glGetAttribLocation(model.program, "a_texcoord");
	glVertexAttribPointer(texcoord, 2, GL_FLOAT, GL_FALSE, stride, (const void *)(sizeof(GLfloat) * 3));
	glEnableVertexAttribArray(texcoord);
	glActiveTexture(GL_TEXTURE0);
	GLuint tex_id;
	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_2D, tex_id);
	model.textures[model.num_textures++] = tex_id;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glUniform1i(glGetUniformLocation(model.program, "s_texture"), 0);
	pv_texturize(texture);
}

static void identity(GLfloat matrix[][4])
{
	memset(matrix, 0x0, sizeof(GLfloat[4][4]));
	for (int i = 0; i < 4; i++)
		matrix[i][i] = 1.0f;
}

static void multiply(GLfloat matrix[][4], GLfloat a[][4], GLfloat b[][4])
{
	GLfloat tmp[4][4];
	for (int i = 0; i < 4; i++) {
		tmp[i][0]
			= a[i][0] * b[0][0]
			+ a[i][1] * b[1][0]
			+ a[i][2] * b[2][0]
			+ a[i][3] * b[3][0];
		tmp[i][1]
			= a[i][0] * b[0][1]
			+ a[i][1] * b[1][1]
			+ a[i][2] * b[2][1]
			+ a[i][3] * b[3][1];
		tmp[i][2]
			= a[i][0] * b[0][2]
			+ a[i][1] * b[1][2]
			+ a[i][2] * b[2][2]
			+ a[i][3] * b[3][2];
		tmp[i][3]
			= a[i][0] * b[0][3]
			+ a[i][1] * b[1][3]
			+ a[i][2] * b[2][3]
			+ a[i][3] * b[3][3];
	}
	memcpy(matrix, tmp, sizeof(GLfloat[4][4]));
}

static void translate(GLfloat matrix[][4], GLfloat x, GLfloat y, GLfloat z)
{
	matrix[3][0] += (matrix[0][0] * x + matrix[1][0] * y + matrix[2][0] * z);
	matrix[3][1] += (matrix[0][1] * x + matrix[1][1] * y + matrix[2][1] * z);
	matrix[3][2] += (matrix[0][2] * x + matrix[1][2] * y + matrix[2][2] * z);
	matrix[3][3] += (matrix[0][3] * x + matrix[1][3] * y + matrix[2][3] * z);
}

static void rotate(GLfloat matrix[][4], GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	angle *= pi / 180.0f;
	GLfloat sin_angle = sinf(angle);
	GLfloat cos_angle = cosf(angle);
	GLfloat mag = sqrtf(x * x + y * y + z * z);
	if (mag > 0.0f) {
		x /= mag;
		y /= mag;
		z /= mag;
		GLfloat one_minus_cos = 1.0f - cos_angle;
		multiply(matrix, (GLfloat[][4]){
				(one_minus_cos * x * x) + cos_angle,
				(one_minus_cos * x * y) - z * sin_angle,
				(one_minus_cos * z * x) + y * sin_angle,
				0.0f,
				(one_minus_cos * x * y) + z * sin_angle,
				(one_minus_cos * y * y) + cos_angle,
				(one_minus_cos * y * z) - x * sin_angle,
				0.0f,
				(one_minus_cos * z * x) - y * sin_angle,
				(one_minus_cos * y * z) + x * sin_angle,
				(one_minus_cos * z * z) + cos_angle,
				0.0f,
				0.0f,
				0.0f,
				0.0f,
				1.0f
				},
				matrix);
	}
}

void pv_draw(unsigned int width, unsigned int height)
{
	GLfloat perspective[4][4];
	identity(perspective);
	GLfloat near_z = 1.0f;
	GLfloat far_z = 20.0f;
	GLfloat delta_y = tanf(pi / 6.0f) * 2;
	GLfloat delta_z = far_z - near_z;
	multiply(perspective, (GLfloat[][4]){
			2.0f * near_z / delta_y / (GLfloat)width * (GLfloat)height, 0.0f, 0.0f, 0.0f,
			0.0f, 2.0f * near_z / delta_y, 0.0f, 0.0f,
			0.0f, 0.0f, -(near_z + far_z) / delta_z, -1.0f,
			0.0f, 0.0f, -2.0f * near_z * far_z / delta_z, 0.0f
			},
			perspective);
	GLfloat model_view[4][4];
	identity(model_view);
	translate(model_view, 0.0, 0.0, -2.7);
	rotate(model_view, -15.0, 1.0, 0.0, 0.0);
	rotate(model_view, model.angle, 0.0, 1.0, 0.0);
	multiply(model.matrix, model_view, perspective);
	glUniformMatrix4fv(glGetUniformLocation(model.program, "u_matrix"), 1, GL_FALSE, &model.matrix[0][0]);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, model.num_triangles * 3);
}

void pv_rotate(GLfloat angle)
{
	model.angle += angle;
	if (model.angle >= 360.0f)
		model.angle -= 360.0f;
}

void pv_quit()
{
	glDeleteTextures(model.num_textures, model.textures);
	glDeleteBuffers(1, model.buffers);
	glDeleteProgram(model.program);
}
