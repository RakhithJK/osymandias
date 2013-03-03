#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <GL/gl.h>

#include "shaders.h"
#include "autoscroll.h"
#include "world.h"
#include "layers.h"
#include "viewport.h"

static double center_x;		// in world coordinates
static double center_y;		// in world coordinates
static unsigned int screen_wd;	// screen dimension
static unsigned int screen_ht;	// screen dimension

static int tile_top;
static int tile_left;
static int tile_right;
static int tile_bottom;
static int tile_last;

static void
viewport_screen_extents_to_world (double *world_left, double *world_bottom, double *world_right, double *world_top)
{
	viewport_screen_to_world(0, 0, world_left, world_bottom);
	viewport_screen_to_world(screen_wd, screen_ht, world_right, world_top);
}

static void
recalc_tile_extents (void)
{
	// Given center_x, center_y and world_size,
	// get the tile extents shown on screen

	double world_left, world_right, world_top, world_bottom;

	viewport_screen_extents_to_world(&world_left, &world_bottom, &world_right, &world_top);

	// Calculate border tiles. Tile origin is left top, world origin is
	// left bottom. That's why we use world_bottom for tile_top and vice
	// versa:
	tile_last = world_get_size() - 1;
	tile_left = floor(world_left);
	tile_right = ceil(world_right);
	tile_top = floor(world_bottom);
	tile_bottom = ceil(world_top);

	// Clip to world:
	if (tile_left < 0) tile_left = 0;
	if (tile_top < 0) tile_top = 0;
	if (tile_right > tile_last) tile_right = tile_last;
	if (tile_bottom > tile_last) tile_bottom = tile_last;
}

static void
center_set (const double world_x, const double world_y)
{
	// Set center to coordinates, but clip to [0, world_size]

	unsigned int world_size = world_get_size();

	center_x = (world_x < 0) ? 0.0
	         : (world_x >= world_size) ? world_size
	         : (world_x);

	center_y = (world_y < 0) ? 0.0
	         : (world_y >= world_size) ? world_size
	         : (world_y);

	recalc_tile_extents();
}

bool
viewport_init (void)
{
	center_x = center_y = (double)world_get_size() / 2.0;
	recalc_tile_extents();
	return true;
}

void
viewport_destroy (void)
{
}

void
viewport_zoom_in (const int screen_x, const int screen_y)
{
	if (world_zoom_in()) {
		center_x *= 2;
		center_y *= 2;
		layers_zoom();
	}
	// Keep same point under mouse cursor:
	int dx = screen_x - screen_wd / 2;
	int dy = screen_y - screen_ht / 2;
	viewport_scroll(-dx, -dy);
}

void
viewport_zoom_out (const int screen_x, const int screen_y)
{
	if (world_zoom_out()) {
		center_x /= 2;
		center_y /= 2;
		layers_zoom();
	}
	int dx = screen_x - screen_wd / 2;
	int dy = screen_y - screen_ht / 2;
	viewport_scroll(dx / 2, dy / 2);
}

void
viewport_scroll (const int dx, const int dy)
{
	double world_x, world_y;

	// Find out which world coordinate will be in the center of the screen
	// after the offsets have been applied, then set the center to that
	// value:
	viewport_screen_to_world(screen_wd / 2 - dx, screen_ht / 2 - dy, &world_x, &world_y);
	center_set(world_x, world_y);
}

void
viewport_center_at (const int screen_x, const int screen_y)
{
	double world_x, world_y;

	viewport_screen_to_world(screen_x, screen_y, &world_x, &world_y);
	center_set(world_x, world_y);
}

void
viewport_reshape (const unsigned int new_width, const unsigned int new_height)
{
	int dx;
	int dy;

	if (autoscroll_update(&dx, &dy)) {
		viewport_scroll(dx, dy);
	}
	screen_wd = new_width;
	screen_ht = new_height;

	recalc_tile_extents();
	viewport_render();
}

void
viewport_render (void)
{
	shaders_init();
	layers_paint();
}

void
viewport_gl_setup_screen (void)
{
	// Setup the OpenGL frustrum to map 1:1 to screen coordinates,
	// with the origin at left bottom. Handy for drawing items that
	// are statically positioned relative to the screen, such as the
	// background and the cursor.

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, screen_wd, 0, screen_ht, 0, 1);
	glViewport(0, 0, screen_wd, screen_ht);

	// Slight translation to snap line artwork to pixels:
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.375, 0.375, 0.0);

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
}

void
viewport_gl_setup_world (void)
{
	// Setup the OpenGL frustrum to map screen to world coordinates,
	// with the origin at left bottom. This is a different convention
	// from the one OSM uses for its tiles: origin at left top. But it
	// makes tile and texture handling much easier (no vertical flips).

	double halfwd = (double)screen_wd / 512.0;
	double halfht = (double)screen_ht / 512.0;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, screen_wd, screen_ht);
	glTranslatef(0.375 / 256.0, 0.375 / 256.0, 0.0);
	glOrtho(-halfwd, halfwd, -halfht, halfht, 0, 1);

	// NB: layers that use this projection need to MANUALLY offset all
	// coordinates with (-center_x, -center_y)! This is necessary because
	// OpenGL uses floats internally, and their precision is not high
	// enough for pixel-perfect placement in far corners of a giant world.
	// So we keep the ortho window close around the origin, where floats
	// are still precise enough, and apply the transformation *manually* in
	// software.
	// Yes, even glTranslated() does not work at the required precision.
	// Yes, even when used on the MODELVIEW matrix.

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
}

bool
viewport_within_world_bounds (void)
{
	double world_left, world_bottom, world_right, world_top;
	unsigned int world_size = world_get_size();

	viewport_screen_extents_to_world(&world_left, &world_bottom, &world_right, &world_top);

	// Does part of the viewport show an area outside of the world?
	if (world_left < 0) return false;
	if (world_bottom < 0) return false;
	if (world_right >= world_size) return false;
	if (world_top >= world_size) return false;
	return true;
}

unsigned int
viewport_get_wd (void)
{
	return screen_wd;
}

unsigned int
viewport_get_ht (void)
{
	return screen_ht;
}

double
viewport_get_center_x (void)
{
	return center_x;
}

double
viewport_get_center_y (void)
{
	return center_y;
}

int viewport_get_tile_top (void) { return tile_top; }
int viewport_get_tile_left (void) { return tile_left; }
int viewport_get_tile_right (void) { return tile_right; }
int viewport_get_tile_bottom (void) { return tile_bottom; }


void
viewport_screen_to_world (double sx, double sy, double *wx, double *wy)
{
	*wx = center_x + (sx - (double)screen_wd / 2.0) / 256.0;
	*wy = center_y + (sy - (double)screen_ht / 2.0) / 256.0;
}

void
viewport_world_to_screen (double wx, double wy, int *sx, int *sy)
{
	*sx = (double)screen_wd / 2.0 + (wx - center_x) * 256.0;
	*sy = (double)screen_ht / 2.0 + (wy - center_y) * 256.0;
}
