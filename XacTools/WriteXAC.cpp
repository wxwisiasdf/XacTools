#include <string.h>
#include <cassert>
#include "XAC.hpp"
#include "WriteXAC.hpp"

namespace ogl {
#define XAC_SAVE(x) \
	std::memcpy(buffer, &x, sizeof(x)); \
	buffer += sizeof(x);
#define XAC_SAVE_STR(x) \
	{	uint32_t tmp = uint32_t(x.size()); \
		std::memcpy(buffer, &tmp, sizeof(tmp)); \
		buffer += sizeof(tmp); \
		std::memcpy(buffer, x.c_str(), x.size()); \
		buffer += x.size(); }

	char* write_xac_node_hierachy(xac_context const& context, char* buffer) {
		// ---------------------------------------------
		// node hierachy
		xac_chunk_header n_chdr;
		n_chdr.ident = uint32_t(xac_chunk_type::node_hierachy);
		n_chdr.len = sizeof(xac_node_hierachy_v1_chunk_header);
		for (const auto& node : context.nodes) {
			n_chdr.len += sizeof(xac_node_hierachy_v1_node_header);
			n_chdr.len += sizeof(uint32_t) + uint32_t(node.name.size());
		}
		n_chdr.version = 1;
		XAC_SAVE(n_chdr);
		xac_node_hierachy_v1_chunk_header n_hdr;
		n_hdr.num_nodes = uint32_t(context.nodes.size());
		n_hdr.num_root_nodes = uint32_t(context.root_nodes.size());
		XAC_SAVE(n_hdr);
		int32_t node_id = 0;
		for (const auto& node : context.nodes) {
			xac_node_hierachy_v1_node_header nd_hdr;
			nd_hdr.importance_factor = 1.f;
			nd_hdr.include_in_bounds_calc = 0;
			nd_hdr.num_children = 0;
			for (const auto& node : context.nodes) {
				if (node.parent_id == node_id)
					nd_hdr.num_children++;
			}
			nd_hdr.parent_id = node.parent_id;
			nd_hdr.position = node.position;
			nd_hdr.rotation = node.rotation;
			nd_hdr.scale = node.scale;
			nd_hdr.scale_rotation = node.scale_rotation;
			nd_hdr.transform = node.transform;
			// The following 3 statments prevents the model from "detaching" from the map
			// Why? Well, why the fuck are you asking me? Do I look like I made the XAC file format?
			// Yes? - Wow, so mean, I could never ever do such horrible thing, XAC is an atrocity
			// and a mischief that should be never ever dealt with ever again in the history
			// of computing - that I would say if I didn't appreciate the beauty of the XAC file format
			// I love XAC its the best file format please use XAC, COLLADA? DAE? No shit, use XAC for fucks sake!
			nd_hdr.include_in_bounds_calc = 0x7c9101e1; // Always this
			nd_hdr.unknown[0] = 0xffffffff; // Always this
			nd_hdr.unknown[1] = 0xffffffff;
			XAC_SAVE(nd_hdr);
			XAC_SAVE_STR(node.name);
			node_id++;
		}
		assert(node_id == int32_t(n_hdr.num_nodes));
		return buffer;
	}

	char* write_xac_material_block(xac_context const& context, char* buffer) {
		// ---------------------------------------------
		// material block chunk
		xac_chunk_header mb_chdr;
		mb_chdr.ident = uint32_t(xac_chunk_type::material_block);
		mb_chdr.version = 1;
		mb_chdr.len = sizeof(xac_material_block_v1_chunk_header);
		XAC_SAVE(mb_chdr);
		xac_material_block_v1_chunk_header mb;
		mb.num_standard_materials = uint32_t(context.materials.size());
		mb.num_fx_materials = 0;
		mb.num_total_materials = mb.num_standard_materials + mb.num_fx_materials;
		XAC_SAVE(mb);
		return buffer;
	}

	char* write_xac_material(xac_context const& context, char* buffer) {
		// ---------------------------------------------
		// material chunk
		int32_t material_id = 0;
		for (const auto& mat : context.materials) {
			xac_chunk_header m_chdr;
			m_chdr.ident = uint32_t(xac_chunk_type::material_3);
			m_chdr.version = 2;
			m_chdr.len = sizeof(xac_material_v2_chunk_header) + sizeof(uint32_t) + uint32_t(mat.name.size());
			for (uint8_t j = 0; j < mat.layers.size(); j++) {
				m_chdr.len += sizeof(xac_material_layer_v2_header);
				m_chdr.len += sizeof(uint32_t) + uint32_t(mat.layers[j].texture.size());
			}
			m_chdr.len = 94;
			XAC_SAVE(m_chdr);
			xac_material_v2_chunk_header m_hdr;
			m_hdr.ambient_color = mat.ambient_color;
			m_hdr.diffuse_color = mat.diffuse_color;
			m_hdr.double_sided = mat.double_sided;
			m_hdr.emissive_color = mat.emissive_color;
			m_hdr.ior = mat.ior;
			m_hdr.num_layers = uint8_t(mat.layers.size());
			m_hdr.opacity = mat.opacity;
			m_hdr.shine = mat.shine;
			m_hdr.shine_strength = mat.shine_strength;
			m_hdr.specular_color = mat.specular_color;
			m_hdr.wireframe = mat.wireframe;
			XAC_SAVE(m_hdr);
			XAC_SAVE_STR(mat.name);
			for (uint8_t j = 0; j < mat.layers.size(); j++) {
				auto const& layer = mat.layers[j];
				xac_material_layer_v2_header l_hdr;
				l_hdr.amount = layer.amount;
				l_hdr.map_type = uint8_t(layer.map_type);
				l_hdr.material_id = material_id;
				l_hdr.rotation = layer.rotation;
				l_hdr.u_offset = layer.u_offset;
				l_hdr.v_offset = layer.v_offset;
				l_hdr.u_tiling = layer.u_tiling;
				l_hdr.v_tiling = layer.v_tiling;
				XAC_SAVE(l_hdr);
				XAC_SAVE_STR(layer.texture);
			}
			material_id++;
		}
		return buffer;
	}

	char* write_xac_single_mesh(xac_context const& context, char* buffer, int32_t node_id, int32_t mesh_id) {
		auto const& node = context.nodes[node_id];
		auto const& mesh = node.meshes[mesh_id];
		// ---------------------------------------------
		// mesh
		uint32_t num_vertices = 0;
		uint32_t num_indices = 0;
		for (const auto& submesh : mesh.submeshes) {
			num_vertices += uint32_t(submesh.num_vertices);
			num_indices += uint32_t(submesh.indices.size());
		}
		//
		xac_chunk_header m_chdr;
		m_chdr.ident = uint32_t(xac_chunk_type::mesh);
		m_chdr.len = sizeof(xac_mesh_v1_chunk_header);
		uint32_t num_attribute_layers = 0;
		if (!mesh.vertices.empty()) {
			m_chdr.len += sizeof(xac_vertex_block_v1_header);
			m_chdr.len += sizeof(mesh.vertices[0]) * num_vertices;
			num_attribute_layers++;
		}
		if (!mesh.normals.empty()) {
			m_chdr.len += sizeof(xac_vertex_block_v1_header);
			m_chdr.len += sizeof(mesh.normals[0]) * num_vertices;
			num_attribute_layers++;
		}
		if (!mesh.texcoords.empty()) {
			m_chdr.len += sizeof(xac_vertex_block_v1_header);
			m_chdr.len += sizeof(mesh.texcoords[0]) * num_vertices;
			num_attribute_layers++;
		}
		if (!mesh.weights.empty()) {
			m_chdr.len += sizeof(xac_vertex_block_v1_header);
			m_chdr.len += sizeof(mesh.weights[0]) * num_vertices;
			num_attribute_layers++;
		}
		if (!mesh.influence_indices.empty()) {
			m_chdr.len += sizeof(xac_vertex_block_v1_header);
			m_chdr.len += sizeof(mesh.influence_indices[0]) * num_vertices;
			num_attribute_layers++;
		}
		for (const auto& sub : mesh.submeshes) {
			m_chdr.len += sizeof(xac_submesh_v1_header);
			m_chdr.len += sizeof(uint32_t) * uint32_t(sub.indices.size());
			m_chdr.len += sizeof(uint32_t) * uint32_t(sub.bone_ids.size());
		}
		if (node_id == int32_t(context.nodes.size() - 1) && mesh.influences.empty()) {
			// The XAC parser will "eat" a random piece of chunk data, because it can't stop eating garbage
			// so here we go, take this garbage you silly goose.
			// Basically, an ident of 0, means an EOF
			m_chdr.len += 4;
		}
		m_chdr.version = 1;
		XAC_SAVE(m_chdr);
		xac_mesh_v1_chunk_header m_hdr;
		m_hdr.node_id = node_id;
		m_hdr.num_indices = num_indices;
		m_hdr.num_influence_ranges = uint32_t(mesh.influence_starts.size());
		m_hdr.num_sub_meshes = uint32_t(mesh.submeshes.size());
		m_hdr.num_vertices = num_vertices;
		m_hdr.num_attribute_layers = num_attribute_layers;
		m_hdr.is_collision_mesh = (node.collision_mesh == mesh_id) ? 1 : 0;
		XAC_SAVE(m_hdr);
		// vertex blocks
		// Order:
		// 5, 0, 1, 3, 2
		if (!mesh.influence_indices.empty()) {
			xac_vertex_block_v1_header vbh;
			vbh.ident = uint32_t(xac_vertex_block_v1_type::influence_indices);
			vbh.size = sizeof(mesh.influence_indices[0]);
			XAC_SAVE(vbh);
			for (uint32_t i = 0; i < num_vertices; i++) {
				uint32_t const v = (size_t(i) >= mesh.influence_indices.size() ? uint32_t{} : mesh.influence_indices[i]);
				XAC_SAVE(v);
			}
		}
		if (!mesh.vertices.empty()) {
			xac_vertex_block_v1_header vbh;
			vbh.ident = uint32_t(xac_vertex_block_v1_type::vertex);
			vbh.size = sizeof(mesh.vertices[0]);
			vbh.keep_original = 1;
			XAC_SAVE(vbh);
			for (uint32_t i = 0; i < num_vertices; i++) {
				xac_vector3f v = (size_t(i) >= mesh.vertices.size() ? xac_vector3f{} : mesh.vertices[i]);
				XAC_SAVE(v);
			}
		}
		if (!mesh.normals.empty()) {
			xac_vertex_block_v1_header vbh;
			vbh.ident = uint32_t(xac_vertex_block_v1_type::normal);
			vbh.size = sizeof(mesh.normals[0]);
			vbh.keep_original = 1;
			XAC_SAVE(vbh);
			for (uint32_t i = 0; i < num_vertices; i++) {
				xac_vector3f const v = (size_t(i) >= mesh.normals.size() ? xac_vector3f{} : mesh.normals[i]);
				XAC_SAVE(v);
			}
		}
		if (!mesh.texcoords.empty()) {
			xac_vertex_block_v1_header vbh;
			vbh.ident = uint32_t(xac_vertex_block_v1_type::texcoord);
			vbh.size = sizeof(mesh.texcoords[0]);
			XAC_SAVE(vbh);
			for (uint32_t i = 0; i < num_vertices; i++) {
				xac_vector2f const v = (size_t(i) >= mesh.texcoords.size() ? xac_vector2f{} : mesh.texcoords[i]);
				XAC_SAVE(v);
			}
		}
		if (!mesh.weights.empty()) {
			xac_vertex_block_v1_header vbh;
			vbh.ident = uint32_t(xac_vertex_block_v1_type::weight);
			vbh.size = sizeof(mesh.weights[0]);
			vbh.keep_original = 1;
			XAC_SAVE(vbh);
			for (uint32_t i = 0; i < num_vertices; i++) {
				xac_vector4f const v = (size_t(i) >= mesh.weights.size() ? xac_vector4f{} : mesh.weights[i]);
				XAC_SAVE(v);
			}
		}
		// sub meshes
		for (const auto& sub : mesh.submeshes) {
			xac_submesh_v1_header sb_hdr;
			sb_hdr.material_id = sub.material_id;
			sb_hdr.num_vertices = sub.num_vertices;
			sb_hdr.num_indices = uint32_t(sub.indices.size());
			sb_hdr.num_bones = uint32_t(sub.bone_ids.size());
			XAC_SAVE(sb_hdr);
			for (uint32_t i = 0; i < sb_hdr.num_indices; i++) {
				uint32_t index = sub.indices[i];
				XAC_SAVE(index);
			}
			for (uint32_t i = 0; i < sb_hdr.num_bones; i++) {
				uint32_t bone_id = sub.bone_ids[i];
				XAC_SAVE(bone_id);
			}
		}
		return buffer;
	}

	char* write_xac_skinning_and_meshes(xac_context const& context, char* buffer) {
		int32_t node_id = 0;
		for (const auto& node : context.nodes) {
			if (node.visual_mesh >= 0) {
				auto& mesh = node.meshes[node.visual_mesh];
				buffer = write_xac_single_mesh(context, buffer, node_id, node.visual_mesh);
				if (!mesh.influences.empty()) {
					xac_chunk_header s_chdr;
					s_chdr.ident = uint32_t(xac_chunk_type::skinning);
					s_chdr.version = 3;
					s_chdr.len = sizeof(xac_skinning_v3_chunk_header);
					s_chdr.len += sizeof(xac_skinning_v3_influence_entry) * uint32_t(mesh.influences.size());
					s_chdr.len += sizeof(xac_skinning_v3_influence_range) * uint32_t(mesh.influence_starts.size());
					if (node_id == int32_t(context.nodes.size() - 1))
						s_chdr.len += 4;
					XAC_SAVE(s_chdr);
					//
					xac_skinning_v3_chunk_header s_hdr;
					s_hdr.num_local_bones = 0;
					s_hdr.node_id = node_id;
					s_hdr.is_for_collision_mesh = 0;
					s_hdr.num_influences = uint32_t(mesh.influences.size());
					XAC_SAVE(s_hdr);
					for (uint32_t i = 0; i < s_hdr.num_influences; i++) {
						xac_skinning_v3_influence_entry influence;
						influence.weight = mesh.influences[i].weight;
						influence.bone_id = mesh.influences[i].bone_id;
						XAC_SAVE(influence);
					}
					for (uint32_t i = 0; i < uint32_t(mesh.influence_starts.size()); i++) {
						xac_skinning_v3_influence_range range;
						range.first_influence_index = mesh.influence_starts[i];
						range.num_influences = mesh.influence_counts[i];
						XAC_SAVE(range);
					}
				}
			}
			if (node.collision_mesh >= 0) {
				auto& mesh = node.meshes[node.collision_mesh];
				buffer = write_xac_single_mesh(context, buffer, node_id, node.collision_mesh);
				if (!mesh.influences.empty()) {
					xac_chunk_header s_chdr;
					s_chdr.ident = uint32_t(xac_chunk_type::skinning);
					s_chdr.version = 3;
					s_chdr.len = sizeof(xac_skinning_v3_chunk_header);
					s_chdr.len += sizeof(xac_skinning_v3_influence_entry) * uint32_t(mesh.influences.size());
					s_chdr.len += sizeof(xac_skinning_v3_influence_range) * uint32_t(mesh.influence_starts.size());
					if (node_id == int32_t(context.nodes.size() - 1))
						s_chdr.len += 4;
					XAC_SAVE(s_chdr);
					//
					xac_skinning_v3_chunk_header s_hdr;
					s_hdr.num_local_bones = 0;
					s_hdr.node_id = node_id;
					s_hdr.is_for_collision_mesh = 1;
					s_hdr.num_influences = uint32_t(mesh.influences.size());
					XAC_SAVE(s_hdr);
					for (uint32_t i = 0; i < s_hdr.num_influences; i++) {
						xac_skinning_v3_influence_entry influence;
						influence.weight = mesh.influences[i].weight;
						influence.bone_id = mesh.influences[i].bone_id;
						XAC_SAVE(influence);
					}
					for (uint32_t i = 0; i < uint32_t(mesh.influence_starts.size()); i++) {
						xac_skinning_v3_influence_range range;
						range.first_influence_index = mesh.influence_starts[i];
						range.num_influences = mesh.influence_counts[i];
						XAC_SAVE(range);
					}
				}
			}
			node_id++;
		}
		return buffer;
	}

	char* write_xac(xac_context const& context, char* buffer) {
		xac_header hdr;
		hdr.ident[0] = 'X';
		hdr.ident[1] = 'A';
		hdr.ident[2] = 'C';
		hdr.ident[3] = ' ';
		hdr.major_version = 1;
		hdr.minor_version = 0;
		hdr.big_endian = 0;
		hdr.multiply_order = 0;
		XAC_SAVE(hdr);
		buffer = write_xac_node_hierachy(context, buffer);
		buffer = write_xac_material_block(context, buffer);
		buffer = write_xac_material(context, buffer);
		buffer = write_xac_skinning_and_meshes(context, buffer);
		return buffer;
	}
#undef XAC_SAVE_STR
#undef XAC_SAVE
}
