#include "XAC.hpp"
#include "parsers.h"
#include <stdio.h>
#include <string>

void write_obj_file(ogl::xac_context& context) {
	std::string out_name = context.filename + ".obj";
	FILE* out = NULL;
	fopen_s(&out, out_name.c_str(), "wt");
	if (out != NULL) {
		fprintf(out, "mtllib %s.mtl\n", context.filename.c_str());
		for (const auto& node : context.nodes) {
			for (const auto& o : node.meshes) {
				for (uint32_t j = 0; j < o.vertices.size(); j++) {
					if (j < o.vertices.size()) {
						ogl::xac_vector3f v = o.vertices[j];
						fprintf(out, "v %f %f %f\n", v.x, -v.y, v.z);
					}
					if (j < o.normals.size()) {
						ogl::xac_vector3f v = o.normals[j];
						fprintf(out, "vn %f %f %f\n", v.x, -v.y, v.z);
					}
					if (j < o.texcoords.size()) {
						ogl::xac_vector2f v = o.texcoords[j];
						fprintf(out, "vt %f %f\n", v.x, -v.y);
					}
				}
			}
		}
		fprintf(out, "v 0.0 0.0 0.0\n");
		uint32_t vertex_index_offset = 0;
		uint32_t normal_index_offset = 0;
		uint32_t texcoord_index_offset = 0;
		for (const auto& node : context.nodes) {
			fprintf(out, "o %s\n", node.name.c_str());
			if (node.meshes.empty()) {
				fprintf(out, "f -1 -1 -1\n");
			}
			for (const auto& o : node.meshes) {
				uint32_t index = 0;
				for (const auto& sub : o.submeshes) {
					fprintf(out, "o %s%i # (%u vertices, %u offset from main)\n", node.name.c_str(), index, sub.num_vertices, sub.vertex_offset);
					ogl::xac_pp_actor_material* mat = nullptr;
					if (sub.material_id < context.materials.size()) {
						mat = &context.materials[sub.material_id];
						auto name = ogl::get_canonical_material_name(*mat);
						fprintf(out, "usemtl %s\n", name.c_str());
					}
					for (uint32_t i = 0; i < sub.indices.size(); i += 3) {
						auto vi1 = sub.indices[i + 0] + 1 + vertex_index_offset + sub.vertex_offset;
						auto vi2 = sub.indices[i + 1] + 1 + vertex_index_offset + sub.vertex_offset;
						auto vi3 = sub.indices[i + 2] + 1 + vertex_index_offset + sub.vertex_offset;
						auto ni1 = sub.indices[i + 0] + 1 + normal_index_offset + sub.vertex_offset;
						auto ni2 = sub.indices[i + 1] + 1 + normal_index_offset + sub.vertex_offset;
						auto ni3 = sub.indices[i + 2] + 1 + normal_index_offset + sub.vertex_offset;
						auto ti1 = sub.indices[i + 0] + 1 + texcoord_index_offset + sub.vertex_offset;
						auto ti2 = sub.indices[i + 1] + 1 + texcoord_index_offset + sub.vertex_offset;
						auto ti3 = sub.indices[i + 2] + 1 + texcoord_index_offset + sub.vertex_offset;
						fprintf(out, "f %i/%i/%i %i/%i/%i %i/%i/%i\n", vi1, ti1, ni1, vi2, ti2, ni2, vi3, ti3, ni3);
					}
					index++;
				}
				vertex_index_offset += uint32_t(o.vertices.size());
				normal_index_offset += uint32_t(o.normals.size());
				texcoord_index_offset += uint32_t(o.texcoords.size());
			}
		}
		fclose(out);
	}
}

void write_mtl_files(ogl::xac_context& context) {
	std::string anims_folder_path = context.base_game_path + "\\gfx\\anims";
	std::string out_name = context.filename + ".mtl";
	FILE* out = NULL;
	fopen_s(&out, out_name.c_str(), "wt");
	if (out != NULL) {
		for (const auto& mtl : context.materials) {
			std::string canon_name = ogl::get_canonical_material_name(mtl);
			fprintf(out, "newmtl %s\n", canon_name.c_str());
			fprintf(out, "Ka %f %f %f %f\n", mtl.ambient_color.r, mtl.ambient_color.g, mtl.ambient_color.b, mtl.ambient_color.a);
			fprintf(out, "Kd %f %f %f %f\n", mtl.diffuse_color.r, mtl.diffuse_color.g, mtl.diffuse_color.b, mtl.diffuse_color.a);
			fprintf(out, "Ks %f %f %f %f\n", mtl.specular_color.r, mtl.specular_color.g, mtl.specular_color.b, mtl.specular_color.a);
			fprintf(out, "d %f\n", mtl.opacity);
			fprintf(out, "Tr %f\n", 1.f - mtl.opacity);
			fprintf(out, "Ns %f\n", mtl.shine_strength);
			fprintf(out, "Ni %f\n", mtl.ior);
			std::string diff_texture = ogl::get_diffuse_texture(context, mtl);
			if (!diff_texture.empty()) {
				auto layer = ogl::get_diffuse_layer(mtl);
				fprintf(out, "map_Ka -o %f %f %s\\%s.dds\n", layer.u_offset, layer.v_offset, anims_folder_path.c_str(), diff_texture.c_str());
				fprintf(out, "map_Kd -o %f %f %s\\%s.dds\n", layer.u_offset, layer.v_offset, anims_folder_path.c_str(), diff_texture.c_str());
			}
			std::string spec_texture = ogl::get_specular_texture(context, mtl);
			if (!spec_texture.empty()) {
				auto layer = ogl::get_diffuse_layer(mtl);
				fprintf(out, "map_Ks -o %f %f %s\\%s.dds\n", layer.u_offset, layer.v_offset, anims_folder_path.c_str(), spec_texture.c_str());
			}
		}
		fclose(out);
	}
}

