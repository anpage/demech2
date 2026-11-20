#ifndef SGL_H
#define SGL_H

// SGL.DLL, VideoLogic's PowerVR SGL (its scene graph library), in the version the PowerVR edition
// imports (sgl.def): our own declarations of the functions it calls and of the types they take, not
// the SDK's header. Names follow the SDK's SGL.H; the parameters' types come from the game's call
// sites. Every function is __cdecl. A name (a device, viewport, list, camera, convex, mesh,
// material, texture or light) is an int, negative for an error.

#ifdef __cplusplus
extern "C"
{
#endif

	typedef int sgl_bool;

	// A point or direction, a color (red, green and blue, 0 to 1), and a texture coordinate.
	typedef float sgl_vector[3];
	typedef float sgl_colour[3];
	typedef float sgl_2d_vec[2];

	// A texture in the form SGL converts it from: "IMAP", the size and the 32-bit pixels.
	// SIZE 0x10
	typedef struct sgl_intermediate_map {
		char id[4];   // 0x0
		int x_dim;    // 0x4
		int y_dim;    // 0x8
		void* pixels; // 0xc
	} sgl_intermediate_map;

	// What sgl_use_ddraw_mode's callback is passed each frame: the DirectDraw object, and the locked
	// back buffer's pixels and pitch, for the 2D drawing over the 3D scene.
#pragma pack(push, 2)
	// SIZE 0x12
	typedef struct {
		void* ddraw;  // 0x00
		long unk04;   // 0x04
		void* pixels; // 0x08 — NULL when there is nothing to draw on
		short unk0c;  // 0x0c
		long pitch;   // 0x0e
	} sgl_ddraw_frame;
#pragma pack(pop)

	typedef int(__cdecl* sgl_ddraw_callback)(sgl_ddraw_frame* frame);

	// A cached texture, as the texture callback sees it: SGL asks the game to load the ones it needs
	// and to unload others when the texture memory is full.
	// SIZE 0x18
	typedef struct {
		int texture;           // 0x00 — its name
		unsigned long user_id; // 0x04 — what sgl_create_cached_texture was given
		sgl_bool loaded;       // 0x08
		sgl_bool needed;       // 0x0c — the frame uses it
		int unk10;             // 0x10
		int priority;          // 0x14
	} sgl_texture_cache_entry;

	typedef void(__cdecl* sgl_texture_callback)(sgl_texture_cache_entry* entries, int count, unsigned int free_memory);

	// Devices and viewports.
	int __cdecl sgl_create_screen_device(int device, int x_dim, int y_dim, int mode, sgl_bool double_buffer);
	void __cdecl sgl_delete_device(int device);
	int __cdecl sgl_get_device(int device, int* number, int* x_dim, int* y_dim, int* mode, sgl_bool* double_buffer);
	int __cdecl sgl_use_ddraw_mode(void* hwnd, sgl_ddraw_callback callback);
	int __cdecl sgl_create_viewport(
		int device,
		int left,
		int top,
		int right,
		int bottom,
		float cam_left,
		float cam_top,
		float cam_right,
		float cam_bottom
	);
	int __cdecl sgl_get_viewport(
		int viewport,
		int* left,
		int* top,
		int* right,
		int* bottom,
		float* cam_left,
		float* cam_top,
		float* cam_right,
		float* cam_bottom
	);
	int __cdecl sgl_set_viewport(
		int viewport,
		int left,
		int top,
		int right,
		int bottom,
		float cam_left,
		float cam_top,
		float cam_right,
		float cam_bottom
	);
	int __cdecl sgl_subtract_viewport(int viewport, int remove);
	void __cdecl sgl_get_errors(int* earliest, int* most_recent);

	// The scene graph: lists, transforms, instances.
	int __cdecl sgl_create_list(sgl_bool generate_name, sgl_bool preserve_state, sgl_bool separate_list);
	void __cdecl sgl_modify_list(int list, sgl_bool clear);
	void __cdecl sgl_delete_list(int list);
	void __cdecl sgl_attach_list(int list);
	void __cdecl sgl_detach_list(int list);
	void __cdecl sgl_set_ignore_list(int list, sgl_bool ignore);
	void __cdecl sgl_to_parent(void);
	void __cdecl sgl_use_instance(int instance);
	int __cdecl sgl_create_transform(sgl_bool generate_name);
	void __cdecl sgl_modify_transform(int transform, sgl_bool clear);
	void __cdecl sgl_translate(float x, float y, float z);
	void __cdecl sgl_rotate(sgl_vector axis, float angle);
	int __cdecl sgl_create_detail_levels(
		sgl_bool generate_name,
		sgl_vector corner1,
		sgl_vector corner2,
		int models[3],
		int change_points[2]
	);

	// Cameras and rendering.
	int __cdecl sgl_create_camera(float zoom, float foreground, float inv_background);
	void __cdecl sgl_set_camera(int camera, float zoom, float foreground, float inv_background);
	void __cdecl sgl_set_background_colour(int camera, sgl_colour colour);
	void __cdecl sgl_set_fog(int camera, sgl_colour colour, float density);
	void __cdecl sgl_render(int camera_or_list, int camera, sgl_bool lights_changed);
	void __cdecl sgl_qual_fog(sgl_bool enable);
	void __cdecl sgl_qual_texturing(sgl_bool enable);
	void __cdecl sgl_qual_smooth_shading(sgl_bool enable);
	void __cdecl sgl_qual_generate_shadows(sgl_bool enable);
	void __cdecl sgl_set_cull_mode(int mode);
	void __cdecl sgl_new_translucent(void);

	// Lights.
	int __cdecl sgl_create_ambient_light(sgl_bool generate_name, sgl_colour colour, sgl_bool relative);
	void __cdecl sgl_set_ambient_light(int light, sgl_colour colour, sgl_bool relative);
	int __cdecl sgl_create_parallel_light(
		sgl_bool generate_name,
		sgl_colour colour,
		sgl_vector direction,
		sgl_bool casts_shadows,
		sgl_bool smooth_highlights
	);
	void __cdecl sgl_set_parallel_light(
		int light,
		sgl_colour colour,
		sgl_vector direction,
		sgl_bool casts_shadows,
		sgl_bool smooth_highlights
	);

	// Convex objects (planes) and meshes (faces of vertices).
	int __cdecl sgl_create_convex(sgl_bool generate_name);
	void __cdecl sgl_modify_convex(int convex, sgl_bool clear);
	void __cdecl sgl_add_plane(
		sgl_vector point1,
		sgl_vector point2,
		sgl_vector point3,
		sgl_bool invisible,
		sgl_vector normal1,
		sgl_vector normal2,
		sgl_vector normal3,
		sgl_2d_vec uv1,
		sgl_2d_vec uv2,
		sgl_2d_vec uv3
	);
	void __cdecl sgl_set_plane(
		int plane,
		sgl_vector point1,
		sgl_vector point2,
		sgl_vector point3,
		sgl_bool invisible,
		sgl_vector normal1,
		sgl_vector normal2,
		sgl_vector normal3,
		sgl_2d_vec uv1,
		sgl_2d_vec uv2,
		sgl_2d_vec uv3
	);
	int __cdecl sgl_create_mesh(sgl_bool generate_name);
	void __cdecl sgl_add_vertices(int count, sgl_vector* vertices, sgl_vector* normals, sgl_2d_vec* uvs);
	int __cdecl sgl_add_face(int count, int* vertex_ids);
	void __cdecl sgl_delete_face(int count);
	int __cdecl sgl_num_faces(void);

	// Materials.
	int __cdecl sgl_create_material(sgl_bool generate_name, sgl_bool param_flag);
	void __cdecl sgl_modify_material(int material, sgl_bool clear);
	void __cdecl sgl_use_material_instance(int material, sgl_bool fixed);
	void __cdecl sgl_set_diffuse(sgl_colour colour);
	void __cdecl sgl_set_ambient(sgl_colour colour);
	void __cdecl sgl_set_glow(sgl_colour colour);
	void __cdecl sgl_set_opacity(float opacity);
	void __cdecl sgl_set_texture_map(int texture, sgl_bool flip_u, sgl_bool flip_v);
	void __cdecl sgl_set_texture_effect(
		sgl_bool affect_ambient,
		sgl_bool affect_diffuse,
		sgl_bool affect_specular,
		sgl_bool affect_glow
	);

	// Textures.
	int __cdecl sgl_create_texture(
		int map_type,
		int map_size,
		sgl_bool generate_mipmap,
		sgl_bool dither,
		sgl_intermediate_map* pixels,
		sgl_intermediate_map* filtered_maps
	);
	void __cdecl sgl_delete_texture(int texture);
	unsigned long __cdecl sgl_texture_size(sgl_intermediate_map* map);
	unsigned long __cdecl sgl_get_free_texture_mem(void);
	int __cdecl sgl_create_cached_texture(unsigned long user_id);
	int __cdecl sgl_load_cached_texture(
		int texture,
		int map_type,
		int map_size,
		sgl_bool generate_mipmap,
		sgl_bool dither,
		sgl_intermediate_map* pixels,
		sgl_intermediate_map* filtered_maps,
		void* unk1c
	);
	void __cdecl sgl_unload_cached_texture(int texture);
	void __cdecl sgl_register_texture_callback(
		sgl_texture_callback callback,
		sgl_texture_cache_entry* entries,
		int count
	);

#ifdef __cplusplus
}
#endif

#endif // SGL_H
