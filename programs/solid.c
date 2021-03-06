#include <stdbool.h>
#include <GL/gl.h>

#include "../inlinebin.h"
#include "../programs.h"
#include "solid.h"

enum	{ MATRIX
	, COLOR
	, VERTEX
	} ;

static struct input inputs[] =
	{ [MATRIX]	= { .name = "matrix", .type = TYPE_UNIFORM }
	, [COLOR]	= { .name = "color",  .type = TYPE_ATTRIBUTE }
	, [VERTEX]	= { .name = "vertex", .type = TYPE_ATTRIBUTE }
	,		  { .name = NULL }
	} ;

static struct program program =
	{ .name     = "solid"
	, .fragment = { .src = SHADER_SOLID_FRAGMENT }
	, .vertex   = { .src = SHADER_SOLID_VERTEX }
	, .inputs   = inputs
	} ;

struct program *
program_solid (void)
{
	return &program;
}

GLint
program_solid_loc_color (void)
{
	return inputs[COLOR].loc;
}

GLint
program_solid_loc_vertex (void)
{
	return inputs[VERTEX].loc;
}

void
program_solid_use (struct program_solid *values)
{
	glUseProgram(program.id);
	glUniformMatrix4fv(inputs[MATRIX].loc, 1, GL_FALSE, values->matrix);
}
