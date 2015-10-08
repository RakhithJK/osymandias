void camera_tilt (const float radians);
void camera_rotate (const int dx);
void camera_setup (const int screen_wd, const int screen_ht);
bool camera_is_tilted (void);
bool camera_is_rotated (void);
bool camera_visible_quad (const vec4f a, const vec4f b, const vec4f c, const vec4f d);
float camera_distance_squared_point (const vec4f a);
vec4f camera_distance_squared_quad (const vec4f x, const vec4f y, const vec4f z);
vec4f camera_distance_squared_quadedge (const vec4f x, const vec4f y, const vec4f z);
