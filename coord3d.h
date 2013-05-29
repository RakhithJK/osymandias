static inline float
world_x_to_lon (float x, float world_size)
{
	// xleft is xmin is -pi, xright is xmax is pi; scale accordingly:
	return (x / world_size - 0.5f) * 2.0f * M_PI;
}

static inline float
world_y_to_lat (float y, float world_size)
{
	// ybottom is ymin is -pi, ytop is ymax is pi:
	float lat = (y / world_size - 0.5f) * 2.0f * M_PI;
	return atanf(sinh(lat));
}

static inline float
tile_y_to_lat (float y, float world_size)
{
	return world_y_to_lat(world_size - y, world_size);
}

static inline void
latlon_to_xyz (float lat, float lon, float world_size, float *x, float *y, float *z)
{
	*x = cosf(lat) * sinf(lon) * world_size;
	*z = cosf(lat) * cosf(lon) * world_size;
	*y = sinf(-lat) * world_size;
}