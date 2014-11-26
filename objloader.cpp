#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>
#include <fstream>
#include <iostream>
#include "objloader.h"
#include "geometry.h"
#include "texture.h"
#include "light.h"

// Very, VERY simple OBJ loader.
// Here is a short list of features a real function would provide : 
// - Binary files. Reading a model should be just a few memcpy's away, not parsing a file at runtime. In short : OBJ is not very great.
// - Animations & bones (includes bones weights)
// - Multiple UVs
// - All attributes should be optional, not "forced"
// - More stable. Change a line in the OBJ file and it crashes.
// - More secure. Change another line and you can inject code.
// - Loading from memory, stream, etc

extern Light lights;
extern glm::vec3 camera, camera_up, camera_lookat;

bool loadOBJ(
	const char * path, 
	std::vector<vec3> & out_vertices, 
	std::vector<vec2> & out_uvs,
	std::vector<vec3> & out_normals,
	bool smooth
){
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<vec3> temp_vertices; 
	std::vector<vec2> temp_uvs;
	std::vector<vec3> temp_normals;


	FILE * file = fopen(path, "r");
	if( file == NULL ){
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

	while( 1 ){

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader
		
		if ( strcmp( lineHeader, "v" ) == 0 ){
			vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
			temp_vertices.push_back(vertex);
		}else if ( strcmp( lineHeader, "vt" ) == 0 ){
			vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y );
			temp_uvs.push_back(uv);
		}else if ( strcmp( lineHeader, "vn" ) == 0 ){
			vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
			temp_normals.push_back(normal);
		}else if ( strcmp( lineHeader, "f" ) == 0 ){
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
			if (matches != 9){
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices    .push_back(uvIndex[0]);
			uvIndices    .push_back(uvIndex[1]);
			uvIndices    .push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}else{
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	std::vector<vec3> ave_normals;
	std::vector<int> num_normals;
	
	if (smooth) {
		ave_normals.resize(temp_vertices.size());
		num_normals.resize(temp_vertices.size());

		for (unsigned int i = 0; i < vertexIndices.size(); ++i) {
			unsigned int index = vertexIndices[i];
			unsigned int nindex = normalIndices[i];
			ave_normals[index - 1].x += temp_normals[nindex - 1].x;
			ave_normals[index - 1].y += temp_normals[nindex - 1].y;
			ave_normals[index - 1].z += temp_normals[nindex - 1].z;
			num_normals[index - 1]++;
		}

		for (unsigned int i = 0; i < temp_vertices.size(); ++i) {
			ave_normals[i].x /= (float)num_normals[i];
			ave_normals[i].y /= (float)num_normals[i];
			ave_normals[i].z /= (float)num_normals[i];
		}
	}
	// For each vertex of each triangle
	for( unsigned int i=0; i<vertexIndices.size(); i++ ){

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		
		// Get the attributes thanks to the index
		vec3 vertex = temp_vertices[ vertexIndex-1 ];
		vec2 uv = temp_uvs[ uvIndex-1 ];
		
		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs     .push_back(uv);
		if (smooth)
			out_normals .push_back(ave_normals[ vertexIndex-1 ]);
		else
			out_normals.push_back(temp_normals[normalIndices[i]-1]);
	}
	return true;
}

bool loadScene(
	const char * path, vector<Geometry*>& geometries
) {
	std::ifstream is(path);
	char buffer[200];
	Geometry* c = 0;
	while (is >> buffer) {
		if (strcmp(buffer, "camera") == 0) {
			int tmp = 0;
			while (tmp != 7) {
				is >> buffer;
				if (strcmp(buffer, "-pos") == 0) {
					tmp += 1;
					is >> buffer;
					sscanf(buffer, "%f", &camera.x);
					is >> buffer;
					sscanf(buffer, "%f", &camera.y);
					is >> buffer;
					sscanf(buffer, "%f", &camera.z);
				} else
				if (strcmp(buffer, "-up") == 0) {
					tmp += 2;
					is >> buffer;
					sscanf(buffer, "%f", &camera_up.x);
					is >> buffer;
					sscanf(buffer, "%f", &camera_up.y);
					is >> buffer;
					sscanf(buffer, "%f", &camera_up.z);
				} else
				if (strcmp(buffer, "-lookat") == 0) {
					tmp += 4;
					is >> buffer;
					sscanf(buffer, "%f", &camera_lookat.x);
					is >> buffer;
					sscanf(buffer, "%f", &camera_lookat.y);
					is >> buffer;
					sscanf(buffer, "%f", &camera_lookat.z);
				} else
				return false;
			}
			continue;
		}
		if (strcmp(buffer, "ambient") == 0) {
			is >> buffer;
			sscanf(buffer, "%f", &lights.ambient.x);
			is >> buffer;
			sscanf(buffer, "%f", &lights.ambient.y);
			is >> buffer;
			sscanf(buffer, "%f", &lights.ambient.z);
			continue;
		}
		if (strcmp(buffer, "direct_light") == 0) {
			int tmp = 0;
			while (tmp != 3) {
				is >> buffer;
				if (strcmp(buffer, "-dir") == 0) {
					tmp += 1;
					vec3 dir;
					is >> buffer;
					sscanf(buffer, "%f", &dir.x);
					is >> buffer;
					sscanf(buffer, "%f", &dir.y);
					is >> buffer;
					sscanf(buffer, "%f", &dir.z);
					lights.direct_light_dir.push_back(dir);
				} else
				if (strcmp(buffer, "-color") == 0) {
					tmp += 2;
					vec3 color;
					is >> buffer;
					sscanf(buffer, "%f", &color.x);
					is >> buffer;
					sscanf(buffer, "%f", &color.y);
					is >> buffer;
					sscanf(buffer, "%f", &color.z);
					lights.direct_light_color.push_back(color);					
				} else {
					return false;
				}
			}
			continue;
		}
		if (strcmp(buffer, "point_light") == 0) {
			int tmp = 0;
			while (tmp != 3) {
				is >> buffer;
				if (strcmp(buffer, "-pos") == 0) {
					tmp += 1;
					vec3 dir;
					is >> buffer;
					sscanf(buffer, "%f", &dir.x);
					is >> buffer;
					sscanf(buffer, "%f", &dir.y);
					is >> buffer;
					sscanf(buffer, "%f", &dir.z);
					lights.point_light_pos.push_back(dir);
				} else
				if (strcmp(buffer, "-color") == 0) {
					tmp += 2;
					vec3 color;
					is >> buffer;
					sscanf(buffer, "%f", &color.x);
					is >> buffer;
					sscanf(buffer, "%f", &color.y);
					is >> buffer;
					sscanf(buffer, "%f", &color.z);
					lights.point_light_color.push_back(color);					
				} else {
					return false;
				}
			}
			continue;
		}
		if (buffer[0] != '-') {
			c = new Geometry((string("obj/")+buffer).c_str());
			geometries.push_back(c);
			continue;
		}
		if (!c) {
			cout << "Wrong format!\n";
			return false;
		}
		if (strcmp(buffer, "-kd") == 0) {
			is >> buffer;
			sscanf(buffer, "%f", &c->kd);
			continue;
		}
		if (strcmp(buffer, "-ks") == 0) {
			is >> buffer;
			sscanf(buffer, "%f", &c->ks);
			continue;
		}
		if (strcmp(buffer, "-ka") == 0) {
			is >> buffer;
			sscanf(buffer, "%f", &c->ka);
			continue;
		}
		if (strcmp(buffer, "-kr") == 0) {
			is >> buffer;
			sscanf(buffer, "%f", &c->kr);
			continue;
		}
		if (strcmp(buffer, "-kf") == 0) {
			is >> buffer;
			sscanf(buffer, "%f", &c->kf);
			continue;
		}
		if (strcmp(buffer, "-alpha") == 0) {
			is >> buffer;
			sscanf(buffer, "%f", &c->alpha);
			continue;
		}
		if (strcmp(buffer, "-nr") == 0) {
			is >> buffer;
			sscanf(buffer, "%f", &c->nr);
			continue;
		}
		if (strcmp(buffer, "-tex") == 0) {
			is >> buffer;
			c->tex = TexManager::createRenderTexture(buffer);
			continue;
		}
		if (strcmp(buffer, "-offset") == 0) {
			float x, y, z;
			is >> buffer;
			sscanf(buffer, "%f", &x);
			is >> buffer;
			sscanf(buffer, "%f", &y);
			is >> buffer;
			sscanf(buffer, "%f", &z);
			c->translate(x, y, z);
			continue;
		}
		if (strcmp(buffer, "-scale") == 0) {
			float x, y, z;
			is >> buffer;
			sscanf(buffer, "%f", &x);
			is >> buffer;
			sscanf(buffer, "%f", &y);
			is >> buffer;
			sscanf(buffer, "%f", &z);
			c->scale(x, y, z);
			continue;
		}
		if (strcmp(buffer, "-rotate") == 0) {
			is >> buffer;
			glm::vec3 axis;
			if (buffer[0] == 'x')
				axis = glm::vec3(1,0,0);
			else
			if (buffer[0] == 'y')
				axis = glm::vec3(0,1,0);
			else
				axis = glm::vec3(0,0,1);
			is >> buffer;
			float angle;
			sscanf(buffer, "%f", &angle);
			c->rotation(axis, angle);
			continue;
		}
	}
	return true;
}