#include "XAC.hpp"
#include <stdio.h>
#include <string.h>
#include <string>

void read_mtl_file(ogl::xac_context& context) {
	std::string input_name = context.filename + ".mtl";
	FILE* fp = NULL;
	fopen_s(&fp, input_name.c_str(), "rt");
	if (fp != NULL) {
#ifdef XAC_DEBUG
		std::printf("Material file readed for opened!\n");
#endif
		char linebuf[512];
		while (fgets(linebuf, sizeof(linebuf), fp) != NULL) {
			if (linebuf[0] == 'm' && linebuf[1] == 'a' && linebuf[2] == 'p' && linebuf[3] == '_') {
				if (linebuf[4] == 'K' && linebuf[5] == 'd') {
					char* s = strstr(linebuf, ".dds");
					if (s != NULL) {
						*s = '\0';
						char* start = linebuf + strlen(linebuf);
						while (start != linebuf && *start != '\\')
							start--;
						if (start != linebuf)
							start++;

						ogl::xac_pp_actor_material_layer layer;
						layer.texture = std::string(start);
						layer.map_type = ogl::xac_pp_material_map_type::diffuse;
						layer.material_id = int16_t(context.materials.size() - 1);
						context.materials.back().layers.push_back(layer);
#ifdef XAC_DEBUG
						std::printf("New diffuse layer: '%s'\n", start);
#endif
					}
				}
				else if (linebuf[4] == 'K' && linebuf[5] == 's') {
					char* s = strstr(linebuf, ".dds");
					if (s != NULL) {
						*s = '\0';
						char* start = linebuf + strlen(linebuf);
						while (start != linebuf && *start != '\\')
							start--;
						if (start != linebuf)
							start++;

						ogl::xac_pp_actor_material_layer layer;
						layer.texture = std::string(start);
						layer.map_type = ogl::xac_pp_material_map_type::specular;
						layer.material_id = int16_t(context.materials.size() - 1);
						context.materials.back().layers.push_back(layer);
#ifdef XAC_DEBUG
						std::printf("New specular layer: '%s'\n", start);
#endif
					}
				}
			}
			else if (linebuf[0] == 'n' && linebuf[1] == 'e' && linebuf[2] == 'w' && linebuf[3] == 'm' && linebuf[4] == 't' && linebuf[5] == 'l') {
				char* name = linebuf + 6;
				while (*name == ' ' && *name != '\0') name++;
				if (strchr(name, '\n') != NULL)
					*strchr(name, '\n') = '\0';

				ogl::xac_pp_actor_material mat;
				mat.name = std::string(name);
				mat.opacity = 1.f;
				mat.ior = 1.f;
				mat.diffuse_color = ogl::xac_color_rgba{ 1.f, 1.f, 1.f };
				context.materials.push_back(mat);
#ifdef XAC_DEBUG
				std::printf("New MATERIAL!: '%s'\n", name);
#endif
			}
			else {
				// do nothing
			}
		}
		fclose(fp);
	}
}

void read_obj_file(ogl::xac_context& context) {
	read_mtl_file(context);

	std::string input_name = context.filename + ".obj";
	FILE* fp = NULL;
	fopen_s(&fp, input_name.c_str(), "rt");
	if (fp != NULL) {
		std::printf("Wavefront object readed for opened!\n");

		char linebuf[512];
		std::vector<ogl::xac_vector3f> vertices;
		std::vector<ogl::xac_vector3f> normals;
		std::vector<ogl::xac_vector2f> texcoords;

		vertices.push_back(ogl::xac_vector3f{});
		normals.push_back(ogl::xac_vector3f{});
		texcoords.push_back(ogl::xac_vector2f{});

		uint32_t vertex_offset = 0;
		uint32_t normal_offset = 0;
		uint32_t texcoord_offset = 0;
		int32_t curr_material_id = -1;
		while (fgets(linebuf, sizeof(linebuf), fp) != NULL) {
			if (linebuf[0] == 'v' && linebuf[1] == 't') {
				ogl::xac_vector2f v;
				(void)sscanf_s(linebuf + 2, " %f %f", &v.x, &v.y);
				v.y = -v.y;
				texcoords.push_back(v);
			}
			else if (linebuf[0] == 'v' && linebuf[1] == 'n') {
				ogl::xac_vector3f v;
				(void)sscanf_s(linebuf + 2, " %f %f %f", &v.x, &v.y, &v.z);
				v.y = -v.y;
				normals.push_back(v);
			}
			else if (linebuf[0] == 'v') {
				ogl::xac_vector3f v;
				(void)sscanf_s(linebuf + 1, " %f %f %f", &v.x, &v.y, &v.z);
				vertices.push_back(v);
			}
			else if (linebuf[0] == 'u' && linebuf[1] == 's' && linebuf[2] == 'e' && linebuf[3] == 'm' && linebuf[4] == 't' && linebuf[5] == 'l') {
				char* name = linebuf + 6;
				while (*name == ' ' && *name != '\0') name++;
				if (strchr(name, '\n') != NULL)
					*strchr(name, '\n') = '\0';
				curr_material_id = -1;
#ifdef XAC_DEBUG
				std::printf("Let's SEARCH this MATERIAL!: '%s'\n", name);
#endif
				for (auto const& mat : context.materials) {
					if (std::string(name) == mat.name) {
						curr_material_id++;
#ifdef XAC_DEBUG
						std::printf("Found material!!!: '%s' (#%i)\n", name, curr_material_id);
#endif
						break;
					}
					curr_material_id++;
				}
			}
			else if (linebuf[0] == 'o') {
				char* name = linebuf + 1;
				while (*name == ' ' && *name != '\0') name++;
				if (strchr(name, '\n') != NULL)
					*strchr(name, '\n') = '\0';

				ogl::xac_pp_actor_node node; // the node
				node.name = std::string(name);
				node.scale.x = 1.f;
				node.scale.y = 1.f;
				node.scale.z = 1.f;
				context.nodes.push_back(node);
				context.root_nodes.push_back(node);
#ifdef XAC_DEBUG
				std::printf("%s : [Total] Verts=%zu,Normals=%zu,Texcoords=%zu\n", node.name.c_str(), vertices.size(), normals.size(), texcoords.size());
#endif
				//vertices.clear();
				//normals.clear();
				//texcoords.clear();
			}
			else if (linebuf[0] == 'f') {
				int indices[9] = { 0 };
				(void)sscanf_s(linebuf + 1, " %i/%i/%i %i/%i/%i %i/%i/%i", &indices[0], &indices[1], &indices[2], &indices[3], &indices[4], &indices[5], &indices[6], &indices[7], &indices[8]);
				// make coordinates into relative ones!?
				//for (uint32_t i = 0; i < 3; i++) {
				//	indices[i * 3 + 0] -= vertex_offset + 1; // (and remove the 1-indexing!)
				//	indices[i * 3 + 1] -= normal_offset + 1;
				//	indices[i * 3 + 2] -= texcoord_offset + 1;
				//}
				bool is_dummy = false;
				for (uint32_t i = 0; i < 3 * 3; i++)
					is_dummy = is_dummy || (indices[i] <= 0);
				if (!is_dummy) {
					if (context.nodes.empty()) {
						ogl::xac_pp_actor_node node; // the node
						node.name = std::string("ImplicitDummyNode");
						node.scale.x = 1.f;
						node.scale.y = 1.f;
						node.scale.z = 1.f;
						context.nodes.push_back(node);
						context.root_nodes.push_back(node);
					}
					if (context.nodes.back().meshes.empty()) {
						ogl::xac_pp_actor_mesh mesh; // containing the mesh
						mesh.vertices = vertices;
						mesh.normals = normals;
						mesh.texcoords = texcoords;
						ogl::xac_pp_actor_submesh sub; // containing the submesh
						sub.num_vertices = uint32_t(mesh.vertices.size());
						sub.material_id = curr_material_id;
						mesh.submeshes.push_back(sub);

						//for (uint32_t i = 0; i < mesh.vertices.size(); i++) {
						//}

						mesh.weights.push_back(ogl::xac_vector4f{ 0.f, 0.f, 0.f, 0.f });
						ogl::xac_pp_bone_influence influence;
						influence.bone_id = 0;
						influence.weight = 1.f;
						mesh.influences.push_back(influence);
						mesh.influence_indices.push_back(uint32_t(mesh.influences.size() - 1));
						mesh.influence_starts.push_back(uint32_t(mesh.influences.size() - 1));
						mesh.influence_counts.push_back(1);

						context.nodes.back().visual_mesh = 0; // this is the visual mesh
						context.nodes.back().meshes.push_back(mesh);
					}
					auto& mesh = context.nodes.back().meshes.back();
					auto& sub = mesh.submeshes.back();
					sub.indices.push_back(uint32_t(indices[0 * 3]) - 1);
					sub.indices.push_back(uint32_t(indices[1 * 3]) - 1);
					sub.indices.push_back(uint32_t(indices[2 * 3]) - 1);
				}
			}
			else {
				// do nothing
			}
		}

		// optimize
		for (auto& node : context.nodes) {
			for (auto& mesh : node.meshes) {
				uint32_t min_index = std::numeric_limits<uint32_t>::max();
				uint32_t max_index = std::numeric_limits<uint32_t>::min();
				for (auto const& sub : mesh.submeshes) {
					for (auto const index : sub.indices) {
						min_index = std::min(index, min_index);
						max_index = std::max(index, max_index);
					}
				}
#ifdef XAC_DEBUG
				std::printf("Deleting from '%s': %i - %i\n", node.name.c_str(), min_index, max_index);
				std::printf("A-PoolSizes: v=%zu,n=%zu,t=%zu\n", mesh.vertices.size(), mesh.normals.size(), mesh.texcoords.size());
#endif
				// Erase top
				mesh.vertices.erase(mesh.vertices.begin() + max_index + 1, mesh.vertices.end());
				mesh.normals.erase(mesh.normals.begin() + max_index + 1, mesh.normals.end());
				mesh.texcoords.erase(mesh.texcoords.begin() + max_index + 1, mesh.texcoords.end());
				// Erase bottom
				mesh.vertices.erase(mesh.vertices.begin(), mesh.vertices.begin() + min_index);
				mesh.normals.erase(mesh.normals.begin(), mesh.normals.begin() + min_index);
				mesh.texcoords.erase(mesh.texcoords.begin(), mesh.texcoords.begin() + min_index);
#ifdef XAC_DEBUG
				std::printf("B-PoolSizes: v=%zu,n=%zu,t=%zu\n", mesh.vertices.size(), mesh.normals.size(), mesh.texcoords.size());
#endif
				uint32_t vertex_offset = 0;
				for (auto& sub : mesh.submeshes) {
					sub.num_vertices = mesh.vertices.size();
					sub.vertex_offset = vertex_offset;
					for (uint32_t i = 0; i < uint32_t(sub.indices.size()); i++) {
						sub.indices[i] -= min_index;
						sub.indices[i] -= vertex_offset;
					}
					vertex_offset += sub.num_vertices;
				}
			}
		}

		fclose(fp);
	}
}
