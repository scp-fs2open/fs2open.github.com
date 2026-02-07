#include "graphics/util/primitives.h"

#include <cmath>

namespace graphics {
namespace util {

generated_mesh generate_sphere_mesh(int rings, int segments)
{
	generated_mesh mesh;

	unsigned int nVertex = (rings + 1) * (segments + 1) * 3;
	unsigned int nIndex = 6 * rings * (segments + 1);

	mesh.vertices.reserve(nVertex);
	mesh.indices.reserve(nIndex);

	float fDeltaRingAngle = (PI / rings);
	float fDeltaSegAngle = (2.0f * PI / segments);
	unsigned short wVerticeIndex = 0;

	// Generate the group of rings for the sphere
	for (int ring = 0; ring <= rings; ring++) {
		float r0 = sinf(ring * fDeltaRingAngle);
		float y0 = cosf(ring * fDeltaRingAngle);

		// Generate the group of segments for the current ring
		for (int seg = 0; seg <= segments; seg++) {
			float x0 = r0 * sinf(seg * fDeltaSegAngle);
			float z0 = r0 * cosf(seg * fDeltaSegAngle);

			// Add one vertex to the strip which makes up the sphere
			mesh.vertices.push_back(x0);
			mesh.vertices.push_back(y0);
			mesh.vertices.push_back(z0);

			if (ring != rings) {
				// each vertex (except the last) has six indices pointing to it
				mesh.indices.push_back(wVerticeIndex + (ushort)segments + 1);
				mesh.indices.push_back(wVerticeIndex);
				mesh.indices.push_back(wVerticeIndex + (ushort)segments);
				mesh.indices.push_back(wVerticeIndex + (ushort)segments + 1);
				mesh.indices.push_back(wVerticeIndex + 1);
				mesh.indices.push_back(wVerticeIndex);
				wVerticeIndex++;
			}
		}
	}

	mesh.vertex_count = wVerticeIndex;
	mesh.index_count = nIndex;

	return mesh;
}

generated_mesh generate_cylinder_mesh(int segments)
{
	generated_mesh mesh;

	unsigned int nVertex = (segments + 1) * 2 * 3 + 6;
	unsigned int nIndex = 12 * (segments + 1) - 6;

	mesh.vertices.reserve(nVertex);
	mesh.indices.reserve(nIndex);

	float fDeltaSegAngle = (2.0f * PI / segments);
	unsigned short wVerticeIndex = 0;

	// Bottom cap center vertex
	mesh.vertices.push_back(0.0f);
	mesh.vertices.push_back(0.0f);
	mesh.vertices.push_back(0.0f);
	wVerticeIndex++;

	// Top cap center vertex
	mesh.vertices.push_back(0.0f);
	mesh.vertices.push_back(0.0f);
	mesh.vertices.push_back(1.0f);
	wVerticeIndex++;

	for (int ring = 0; ring <= 1; ring++) {
		float z0 = (float)ring;

		// Generate the group of segments for the current ring
		for (int seg = 0; seg <= segments; seg++) {
			float x0 = sinf(seg * fDeltaSegAngle);
			float y0 = cosf(seg * fDeltaSegAngle);

			// Add one vertex to the strip which makes up the cylinder
			mesh.vertices.push_back(x0);
			mesh.vertices.push_back(y0);
			mesh.vertices.push_back(z0);

			if (!ring) {
				mesh.indices.push_back(wVerticeIndex + (ushort)segments + 1);
				mesh.indices.push_back(wVerticeIndex);
				mesh.indices.push_back(wVerticeIndex + (ushort)segments);
				mesh.indices.push_back(wVerticeIndex + (ushort)segments + 1);
				mesh.indices.push_back(wVerticeIndex + 1);
				mesh.indices.push_back(wVerticeIndex);
				if (seg != segments) {
					mesh.indices.push_back(wVerticeIndex + 1);
					mesh.indices.push_back(wVerticeIndex);
					mesh.indices.push_back(0);
				}
				wVerticeIndex++;
			} else {
				if (seg != segments) {
					mesh.indices.push_back(wVerticeIndex + 1);
					mesh.indices.push_back(wVerticeIndex);
					mesh.indices.push_back(1);
					wVerticeIndex++;
				}
			}
		}
	}

	mesh.vertex_count = wVerticeIndex;
	mesh.index_count = nIndex;

	return mesh;
}

}
}
