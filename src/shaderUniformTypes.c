
#include "shaderUniformTypes.h"

const int size_from_gl_type(const GLenum Type) {
    // I would like to not have a massive switch case but there isn't really a better way.
    // The API just does not have functionality for this since GLfloat and GLint are always assumed to be 
    // exactly the same as C / C++ standards. 
    switch (Type) {
    case GL_FLOAT: return sizeof(GLfloat);
    case GL_FLOAT_VEC2: return sizeof(GLvec2f);
    case GL_FLOAT_VEC3: return sizeof(GLvec3f);
    case GL_FLOAT_VEC4: return sizeof(GLvec4f);
    case GL_DOUBLE: return sizeof(GLdouble);
    case GL_DOUBLE_VEC2: return sizeof(GLvec2d);
    case GL_DOUBLE_VEC3: return sizeof(GLvec3d);
    case GL_DOUBLE_VEC4: return sizeof(GLvec4d);
    case GL_INT: return sizeof(GLint);
    case GL_INT_VEC2: return sizeof(GLvec2i);
    case GL_INT_VEC3: return sizeof(GLvec3i);
    case GL_INT_VEC4: return sizeof(GLvec4i);
    case GL_UNSIGNED_INT: return sizeof(GLuint);
    case GL_UNSIGNED_INT_VEC2: return sizeof(GLvec2ui);
    case GL_UNSIGNED_INT_VEC3: return sizeof(GLvec3ui);
    case GL_UNSIGNED_INT_VEC4: return sizeof(GLvec4ui);
    case GL_BOOL: return sizeof(GLboolean);
    case GL_BOOL_VEC2: return sizeof(GLvec2b);
    case GL_BOOL_VEC3: return sizeof(GLvec3b);
    case GL_BOOL_VEC4: return sizeof(GLvec4b);
    case GL_FLOAT_MAT2: return sizeof(GLmat2f);
    case GL_FLOAT_MAT2x3: return sizeof(GLmat2x3f);
    case GL_FLOAT_MAT2x4: return sizeof(GLmat2x4f);
    case GL_FLOAT_MAT3: return sizeof(GLmat3f);
    case GL_FLOAT_MAT3x2: return sizeof(GLmat3x2f);
    case GL_FLOAT_MAT3x4: return sizeof(GLmat3x4f);
    case GL_FLOAT_MAT4: return sizeof(GLmat4f);
    case GL_FLOAT_MAT4x2: return sizeof(GLmat4x2f);
    case GL_FLOAT_MAT4x3: return sizeof(GLmat4x3f);
    case GL_DOUBLE_MAT2: return sizeof(GLmat2d);
    case GL_DOUBLE_MAT2x3: return sizeof(GLmat2x3d);
    case GL_DOUBLE_MAT2x4: return sizeof(GLmat2x4d);
    case GL_DOUBLE_MAT3: return sizeof(GLmat3d);
    case GL_DOUBLE_MAT3x2: return sizeof(GLmat3x2d);
    case GL_DOUBLE_MAT3x4: return sizeof(GLmat3x4d);
    case GL_DOUBLE_MAT4: return sizeof(GLmat4d);
    case GL_DOUBLE_MAT4x2: return sizeof(GLmat4x2d);
    case GL_DOUBLE_MAT4x3: return sizeof(GLmat4x3d);
    default: return 0;
    }
}