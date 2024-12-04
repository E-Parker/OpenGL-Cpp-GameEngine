
#include "gl_types.h"
#include <stdbool.h>

const int size_from_gl_type(const GLenum Type) {
    // I would like to not have a massive switch case but there isn't really a better way.
    // The API just does not have functionality for this since GLfloat and GLint are always assumed to be 
    // exactly the same as C / C++ standards. 
    switch (Type) {
    case GL_FLOAT: return sizeof(GLfloat);
    case GL_FLOAT_VEC2: return sizeof(GLvec2f);
    case GL_FLOAT_VEC3: return sizeof(GLvec3f);
    case GL_FLOAT_VEC4: return sizeof(GLvec4f);
    //case GL_DOUBLE: return sizeof(GLdouble);
    //case GL_DOUBLE_VEC2: return sizeof(GLvec2d);
    //case GL_DOUBLE_VEC3: return sizeof(GLvec3d);
    //case GL_DOUBLE_VEC4: return sizeof(GLvec4d);
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
    //case GL_DOUBLE_MAT2: return sizeof(GLmat2d);
    //case GL_DOUBLE_MAT2x3: return sizeof(GLmat2x3d);
    //case GL_DOUBLE_MAT2x4: return sizeof(GLmat2x4d);
    //case GL_DOUBLE_MAT3: return sizeof(GLmat3d);
    //case GL_DOUBLE_MAT3x2: return sizeof(GLmat3x2d);
    //case GL_DOUBLE_MAT3x4: return sizeof(GLmat3x4d);
    //case GL_DOUBLE_MAT4: return sizeof(GLmat4d);
    //case GL_DOUBLE_MAT4x2: return sizeof(GLmat4x2d);
    //case GL_DOUBLE_MAT4x3: return sizeof(GLmat4x3d);
    default: return 0;
    }
}

void upload_form_gl_type(GLint location, GLenum type, GLint elements, void* data) {
    switch (type) {
    case GL_FLOAT: glUniform1f(location, *((GLfloat*)data)); break;
    case GL_FLOAT_VEC2: glUniform2f(location, ((GLvec2f*)data)->X, ((GLvec2f*)data)->Y); break;
    case GL_FLOAT_VEC3: glUniform3f(location, ((GLvec3f*)data)->X, ((GLvec3f*)data)->Y, ((GLvec3f*)data)->Z); break;
    case GL_FLOAT_VEC4: glUniform4f(location, ((GLvec4f*)data)->X, ((GLvec4f*)data)->Y, ((GLvec4f*)data)->Z, ((GLvec4f*)data)->W); break;
    //case GL_DOUBLE: glUniform1f(location, (GLfloat)(*((GLdouble*)data))); break;
    //case GL_DOUBLE_VEC2: glUniform2f(location, (GLfloat)(((GLvec2d*)data)->X), (GLfloat)(((GLvec2d*)data)->Y)); break;
    //case GL_DOUBLE_VEC3: glUniform3f(location, (GLfloat)(((GLvec3d*)data)->X), (GLfloat)(((GLvec3d*)data)->Y), (GLfloat)(((GLvec3d*)data)->Z)); break;
    //case GL_DOUBLE_VEC4: glUniform4f(location, (GLfloat)(((GLvec4d*)data)->X), (GLfloat)(((GLvec4d*)data)->Y), (GLfloat)(((GLvec4d*)data)->Z), (GLfloat)(((GLvec4d*)data)->W)); break;
    case GL_INT: glUniform1i(location, *((GLint*)data)); break;
    case GL_INT_VEC2: glUniform2i(location, ((GLvec2i*)data)->X, ((GLvec2i*)data)->Y); break;
    case GL_INT_VEC3: glUniform3i(location, ((GLvec3i*)data)->X, ((GLvec3i*)data)->Y, ((GLvec3i*)data)->Z); break;
    case GL_INT_VEC4: glUniform4i(location, ((GLvec4i*)data)->X, ((GLvec4i*)data)->Y, ((GLvec4i*)data)->Z, ((GLvec4i*)data)->W); break;
    case GL_UNSIGNED_INT: glUniform1i(location, *((GLuint*)data)); break;
    case GL_UNSIGNED_INT_VEC2: glUniform2i(location, ((GLvec2ui*)data)->X, ((GLvec2ui*)data)->Y); break;
    case GL_UNSIGNED_INT_VEC3: glUniform3i(location, ((GLvec3ui*)data)->X, ((GLvec3ui*)data)->Y, ((GLvec3ui*)data)->Z); break;
    case GL_UNSIGNED_INT_VEC4: glUniform4i(location, ((GLvec4ui*)data)->X, ((GLvec4ui*)data)->Y, ((GLvec4ui*)data)->Z, ((GLvec4ui*)data)->W); break;
    case GL_BOOL: glUniform1i(location, *((GLint*)data)); break;
    case GL_BOOL_VEC2: glUniform2i(location, ((GLvec2b*)data)->X, ((GLvec2b*)data)->Y); break;
    case GL_BOOL_VEC3: glUniform3i(location, ((GLvec3b*)data)->X, ((GLvec3b*)data)->Y, ((GLvec3b*)data)->Z); break;
    case GL_BOOL_VEC4: glUniform4i(location, ((GLvec4b*)data)->X, ((GLvec4b*)data)->Y, ((GLvec4b*)data)->Z, ((GLvec4b*)data)->W); break;
    case GL_FLOAT_MAT2: glUniformMatrix2fv(location, elements, *((bool*)data), (GLfloat*)(((bool*)data) + 1)); break;
    case GL_FLOAT_MAT2x3: glUniformMatrix2x3fv(location, elements, *((bool*)data), (GLfloat*)(((bool*)data) + 1)); break;
    case GL_FLOAT_MAT2x4: glUniformMatrix2x4fv(location, elements, *((bool*)data), (GLfloat*)(((bool*)data) + 1)); break;
    case GL_FLOAT_MAT3: glUniformMatrix3fv(location, elements, *((bool*)data), (GLfloat*)(((bool*)data) + 1)); break;
    case GL_FLOAT_MAT3x2: glUniformMatrix3x2fv(location, elements, *((bool*)data), (GLfloat*)(((bool*)data) + 1)); break;
    case GL_FLOAT_MAT3x4: glUniformMatrix3x4fv(location, elements, *((bool*)data), (GLfloat*)(((bool*)data) + 1)); break;
    case GL_FLOAT_MAT4: glUniformMatrix4fv(location, elements, *((bool*)data), (GLfloat*)(((bool*)data) + 1)); break;
    case GL_FLOAT_MAT4x2:glUniformMatrix4x2fv(location, elements, *((bool*)data), (GLfloat*)(((bool*)data) + 1)); break;
    case GL_FLOAT_MAT4x3: glUniformMatrix4x3fv(location, elements, *((bool*)data), (GLfloat*)(((bool*)data) + 1)); break;
    }
}