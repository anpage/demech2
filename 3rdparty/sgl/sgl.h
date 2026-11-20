#ifndef SGL_H
#define SGL_H

// SGL.DLL, VideoLogic's PowerVR SGL (its scene graph library), in the version the PowerVR edition
// imports (sgl.def): our own declarations of the functions it calls and of the types and constants
// they take, not the SDK's header. Names, types and values follow SGL's own sgl.h (revision 1.228,
// in Imagination's release of the PowerVR Series 1 source), which is later than the edition; only
// what the game uses is declared. Every function is __cdecl. A name (a device, viewport, list,
// camera, convex, mesh, material, texture or light) is an int, negative for an error.

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

	// What sgl_use_ddraw_mode's callback is passed each frame: the DirectDraw object and the surface
	// SGL rendered to, locked, for the 2D drawing over the 3D scene (its width and height follow,
	// which the game doesn't read).
#pragma pack(push, 1)
	// SIZE 0x12
	typedef struct {
		void* pDDObject;              // 0x00
		void* p3DSurfaceObject;       // 0x04
		void* p3DSurfaceMemory;       // 0x08 — NULL when there is nothing to draw on
		unsigned short wBitsPerPixel; // 0x0c
		unsigned long dwStride;       // 0x0e
	} CALLBACK_SURFACE_PARAMS;
#pragma pack(pop)

	typedef int(__cdecl* PROC_2D_CALLBACK)(CALLBACK_SURFACE_PARAMS* p_params);

	// A cached texture, as the texture callback sees it: SGL asks the game to load the ones a render
	// needs and to unload others when the texture memory is full.
	// SIZE 0x18
	typedef struct {
		int texture_name;              // 0x00
		unsigned long user_id;         // 0x04 — what sgl_create_cached_texture was given
		sgl_bool loaded;               // 0x08
		sgl_bool needed;               // 0x0c — this render uses it
		sgl_bool used_previous_render; // 0x10
		int approx_object_size;        // 0x14 — of the largest object that uses it, in pixels
	} sgl_tex_callback_struct;

	typedef void(__cdecl*
					 sgl_tex_callback_func)(sgl_tex_callback_struct* tex_data_array, int num_used, long free_space);

	// The SDK's enumerations of what the game passes, as constants: the screen device's colour depth
	// (sgl_device_colour_types), the texture maps' types and sizes and their mip maps' generation
	// (sgl_map_types, sgl_map_sizes, sgl_mipmap_generation_options), and the faces a mesh drops
	// (sgl_cull_mode).
#define sgl_device_16bit 0
#define sgl_device_24bit 1
#define sgl_map_16bit 0
#define sgl_map_16bit_mm 1
#define sgl_map_8bit 2
#define sgl_map_trans16 3
#define sgl_map_trans16_mm 4
#define sgl_map_32x32 0
#define sgl_map_64x64 1
#define sgl_map_128x128 2
#define sgl_map_256x256 3
#define sgl_mipmap_generate_none 0
#define sgl_mipmap_generate_2x2 1
#define sgl_mipmap_generate_4x4 2
#define sgl_no_cull 0
#define sgl_cull_anticlock 1
#define sgl_cull_clockwise 2

	// Devices and viewports.
	int __cdecl sgl_create_screen_device(
		int device_number,
		int x_dimension,
		int y_dimension,
		int device_mode,
		sgl_bool double_buffer
	);
	void __cdecl sgl_delete_device(int device_name);
	int __cdecl sgl_get_device(
		int device_name,
		int* device_number,
		int* x_dimension,
		int* y_dimension,
		int* device_mode,
		sgl_bool* double_buffer
	);
	int __cdecl sgl_use_ddraw_mode(void* hWnd, PROC_2D_CALLBACK Proc2d);
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
	void __cdecl sgl_subtract_viewport(int viewport, int removed_viewport);
	void __cdecl sgl_get_errors(int* earliest_error, int* most_recent_error);

	// The scene graph: lists, transforms, instances.
	int __cdecl sgl_create_list(sgl_bool generate_name, sgl_bool preserve_state, sgl_bool separate_list);
	void __cdecl sgl_modify_list(int list_name, sgl_bool clear_list);
	void __cdecl sgl_delete_list(int list_name);
	void __cdecl sgl_attach_list(int list_name);
	void __cdecl sgl_detach_list(int list_name);
	void __cdecl sgl_set_ignore_list(int list_name, sgl_bool ignore);
	void __cdecl sgl_to_parent(void);
	void __cdecl sgl_use_instance(int list_to_instance);
	int __cdecl sgl_create_transform(sgl_bool generate_name);
	void __cdecl sgl_modify_transform(int name, sgl_bool clear_transform);
	void __cdecl sgl_translate(float x, float y, float z);
	void __cdecl sgl_rotate(sgl_vector axis, float angle);
	int __cdecl sgl_create_detail_levels(
		sgl_bool generate_name,
		sgl_vector box_corner1,
		sgl_vector box_corner2,
		int models[3],
		int change_sizes[2]
	);

	// Cameras and rendering.
	int __cdecl sgl_create_camera(float zoom_factor, float foreground, float inv_background);
	void __cdecl sgl_set_camera(int name, float zoom_factor, float foreground, float inv_background);
	void __cdecl sgl_set_background_colour(int camera_name, sgl_colour colour);
	void __cdecl sgl_set_fog(int camera_name, sgl_colour fog_colour, float fog_density);
	void __cdecl sgl_render(int viewport_or_device, int camera_or_list, sgl_bool swap_buffers);
	void __cdecl sgl_qual_fog(sgl_bool enable);
	void __cdecl sgl_qual_texturing(sgl_bool enable);
	void __cdecl sgl_qual_smooth_shading(sgl_bool enable);
	void __cdecl sgl_qual_generate_shadows(sgl_bool enable);
	void __cdecl sgl_set_cull_mode(int cull_mode);
	void __cdecl sgl_new_translucent(void);

	// Lights.
	int __cdecl sgl_create_ambient_light(sgl_bool generate_name, sgl_colour colour, sgl_bool relative);
	void __cdecl sgl_set_ambient_light(int name, sgl_colour colour, sgl_bool relative);
	int __cdecl sgl_create_parallel_light(
		sgl_bool generate_name,
		sgl_colour colour,
		sgl_vector direction,
		sgl_bool casts_shadows,
		sgl_bool smooth_highlights
	);
	void __cdecl sgl_set_parallel_light(
		int name,
		sgl_colour colour,
		sgl_vector direction,
		sgl_bool casts_shadows,
		sgl_bool smooth_highlights
	);

	// Convex objects (planes) and meshes (faces of vertices).
	int __cdecl sgl_create_convex(sgl_bool generate_name);
	void __cdecl sgl_modify_convex(int name, sgl_bool clear);
	void __cdecl sgl_add_plane(
		sgl_vector surface_point,
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
		int plane_index,
		sgl_vector surface_point,
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
	void __cdecl sgl_add_vertices(
		int num_to_add,
		sgl_vector* vertices,
		sgl_vector* vertex_normals,
		sgl_2d_vec* vertex_uvs
	);
	void __cdecl sgl_add_face(int num_face_points, int* vertex_ids);
	void __cdecl sgl_delete_face(int face);
	int __cdecl sgl_num_faces(void);

	// Materials.
	int __cdecl sgl_create_material(sgl_bool generate_name, sgl_bool param_is_local);
	void __cdecl sgl_modify_material(int name, sgl_bool clear_material);
	void __cdecl sgl_use_material_instance(int instanced_material, sgl_bool param_is_local);
	void __cdecl sgl_set_diffuse(sgl_colour colour);
	void __cdecl sgl_set_ambient(sgl_colour colour);
	void __cdecl sgl_set_glow(sgl_colour colour);
	void __cdecl sgl_set_opacity(float opacity);
	void __cdecl sgl_set_texture_map(int texture_name, sgl_bool flip_u, sgl_bool flip_v);
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
		int generate_mipmap,
		sgl_bool dither,
		const sgl_intermediate_map* pixel_data,
		const sgl_intermediate_map* filtered_maps[]
	);
	void __cdecl sgl_delete_texture(int texture_name);
	long __cdecl sgl_texture_size(sgl_intermediate_map* texture_map);
	unsigned long __cdecl sgl_get_free_texture_mem(void);
	int __cdecl sgl_create_cached_texture(long user_id);
	void __cdecl sgl_load_cached_texture(
		int name,
		int map_type,
		int map_size,
		int generate_mipmap,
		sgl_bool dither,
		const sgl_intermediate_map* pixel_data,
		const sgl_intermediate_map* filtered_maps[],
		sgl_bool overide_prepocessed_type
	);
	void __cdecl sgl_unload_cached_texture(int name);
	void __cdecl sgl_register_texture_callback(
		sgl_tex_callback_func func,
		sgl_tex_callback_struct* tex_data_array,
		int array_size
	);

#ifdef __cplusplus
}
#endif

#endif // SGL_H
