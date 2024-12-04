#pragma once

#include <glad/glad.h>

#ifdef __cplusplus
extern "C" {
#endif

#define bool int
#define true 1
#define false 0

const int size_from_gl_type(GLenum type);
void upload_form_gl_type(GLint location, GLenum type, GLint elements, void* data);


// Define analogues for all GLSL types not specified by glad.
// These will match up exactly with the upload functions provided by OpenGL
// kinda nasty that i have to do this, but I would really rather not pass around
// arrays without any kind of type information.
//

typedef struct GLvec2f{
	GLfloat X;
	GLfloat Y;
} GLvec2f;

typedef struct GLvec3f {
	GLfloat X;
	GLfloat Y;
	GLfloat Z;
} GLvec3f;

typedef struct GLvec4f {
	GLfloat X;
	GLfloat Y;
	GLfloat Z;
	GLfloat W;
} GLvec4f;


/*
typedef struct GLvec2d {
	GLdouble X;
	GLdouble Y;
} GLvec2d;

typedef struct GLvec3d {
	GLdouble X;
	GLdouble Y;
	GLdouble Z;
} GLvec3d;

typedef struct GLvec4d {
	GLdouble X;
	GLdouble Y;
	GLdouble Z;
	GLdouble W;
} GLvec4d;
*/


typedef struct GLvec2i {
	GLint X;
	GLint Y;
} GLvec2i;

typedef struct GLvec3i {
	GLint X;
	GLint Y;
	GLint Z;
} GLvec3i;

typedef struct GLvec4i {
	GLint X;
	GLint Y;
	GLint Z;
	GLint W;
} GLvec4i;


typedef struct GLvec2ui {
	GLuint X;
	GLuint Y;
} GLvec2ui;

typedef struct GLvec3ui {
	GLuint X;
	GLuint Y;
	GLuint Z;
} GLvec3ui;

typedef struct GLvec4ui {
	GLuint X;
	GLuint Y;
	GLuint Z;
	GLuint W;
} GLvec4ui;


typedef struct GLvec2b {
	GLboolean X;
	GLboolean Y;
} GLvec2b;

typedef struct GLvec3b {
	GLboolean X;
	GLboolean Y;
	GLboolean Z;
} GLvec3b;

typedef struct GLvec4b {
	GLboolean X;
	GLboolean Y;
	GLboolean Z;
	GLboolean W;
} GLvec4b;


typedef struct GLmat2f {
	GLboolean transpose;

	GLfloat m0;
	GLfloat m1;

	GLfloat m2;
	GLfloat m3;

} GLmat2f;

typedef struct GLmat2x3f {
	GLboolean transpose;

	GLfloat m0;
	GLfloat m1;

	GLfloat m2;
	GLfloat m3;

	GLfloat m4;
	GLfloat m5;

} GLmat2x3f;

typedef struct GLmat2x4f {
	GLboolean transpose;

	GLfloat m0;
	GLfloat m1;

	GLfloat m2;
	GLfloat m3;

	GLfloat m4;
	GLfloat m5;

	GLfloat m6;
	GLfloat m7;

} GLmat2x4f;

typedef struct GLmat3f {
	GLboolean transpose;

	GLfloat m0;
	GLfloat m1;
	GLfloat m2;

	GLfloat m3;
	GLfloat m4;
	GLfloat m5;

	GLfloat m6;
	GLfloat m7;
	GLfloat m8;

} GLmat3f;

typedef struct GLmat3x2f {
	GLboolean transpose;

	GLfloat m0;
	GLfloat m1;
	GLfloat m2;

	GLfloat m3;
	GLfloat m4;
	GLfloat m5;

} GLmat3x2f;

typedef struct GLmat3x4f {
	GLboolean transpose;

	GLfloat m0;
	GLfloat m1;
	GLfloat m2;

	GLfloat m3;
	GLfloat m4;
	GLfloat m5;

	GLfloat m6;
	GLfloat m7;
	GLfloat m8;

	GLfloat m9;
	GLfloat m10;
	GLfloat m11;

} GLmat3x4f;

typedef struct GLmat4f {
	GLboolean transpose;

	GLfloat m0;
	GLfloat m1;
	GLfloat m2;
	GLfloat m3;

	GLfloat m4;
	GLfloat m5;
	GLfloat m6;
	GLfloat m7;
	
	GLfloat m8;
	GLfloat m9;
	GLfloat m10;
	GLfloat m11;

	GLfloat m12;
	GLfloat m13;
	GLfloat m14;
	GLfloat m15;

} GLmat4f;

typedef struct GLmat4x2f {
	GLboolean transpose;

	GLfloat m0;
	GLfloat m1;
	GLfloat m2;
	GLfloat m3;

	GLfloat m4;
	GLfloat m5;
	GLfloat m6;
	GLfloat m7;

} GLmat4x2f;

typedef struct GLmat4x3f {
	GLboolean transpose;

	GLfloat m0;
	GLfloat m1;
	GLfloat m2;
	GLfloat m3;

	GLfloat m4;
	GLfloat m5;
	GLfloat m6;
	GLfloat m7;

	GLfloat m8;
	GLfloat m9;
	GLfloat m10;
	GLfloat m11;

} GLmat4x3f;


/*
typedef struct GLmat2d {
	GLboolean transpose;

	GLdouble m0;
	GLdouble m1;

	GLdouble m2;
	GLdouble m3;

} GLmat2d;

typedef struct GLmat2x3d {
	GLboolean transpose;

	GLdouble m0;
	GLdouble m1;

	GLdouble m2;
	GLdouble m3;

	GLdouble m4;
	GLdouble m5;

} GLmat2x3d;

typedef struct GLmat2x4d {
	GLboolean transpose;

	GLdouble m0;
	GLdouble m1;

	GLdouble m2;
	GLdouble m3;

	GLdouble m4;
	GLdouble m5;

	GLdouble m6;
	GLdouble m7;

} GLmat2x4d;

typedef struct GLmat3d {
	GLboolean transpose;

	GLdouble m0;
	GLdouble m1;
	GLdouble m2;

	GLdouble m3;
	GLdouble m4;
	GLdouble m5;

	GLdouble m6;
	GLdouble m7;
	GLdouble m8;

} GLmat3d;

typedef struct GLmat3x2d {
	GLboolean transpose;

	GLdouble m0;
	GLdouble m1;
	GLdouble m2;

	GLdouble m3;
	GLdouble m4;
	GLdouble m5;

} GLmat3x2d;

typedef struct GLmat3x4d {
	GLboolean transpose;

	GLdouble m0;
	GLdouble m1;
	GLdouble m2;

	GLdouble m3;
	GLdouble m4;
	GLdouble m5;

	GLdouble m6;
	GLdouble m7;
	GLdouble m8;

	GLdouble m9;
	GLdouble m10;
	GLdouble m11;

} GLmat3x4d;

typedef struct GLmat4d {
	GLboolean transpose;

	GLdouble m0;
	GLdouble m1;
	GLdouble m2;
	GLdouble m3;

	GLdouble m4;
	GLdouble m5;
	GLdouble m6;
	GLdouble m7;

	GLdouble m8;
	GLdouble m9;
	GLdouble m10;
	GLdouble m11;

	GLdouble m12;
	GLdouble m13;
	GLdouble m14;
	GLdouble m15;

} GLmat4d;

typedef struct GLmat4x2d {
	GLboolean transpose;

	GLdouble m0;
	GLdouble m1;
	GLdouble m2;
	GLdouble m3;

	GLdouble m4;
	GLdouble m5;
	GLdouble m6;
	GLdouble m7;

} GLmat4x2d;

typedef struct GLmat4x3d {
	GLboolean transpose;

	GLdouble m0;
	GLdouble m1;
	GLdouble m2;
	GLdouble m3;

	GLdouble m4;
	GLdouble m5;
	GLdouble m6;
	GLdouble m7;

	GLdouble m8;
	GLdouble m9;
	GLdouble m10;
	GLdouble m11;

} GLmat4x3d;
*/
#ifdef __cplusplus
}
#endif