#include "XAC.hpp"
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

void write_pscript_single_mesh(ogl::xac_context const& context, FILE* fp, int32_t node_id, int32_t mesh_id) {
	auto const& node = context.nodes[node_id];
	auto const& mesh = node.meshes[mesh_id];
	fprintf(fp, "\tmesh = {\n");
	fprintf(fp, "\t\tnodeId = %u\n", node_id);
	fprintf(fp, "\t\tisCollision = %s\n", (node.collision_mesh == mesh_id) ? "yes" : "no");
	fprintf(fp, "\t\tinfluence_indices = { ");
	for (uint32_t i = 0; i < uint32_t(mesh.vertices.size()); i++) {
		uint32_t const index = (size_t(i) >= mesh.influence_indices.size() ? uint32_t{} : mesh.influence_indices[i]);
		fprintf(fp, "%u ", index);
	}
	fprintf(fp, "}\n");
	fprintf(fp, "\t\tvertices = { ");
	//fprintf(fp, "\t\t\tkeep = yes\n");
	for (uint32_t i = 0; i < uint32_t(mesh.vertices.size()); i++) {
		auto const v = (size_t(i) >= mesh.vertices.size() ? ogl::xac_vector3f{} : mesh.vertices[i]);
		fprintf(fp, "{ %f %f %f } ", v.x, v.y, v.z);
	}
	fprintf(fp, "}\n");
	fprintf(fp, "\t\tnormals = { ");
	//fprintf(fp, "\t\t\tkeep = yes\n");
	for (uint32_t i = 0; i < uint32_t(mesh.vertices.size()); i++) {
		auto const v = (size_t(i) >= mesh.normals.size() ? ogl::xac_vector3f{} : mesh.normals[i]);
		fprintf(fp, "{ %f %f %f } ", v.x, v.y, v.z);
	}
	fprintf(fp, "}\n");
	fprintf(fp, "\t\ttexcoords = { ");
	for (uint32_t i = 0; i < uint32_t(mesh.vertices.size()); i++) {
		auto const v = (size_t(i) >= mesh.texcoords.size() ? ogl::xac_vector2f{} : mesh.texcoords[i]);
		fprintf(fp, "{ %f %f } ", v.x, v.y);
	}
	fprintf(fp, "}\n");
	fprintf(fp, "\t\tweights = { ");
	for (uint32_t i = 0; i < uint32_t(mesh.vertices.size()); i++) {
		auto const v = (size_t(i) >= mesh.weights.size() ? ogl::xac_vector4f{} : mesh.weights[i]);
		fprintf(fp, "{ %f %f %f %f } ", v.x, v.y, v.z, v.w);
	}
	fprintf(fp, "}\n");
	fprintf(fp, "\t\tinfluences = { ");
	for (uint32_t i = 0; i < uint32_t(mesh.influences.size()); i++) {
		fprintf(fp, "{ %f %i } ", mesh.influences[i].weight, mesh.influences[i].bone_id);
	}
	fprintf(fp, "}\n");
	fprintf(fp, "\t\tinfluence_ranges = { ");
	for (uint32_t i = 0; i < uint32_t(mesh.influence_starts.size()); i++) {
		fprintf(fp, "{ %u %u } ", mesh.influence_starts[i], mesh.influence_counts[i]);
	}
	fprintf(fp, "}\n");
	// sub meshes
	for (const auto& sub : mesh.submeshes) {
		fprintf(fp, "\t\tsubmesh = {\n");
		fprintf(fp, "\t\t\tmaterial = %s\n", context.materials[sub.material_id].name.c_str());
		fprintf(fp, "\t\t\tnumVertices = %u\n", sub.num_vertices);
		fprintf(fp, "\t\t\tindices = { ");
		for (uint32_t i = 0; i < uint32_t(sub.indices.size()); i++) {
			fprintf(fp, "%u ", sub.indices[i]);
		}
		fprintf(fp, "}\n");
		fprintf(fp, "\t\t\tbone_ids = { ");
		for (uint32_t i = 0; i < uint32_t(sub.bone_ids.size()); i++)
			fprintf(fp, "%u ", sub.bone_ids[i]);
		fprintf(fp, "}\n");
		fprintf(fp, "\t\t}\n");
	}
	fprintf(fp, "\t}\n");
}

void write_pscript(ogl::xac_context const& context, FILE* fp) {
	fprintf(fp, "version = 1.0\n");
	fprintf(fp, "endian = little\n");
	fprintf(fp, "multiplyOrder = 0\n");
	fprintf(fp, "maxStandardMaterials = %u\n", context.max_standard_materials);
	fprintf(fp, "maxFxMaterials = %u\n", context.max_fx_materials);
	int32_t material_id = 0;
	for (const auto& mat : context.materials) {
		fprintf(fp, "material = {\n");
		fprintf(fp, "\tname = \"%s\"\n", mat.name.c_str());
		fprintf(fp, "\tambientColor = { %f %f %f %f }\n", mat.ambient_color.r, mat.ambient_color.g, mat.ambient_color.b, mat.ambient_color.a);
		fprintf(fp, "\tdiffuseColor = { %f %f %f %f }\n", mat.diffuse_color.r, mat.diffuse_color.g, mat.diffuse_color.b, mat.diffuse_color.a);
		fprintf(fp, "\temissiveColor = { %f %f %f %f }\n", mat.emissive_color.r, mat.emissive_color.g, mat.emissive_color.b, mat.emissive_color.a);
		fprintf(fp, "\tspecularColor = { %f %f %f %f }\n", mat.specular_color.r, mat.specular_color.g, mat.specular_color.b, mat.specular_color.a);
		fprintf(fp, "\tshine = %f\n", mat.shine);
		fprintf(fp, "\tshineStrength = %f\n", mat.shine_strength);
		fprintf(fp, "\twireframe = %s\n", mat.wireframe ? "yes" : "no");
		fprintf(fp, "\tior = %f\n", mat.ior);
		fprintf(fp, "\topacity = %f\n", mat.opacity);
		for (const auto& layer : mat.layers) {
			fprintf(fp, "\tlayer = {\n");
			fprintf(fp, "\t\ttexture = \"%s\"\n", layer.texture.c_str());
			fprintf(fp, "\t\tamount = %f\n", layer.amount);
			fprintf(fp, "\t\tmapType = %u\n", uint32_t(layer.map_type));
			fprintf(fp, "\t\tmaterialId = %i\n", material_id);
			fprintf(fp, "\t\trotation = %f #radians\n", layer.rotation);
			fprintf(fp, "\t\toffset = { %f %f }\n", layer.u_offset, layer.v_offset);
			fprintf(fp, "\t\ttiling = { %f %f }\n", layer.u_tiling, layer.v_tiling);
			fprintf(fp, "\t}\n");
		}
		fprintf(fp, "}\n");
		material_id++;
	}
	int32_t node_id = 0;
	for (const auto& node : context.nodes) {
		fprintf(fp, "node = {\n");
		fprintf(fp, "\tname = \"%s\"\n", node.name.c_str());
		fprintf(fp, "\tparentId = %i\n", node.parent_id);
		fprintf(fp, "\tposition = { %f %f %f }\n", node.position.x, node.position.y, node.position.z);
		fprintf(fp, "\trotation = { %f %f %f %f }\n", node.rotation.x, node.rotation.y, node.rotation.z, node.rotation.w);
		fprintf(fp, "\tscale = { %f %f %f }\n", node.scale.x, node.scale.y, node.scale.z);
		fprintf(fp, "\tscaleRotation = { %f %f %f %f }\n", node.scale_rotation.x, node.scale_rotation.y, node.scale_rotation.z, node.scale_rotation.w);
		fprintf(fp, "\tcollisionMesh = %i\n", node.collision_mesh);
		fprintf(fp, "\tvisualMesh = %i\n", node.visual_mesh);
		fprintf(fp, "\timportanceFactor = 1.0\n");
		if (node.visual_mesh >= 0) {
			auto& mesh = node.meshes[node.visual_mesh];
			write_pscript_single_mesh(context, fp, node_id, node.visual_mesh);
		}
		if (node.collision_mesh >= 0) {
			auto& mesh = node.meshes[node.collision_mesh];
			write_pscript_single_mesh(context, fp, node_id, node.collision_mesh);
		}
		fprintf(fp, "}\n");
		node_id++;
	}
}
