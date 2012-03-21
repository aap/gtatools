#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "gta.h"
#include "gl.h"
#include "primitives.h"
#include "drawable.h"
#include "directory.h"

using namespace std;
using namespace gl;

void Drawable::printFrames(int level, Frame *r)
{
	for (int i = 0; i < level; i++)
		cout << "  ";
	cout << r->name << " " << r->parent << " " <<
	        r->boneId << " " << r->geo<< endl;
	for (uint i = 0; i < r->children.size(); i++)
		printFrames(level+1, r->children[i]);
}

void Drawable::attachClump(rw::Clump &c)
{
	clump = c;
	// frames
	for (uint i = 0; i < clump.frameList.size(); i++) {
		rw::Frame &rwf = clump.frameList[i];
		Frame *f = new Frame;

		if (i == 0)	// sensible default if there is no hanim
			animRoot = f;

		f->boneId = -1;
		if (rwf.hasHAnim)
			f->boneId = rwf.hAnimBoneId;

		glm::vec4 x, y, z, w;
		#define M(i,j) rwf.rotationMatrix[(i)*3+(j)]
		x.x = M(0,0); y.x = M(1,0); z.x = M(2,0); w.x = 0.0f;
		x.y = M(0,1); y.y = M(1,1); z.y = M(2,1); w.y = 0.0f;
		x.z = M(0,2); y.z = M(1,2); z.z = M(2,2); w.z = 0.0f;
		x.w = 0.0f;   y.w = 0.0f;   z.w = 0.0f;   w.w = 1.0f;
		#undef M
		f->pos = glm::vec3(rwf.position[0],
		                   rwf.position[1],
		                   rwf.position[2]);

		f->modelMat = glm::mat4(x, y, z, w);
		f->boneMat = glm::mat4(1.0f);
		f->boneInverseMat = glm::mat4(1.0f);
		f->modelMat[3][0] = f->pos.x;
		f->modelMat[3][1] = f->pos.y;
		f->modelMat[3][2] = f->pos.z;
		f->name = rwf.name;
		stringToLower(f->name);
		f->index = i;
		f->geo = -1;

		if (rwf.parent == -1) {
			f->parent = 0;
			root = f;
		} else {
			f->parent = frmList[rwf.parent];
			f->parent->children.push_back(f);
		}

		// found bone hierarchy, build boneindex-to-frame table
		// don't know if there are models with more than one hierarchy
		if (rwf.hAnimBoneCount) {
			animRoot = f;
			for (uint j = 0; j < rwf.hAnimBoneCount; j++) {
				bool found = false;
				for (uint k=0;k < clump.frameList.size(); k++) {
					rw::Frame &rwf2 = clump.frameList[k];
					if (rwf2.hAnimBoneId ==
					    rwf.hAnimBoneIds[j]) {
						boneToFrame.push_back(k);
						found = true;
						break;
					}
				}
				if (!found) {
					cerr << "bone missing\n";
					boneToFrame.push_back(0);
				}
			}
		}

		frmList.push_back(f);
	}


	// geometries
	GLuint vbo, ibo;
	GLint size;
	for (uint i = 0; i < clump.geometryList.size(); i++) {
		rw::Geometry &rwg = clump.geometryList[i];
		Geometry geo;

		for (uint j = 0; j < rwg.materialList.size(); j++) {
			rw::Material &m = rwg.materialList[j];
			if (m.hasTex)
				stringToLower(m.texture.name);
				stringToLower(m.texture.maskName);
			// TODO: to this for other textures also
		}

		geo.boundingSphere = quat(rwg.boundingSphere[3],
		                          rwg.boundingSphere[0],
		                          rwg.boundingSphere[1],
		                          rwg.boundingSphere[2]);

		uint numVertices = rwg.vertices.size() / 3;

		size = numVertices*3*sizeof(GLfloat);	// vertices
		if (rwg.flags & rw::FLAGS_PRELIT)
			size += numVertices*4*sizeof(GLubyte);	// colors
		if (rwg.flags & rw::FLAGS_NORMALS)
			size += numVertices*3*sizeof(GLfloat);	// normals
		if (rwg.flags & rw::FLAGS_TEXTURED ||
		    rwg.flags & rw::FLAGS_TEXTURED2)
			size += numVertices*2*sizeof(GLfloat);	// texcoords

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, size, 0, GL_STATIC_DRAW);

		GLfloat *vertices = &rwg.vertices[0];
		geo.isSkinned = rwg.hasSkin;
		if (geo.isSkinned) {
			geo.vertices = rwg.vertices;
			vertices = &geo.vertices[0];

			if (rwg.boneCount != boneToFrame.size())
				cerr << "error: bone count doesn't match\n";
			uint o = 0;
			for (uint j = 0; j < rwg.boneCount; j++) {
				Frame *f = frmList[boneToFrame[j]];
				#define M(i,j) f->boneInverseMat[(i)][(j)]
				#define N(i,j) rwg.inverseMatrices[o+(i)*4+(j)]
				M(0,0) = N(0,0);
				M(0,1) = N(0,1);
				M(0,2) = N(0,2);
				M(0,3) = 0.0f;//N(0,3); // these are junk
				M(1,0) = N(1,0);
				M(1,1) = N(1,1);
				M(1,2) = N(1,2);
				M(1,3) = 0.0f;//N(1,3);
				M(2,0) = N(2,0);
				M(2,1) = N(2,1);
				M(2,2) = N(2,2);
				M(2,3) = 0.0f;//N(2,3);
				M(3,0) = N(3,0);
				M(3,1) = N(3,1);
				M(3,2) = N(3,2);
				M(3,3) = 1.0f;//N(3,3);
				#undef M
				#undef N
				o += 16;
			}
		}

		GLint offset = 0;
		glBufferSubData(GL_ARRAY_BUFFER, offset,
		                numVertices*3*sizeof(GLfloat),
		                vertices);
		offset += numVertices*3*sizeof(GLfloat);

		if (rwg.flags & rw::FLAGS_PRELIT) {
			glBufferSubData(GL_ARRAY_BUFFER, offset,
					numVertices*4*sizeof(GLubyte),
			                &rwg.vertexColors[0]);
			offset += numVertices*4*sizeof(GLubyte);
		}
		if (rwg.flags & rw::FLAGS_NORMALS) {
			glBufferSubData(GL_ARRAY_BUFFER, offset,
					numVertices*3*sizeof(GLfloat),
			                &rwg.normals[0]);
			offset += numVertices*3*sizeof(GLfloat);
		}
		if (rwg.flags & rw::FLAGS_TEXTURED ||
		    rwg.flags & rw::FLAGS_TEXTURED2) {
			glBufferSubData(GL_ARRAY_BUFFER, offset,
					numVertices*2*sizeof(GLfloat),
			                &rwg.texCoords[0][0]);
			offset += numVertices*2*sizeof(GLfloat);
		}

		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ARRAY_BUFFER, ibo);

		GLint size = 0;
		for (uint j = 0; j < rwg.splits.size(); j++)
			size += rwg.splits[j].indices.size()*sizeof(rw::uint32);
		glBufferData(GL_ARRAY_BUFFER, size, 0, GL_STATIC_DRAW);

		offset = 0;
		for (uint j = 0; j < rwg.splits.size(); j++) {
			rw::Split &s = rwg.splits[j];
			glBufferSubData(GL_ARRAY_BUFFER,
				        offset,
				        s.indices.size()*sizeof(rw::uint32),
			                &s.indices[0]);
			offset += s.indices.size()*sizeof(rw::uint32);
		}

		geo.vbo = vbo;
		geo.ibo = ibo;
		geoList.push_back(geo);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	// atomics
	//
	// reorder so number of object according to item definition matches
	// number of atomic (based on suffix '_L[012]' in frame name
	vector<int> tmpList;
	for (uint i = 0; i < clump.atomicList.size(); i++) {
		rw::Atomic &atm = clump.atomicList[i];
		Frame *f = frmList[atm.frameIndex];
		f->geo = atm.geometryIndex;
		string name = f->name;
		stringToLower(name);
		if (name.size() < 3) {
			tmpList.push_back(atm.frameIndex);
			continue;
		}
		string end = name.substr(name.size()-3, 3);
		if (end.substr(0,2) == "_l") {
			int number = end[2] - '0';
			if (number >= atomicList.size())
				atomicList.resize(number+1);
			atomicList[number] = atm.frameIndex;
		} else {
			tmpList.push_back(atm.frameIndex);
		}
	}
	atomicList.insert(atomicList.end(), tmpList.begin(), tmpList.end());

	updateFrames(root);
	updateGeometries();
}

void Drawable::attachAnim(rw::Animation &a)
{
	anim = a;
	endFrame = 0;
	for (uint i = 0; i < anim.objList.size(); i++) {
		if (endFrame < anim.objList[i].frames)
			endFrame = anim.objList[i].frames;
		stringToLower(anim.objList[i].name);
	}
	endFrame-=1;
	frame = -1;	// will be incremented to 0 upon updating
	cout << "attached animation " << anim.name << endl;
	cout << endFrame + 1 << " frames\n";
}

int Drawable::load(string model, string texdict)
{
	ifstream dff;

	if (directory.openFile(dff, model) == -1) {
		cout << "couldn't open " << model << endl;
		return -1;
	}
	rw::Clump c;
	c.read(dff);
	dff.close();

	texDict = texMan.get(texdict);

	// catch (uv anim dicts)
	if (c.atomicList.size() == 0)
		return 0;

	attachClump(c);

	return 0;
}

void Drawable::unload(void)
{
	clump.clear();
	texMan.release(texDict->fileName);
	anim.clear();
	for (uint i = 0; i < geoList.size(); i++) {
		if (geoList[i].vbo != 0)
			glDeleteBuffers(1, &geoList[i].vbo);
		if (geoList[i].ibo != 0)
			glDeleteBuffers(1, &geoList[i].ibo);
		geoList[i].vbo = 0;
		geoList[i].ibo = 0;
	}
	geoList.resize(0);
	for (uint i = 0; i < frmList.size(); i++) {
		delete frmList[i];
		frmList[i] = 0;
	}
	frmList.resize(0);
	boneToFrame.resize(0);
	animRoot = root = 0;
}

void Drawable::applyAnim(Frame *f)
{
	uint oi;
	for (oi = 0; oi < anim.objList.size(); oi++)
		if (f->name == anim.objList[oi].name)
			break;

	if (oi < anim.objList.size()) {
		rw::AnimObj &ao = anim.objList[oi];
		rw::KeyFrame &kf = ao.frmList[frame];

		glm::quat q;
		q.x = -kf.rot[0];
		q.y = -kf.rot[1];
		q.z = -kf.rot[2];
		q.w = kf.rot[3];

		if (kf.type == rw::KRT0 || kf.type == rw::KRTS) {
			f->pos.x = kf.pos[0];
			f->pos.y = kf.pos[1];
			f->pos.z = kf.pos[2];
		}
		// no scaling yet

		f->modelMat = glm::mat4_cast(q);
		f->modelMat[3][0] = f->pos.x;
		f->modelMat[3][1] = f->pos.y;
		f->modelMat[3][2] = f->pos.z;
	}

	for (uint i = 0; i < f->children.size(); i++)
		applyAnim(f->children[i]);
}

void Drawable::nextFrame(void)
{
	frame++;
	if (frame >= endFrame)
		frame = 0;
	applyAnim(animRoot);
	updateFrames(animRoot);
	updateGeometries();
}

void Drawable::updateGeometries(void)
{
	for (uint i = 0; i < geoList.size(); i++) {
		Geometry &g = geoList[i];
		rw::Geometry &rwg = clump.geometryList[i];

		if (!g.isSkinned)
			continue;

		for (uint j = 0; j < g.vertices.size()/3; j++) {
			glm::vec4 v(rwg.vertices[j*3+0],
			            rwg.vertices[j*3+1],
			            rwg.vertices[j*3+2], 1.0f);
			uint indices[4];
			float weights[4];

			indices[0] = rwg.vertexBoneIndices[j]&0xFF;
			indices[1] = (rwg.vertexBoneIndices[j]&0xFF00)>>8;
			indices[2] = (rwg.vertexBoneIndices[j]&0xFF0000)>>16;
			indices[3] = (rwg.vertexBoneIndices[j]&0xFF000000)>>24;

/*
			if (rwg.specialIndices.size() != 0) {
				indices[0] = rwg.specialIndices[indices[0]];
				indices[1] = rwg.specialIndices[indices[1]];
				indices[2] = rwg.specialIndices[indices[2]];
				indices[3] = rwg.specialIndices[indices[3]];
			}
*/

			weights[0] = rwg.vertexBoneWeights[j*4+0];
			weights[1] = rwg.vertexBoneWeights[j*4+1];
			weights[2] = rwg.vertexBoneWeights[j*4+2];
			weights[3] = rwg.vertexBoneWeights[j*4+3];

			glm::mat4 m;
			m  = frmList[boneToFrame[indices[0]]]->boneMat *
			     weights[0];
			m += frmList[boneToFrame[indices[1]]]->boneMat *
			     weights[1];
			m += frmList[boneToFrame[indices[2]]]->boneMat *
			     weights[2];
			m += frmList[boneToFrame[indices[3]]]->boneMat *
			     weights[3];

			v = m * v;

			g.vertices[j*3+0] = v.x;
			g.vertices[j*3+1] = v.y;
			g.vertices[j*3+2] = v.z;
		}
		if (g.vbo == 0)
			continue;
		glBindBuffer(GL_ARRAY_BUFFER, g.vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0,
		                g.vertices.size()*sizeof(GLfloat),
		                &g.vertices[0]);
	}
}

void Drawable::updateFrames(Frame *f)
{
	if (f->parent)
		f->ltm = f->parent->ltm * f->modelMat;
	else
		f->ltm = f->modelMat;
	if (f->boneId != -1)
		f->boneMat = f->ltm * f->boneInverseMat;
	for (uint i = 0; i < f->children.size(); i++)
		updateFrames(f->children[i]);
}

void Drawable::draw(void)
{
	if (frmList.size() == 0)
		return;
	drawFrame(0, true, true);
}

void Drawable::drawAtomic(int ai)
{
	if (ai >= 0 && ai < atomicList.size())
		drawFrame(atomicList[ai], true, false);
	else
		; // can happen
}

void Drawable::drawFrame(int fi, bool recurse, bool transform)
{
	if (uint(fi) >= frmList.size())
		return;

	Frame *f = frmList[fi];

	if (!f)
		return;

	glm::mat4 save = modelMat;

	if (transform)
		modelMat *= f->ltm;

	glm::mat4 modelView = viewMat * modelMat;
	glm::mat3 normal = glm::inverseTranspose(glm::mat3(modelView));
	glUniformMatrix4fv(gl::u_ModelView, 1, GL_FALSE,
	                   glm::value_ptr(modelView));
	glUniformMatrix3fv(gl::u_NormalMat, 1, GL_FALSE,
	                   glm::value_ptr(normal));

	if (!strstr(f->name.c_str(), "chassis_vlo") &&
	    !strstr(f->name.c_str(), "_dam")) {
		if (f->geo != -1) {
			drawGeometry(f->geo);
		} else {
			glBindTexture(GL_TEXTURE_2D, gl::whiteTex);
			gl::drawSphere(0.1f, 10, 10);
		}
	}

	modelMat = save;
	modelView = viewMat * modelMat;
	normal = glm::inverseTranspose(glm::mat3(modelView));
	glUniformMatrix4fv(gl::u_ModelView, 1, GL_FALSE,
	                   glm::value_ptr(modelView));
	glUniformMatrix3fv(gl::u_NormalMat, 1, GL_FALSE,
	                   glm::value_ptr(normal));

	if (recurse)
		for (uint i = 0; i < f->children.size(); i++)
			drawFrame(f->children[i]->index, true, transform);

}

void Drawable::drawGeometry(int gi)
{
	if (uint(gi) >= clump.geometryList.size())
		return;

	rw::Geometry &g = clump.geometryList[gi];

	if (geoList[gi].vbo == 0|| geoList[gi].ibo == 0)
		return;

	// sensible defaults
	glVertexAttrib4f(in_Color, 1.0f, 1.0f, 1.0f, 1.0f);
	glVertexAttrib3f(in_Normal, 0.0f, 0.0f, 0.0f);

	uint numVertices = g.vertices.size() / 3;
	glBindBuffer(GL_ARRAY_BUFFER, geoList[gi].vbo);
	glEnableVertexAttribArray(in_Vertex);
	GLint offset = 0;
	glVertexAttribPointer(in_Vertex, 3, GL_FLOAT, GL_FALSE, 0,
			      (GLvoid*) offset);
	offset += numVertices*3*sizeof(GLfloat);
	if (g.flags & rw::FLAGS_PRELIT) {
		glEnableVertexAttribArray(in_Color);
		glVertexAttribPointer(in_Color, 4, GL_UNSIGNED_BYTE,
				      GL_TRUE, 0, (GLvoid*) offset);
		offset += numVertices*4*sizeof(GLubyte);
	}
	if (g.flags & rw::FLAGS_NORMALS) {
		glEnableVertexAttribArray(in_Normal);
		glVertexAttribPointer(in_Normal, 3, GL_FLOAT,
				      GL_FALSE, 0, (GLvoid*) offset);
		offset += numVertices*3*sizeof(GLfloat);
	}
	if (g.flags & rw::FLAGS_TEXTURED ||
	    g.flags & rw::FLAGS_TEXTURED2) {
		glEnableVertexAttribArray(in_TexCoord);
		glVertexAttribPointer(in_TexCoord, 2, GL_FLOAT,
				      GL_FALSE, 0, (GLvoid*) offset);
		offset += numVertices*2*sizeof(GLfloat);
	}


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geoList[gi].ibo);
	GLenum mode = (g.faceType == rw::FACETYPE_STRIP) ?
		      GL_TRIANGLE_STRIP : GL_TRIANGLES;
	offset = 0;
	for (uint j = 0; j < g.splits.size(); j++) {
		rw::Split s = g.splits[j];
		bool isTransparent = false;

		// bind texture
		glActiveTexture(GL_TEXTURE0);
		int matid = s.matIndex;
		string texname = "";
		if (g.materialList[matid].hasTex) {
			texname = g.materialList[matid].texture.name;
			Texture *t = texDict->get(texname);

			if (t != 0 && t->tex != 0) {
				if (t->hasAlpha)
					isTransparent = true;
				glBindTexture(GL_TEXTURE_2D,
				              t->tex);
			} else {
				glBindTexture(GL_TEXTURE_2D,
				              gl::whiteTex);
			}
		} else {
			glBindTexture(GL_TEXTURE_2D, gl::whiteTex);
		}
		if (g.materialList[matid].color[3] != 255)
			isTransparent = true;
		gl::wasTransparent |= isTransparent;
		if (isTransparent != gl::drawTransparent) {
			offset += s.indices.size();
			continue;
		}

		glUniform1i(u_Texture, 0);

		glm::vec4 matCol;
		matCol.x = (float) g.materialList[matid].color[0] / 255.0f;
		matCol.y = (float) g.materialList[matid].color[1] / 255.0f;
		matCol.z = (float) g.materialList[matid].color[2] / 255.0f;
		matCol.w = (float) g.materialList[matid].color[3] / 255.0f;
		glUniform4fv(u_MatColor, 1, glm::value_ptr(matCol));

		if (isTransparent) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0.0);
		} else {
			glDisable(GL_BLEND);
			glDisable(GL_ALPHA_TEST);
		}

		if (gl::drawWire) {
			glBindTexture(GL_TEXTURE_2D, gl::whiteTex);
			glDisableVertexAttribArray(in_Color);
			glDisableVertexAttribArray(in_Normal);
			glVertexAttrib4f(in_Color, 0.8f, 0.8f, 0.8f, 1.0f);
			glLineWidth(2);
			int add = (mode == rw::FACETYPE_STRIP) ? 1 : 3;
			for (uint i = 0; i < s.indices.size()-2; i += add) {
				glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_INT,
					       (GLvoid*)
				               (offset*sizeof(rw::uint32)));
				offset += add;
			}
		} else {
			glDrawElements(mode, s.indices.size(),
				       GL_UNSIGNED_INT,
				       (GLvoid*) (offset*sizeof(rw::uint32)));
			offset += g.splits[j].indices.size();
		}
	}
	glUniform1i(u_Texture, -1);

	glDisableVertexAttribArray(in_Vertex);
	glDisableVertexAttribArray(in_Color);
	glDisableVertexAttribArray(in_Normal);
	glDisableVertexAttribArray(in_TexCoord);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


//quat Drawable::getBoundingSphere(void)
vector<quat> Drawable::getBoundingSpheres(void)
{
	vector<quat> spheres;
	for (uint i = 0; i < geoList.size(); i++)
		spheres.push_back(geoList[i].boundingSphere);
	if (spheres.size() == 0)
		spheres.push_back(quat(0.0f, 0.0f, 0.0f, 0.0f));
	return spheres;
}
