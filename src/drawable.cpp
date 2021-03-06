#include <iostream>
#include <cstring>

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
#include "world.h"
#include "timecycle.h"
#include "renderer.h"
#include "jobqueue.h"

using namespace std;
using namespace gl;

void dumpmat(const float *m)
{
	cout.precision(5);
	cout << std::fixed;
	cout << m[0] << " " << m[4] << " " << m[8] << " " << m[12] << endl;
	cout << m[1] << " " << m[5] << " " << m[9] << " " << m[13] << endl;
	cout << m[2] << " " << m[6] << " " << m[10] << " " << m[14] << endl;
	cout << m[3] << " " << m[7] << " " << m[11] << " " << m[15] << endl;
}

void Drawable::printFrames(int level, Frame *r)
{
	for (int i = 0; i < level; i++)
		cout << "  ";
	if (r == 0) {
		r = root;
		cout << endl;
	}
	cout << r->name << dec << " " << //(r->parent != 0) << " " <<
	        r->boneId << " " << r->geo;
	if (r == animRoot)
		cout << "    <--";
	if (r == skin)
		cout << "    <==";
	cout << endl;

/*
	dumpmat(glm::value_ptr(r->defMat));
	cout << "---- ^ def\n";
	dumpmat(glm::value_ptr(r->modelMat));
	cout << "---- ^ modelmat\n";
	dumpmat(glm::value_ptr(r->ltm));
	cout << "---- ^ ltm\n";
	dumpmat(glm::value_ptr(r->boneMat));
	cout << "---- ^ bonemat\n";
	dumpmat(glm::value_ptr(glm::inverse(r->boneInverseMat)));
	cout << "---- ^ boneinvinvmat\n";
	dumpmat(glm::value_ptr(r->boneInverseMat));
	cout << "---- ^ boneinvmat\n";
	dumpmat(glm::value_ptr(r->ltm*r->boneInverseMat));
	cout << "---- ^ **\n";
	cout << endl;
*/

/*
	for (int i = 0; i < level; i++)
		cout << "  ";
	cout << r->pos[0] << " " << r->pos[1] << " " << r->pos[2] << endl;
*/
	for (size_t i = 0; i < r->children.size(); i++)
		printFrames(level+1, r->children[i]);
}

void Drawable::attachClump(rw::Clump *clp)
{
	THREADCHECK();
	// frames
	size_t frmsize = clp->frameList.size();
	for (uint i = 0; i < frmsize; i++) {
		rw::Frame &rwf = clp->frameList[i];
		Frame *f = new Frame;

		f->dotransform = true;

		animRoot = 0;

		f->boneId = rwf.hasHAnim ? rwf.hAnimBoneId : -1;

		f->pos = glm::vec3(rwf.position[0],
		                   rwf.position[1],
		                   rwf.position[2]);
		f->defPos = f->pos;

		glm::vec4 x, y, z, w;
		#define M(i,j) rwf.rotationMatrix[(i)*3+(j)]
		x.x = M(0,0); y.x = M(1,0); z.x = M(2,0); w.x = f->pos.x;
		x.y = M(0,1); y.y = M(1,1); z.y = M(2,1); w.y = f->pos.y;
		x.z = M(0,2); y.z = M(1,2); z.z = M(2,2); w.z = f->pos.z;
		x.w = 0.0f;   y.w = 0.0f;   z.w = 0.0f;   w.w = 1.0f;
		#undef M
		f->modelMat = glm::mat4(x, y, z, w);
		f->defMat = f->modelMat;

		f->boneMat = glm::mat4(1.0f);
		f->boneInverseMat = glm::mat4(1.0f);

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
				for (size_t k=0;k < clp->frameList.size();k++) {
					rw::Frame &rwf2 = clp->frameList[k];
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
	if (animRoot == 0) {
		animRoot = root;
		if (!root->children.empty())
			animRoot = root->children[0];
	}

	// geometries
	GLuint vbo, ibo;
	GLint size;
	for (size_t i = 0; i < clp->geometryList.size(); i++) {
		rw::Geometry &rwg = clp->geometryList[i];
		Geometry geo;

		for (size_t j = 0; j < rwg.materialList.size(); j++) {
			rw::Material &m = rwg.materialList[j];
			if (m.hasTex) {
				stringToLower(m.texture.name);
				stringToLower(m.texture.maskName);
			}
			// TODO: to this for other textures also
		}

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
		GLfloat *normals = &rwg.normals[0];
		geo.isSkinned = rwg.hasSkin;
		if (geo.isSkinned) {
			geo.vertices = rwg.vertices;
			vertices = &geo.vertices[0];
			if (rwg.flags & rw::FLAGS_NORMALS) {
				geo.normals = rwg.normals;
				normals = &rwg.normals[0];
			}

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
		currentColorStep = 0;
		geo.vertexColors = rwg.vertexColors;

		GLint offset = 0;
		glBufferSubData(GL_ARRAY_BUFFER, offset,
		                numVertices*3*sizeof(GLfloat),
		                vertices);
		offset += numVertices*3*sizeof(GLfloat);

		if (rwg.flags & rw::FLAGS_PRELIT) {
			glBufferSubData(GL_ARRAY_BUFFER, offset,
					numVertices*4*sizeof(GLubyte),
					&geo.vertexColors[0]);
			offset += numVertices*4*sizeof(GLubyte);
		}
		if (rwg.flags & rw::FLAGS_NORMALS) {
			glBufferSubData(GL_ARRAY_BUFFER, offset,
					numVertices*3*sizeof(GLfloat),
			                normals);
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
		for (size_t j = 0; j < rwg.splits.size(); j++)
			size += rwg.splits[j].indices.size()*sizeof(rw::uint32);
		glBufferData(GL_ARRAY_BUFFER, size, 0, GL_STATIC_DRAW);

		offset = 0;
		for (size_t j = 0; j < rwg.splits.size(); j++) {
			rw::Split &s = rwg.splits[j];
			glBufferSubData(GL_ARRAY_BUFFER,
				        offset,
				        s.indices.size()*sizeof(rw::uint32),
			                &s.indices[0]);
			offset += s.indices.size()*sizeof(rw::uint32);
		}

		geo.vbo = vbo;
		geo.ibo = ibo;
		geo.dirty = false;
		geoList.push_back(geo);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// atomics
	//
	// reorder so number of object according to item definition matches
	// number of atomic (based on suffix '_L[012]' in frame name)
	vector<int> tmpList;
	for (size_t i = 0; i < clp->atomicList.size(); i++) {
		rw::Atomic &atm = clp->atomicList[i];
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
			uint number = end[2] - '0';
			if (number >= atomicList.size())
				atomicList.resize(number+1);
			atomicList[number] = atm.frameIndex;
		} else {
			tmpList.push_back(atm.frameIndex);
		}
	}
	atomicList.insert(atomicList.end(), tmpList.begin(), tmpList.end());

	for (size_t i = 0; i < frmList.size(); i++) {
		if (frmList[i]->geo >= 0)
			if (geoList[frmList[i]->geo].isSkinned)
				skin =frmList[i];
	}

	clump = clp;

	updateFrames(root);
	updateGeometries();
}

void Drawable::attachAnim(Animation *a)
{
	manim.attachAnims(a, 0, 1.0f);
	setTime(0.0);
	cout << "attached animation " << a->name << endl;
	cout << manim.endTime << " seconds\n";
}

void Drawable::attachMixedAnim(Animation *a, Animation *b, float f)
{
	manim.attachAnims(a, b, f);
	setTime(0.0);
	cout << "attached animations " << a->name << " " << b->name << endl;
	cout << manim.endTime << " seconds\n";
}

void Drawable::attachOverrideAnim(Animation *a)
{
	overrideAnim = a;
	curOvrTime = 0.0f;
	cout << "attached override animation " << overrideAnim->name << endl;
	cout << overrideAnim->endTime << " seconds\n";
}

void Drawable::request(string model, string texdict)
{
	char *str = new char[model.size()+1];
	strcpy(str, model.c_str());
	normalJobs.addJob(JobQueue::readDff, this, str);
	texDict = 0;
	if (renderer->doTextures)
		texDict = texMan->get(texdict);
}

void Drawable::release(void)
{
	normalJobs.addJob(JobQueue::deleteDrawable, this, 0);
}

int Drawable::loadSynch(string model, string texdict)
{
	ifstream dff;

	rw::Clump *clp = new rw::Clump;
	if (directory->openFile(dff, model) == -1) {
		cout << "couldn't open " << model << endl;
		return -1;
	}
	clp->read(dff);
	dff.close();

	attachClump(clp);

	texDict = 0;
	if (renderer->doTextures)
		texDict = texMan->get(texdict, true);

	return 0;
}

float Drawable::getOvrTime(void)
{
	return curOvrTime;
}

void Drawable::setOvrTime(float t)
{
	if (overrideAnim == 0)
		return;
	curOvrTime = t;
	while (curOvrTime > overrideAnim->endTime)
		curOvrTime -= overrideAnim->endTime;
	while (curOvrTime < 0.0f)
		curOvrTime += overrideAnim->endTime;
	overrideAnim->apply(curOvrTime/overrideAnim->endTime, animRoot);
	updateFrames(animRoot);
	updateGeometries();
}

float Drawable::getTime(void)
{
	return curTime;
}

bool Drawable::setTime(float t)
{
	if (manim.endTime <= 0.0)
		return false;
	curTime = t;
	bool skipped = false;
	while (curTime > manim.endTime) {
		curTime -= manim.endTime;
		skipped = true;
	}
	while (curTime < 0.0f) {
		curTime += manim.endTime;
		skipped = true;
	}
	manim.apply(curTime/manim.endTime, animRoot);
	updateFrames(animRoot);
	updateGeometries();
	return skipped;
}

void Drawable::updateGeometries(void)
{
	for (size_t i = 0; i < geoList.size(); i++) {
		Geometry &g = geoList[i];
		rw::Geometry &rwg = clump->geometryList[i];

		if (!g.isSkinned)
			continue;

		uint indices[4];
		float weights[4];
		for (size_t j = 0; j < g.vertices.size()/3; j++) {
			indices[0] = rwg.vertexBoneIndices[j]&0xFF;
			indices[1] = (rwg.vertexBoneIndices[j]&0xFF00)>>8;
			indices[2] = (rwg.vertexBoneIndices[j]&0xFF0000)>>16;
			indices[3] = (rwg.vertexBoneIndices[j]&0xFF000000)>>24;
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

			// Vertices
			glm::vec4 v(rwg.vertices[j*3+0],
			            rwg.vertices[j*3+1],
			            rwg.vertices[j*3+2], 1.0f);
			v = m * v;
			g.vertices[j*3+0] = v.x;
			g.vertices[j*3+1] = v.y;
			g.vertices[j*3+2] = v.z;

			// Normals
			// I hope, we don't have to care about scaling
			if (rwg.flags & rw::FLAGS_NORMALS) {
				glm::vec4 n(rwg.normals[j*3+0],
					    rwg.normals[j*3+1],
					    rwg.normals[j*3+2], 0.0f);
				n = m * n;
				g.normals[j*3+0] = n.x;
				g.normals[j*3+1] = n.y;
				g.normals[j*3+2] = n.z;
			}
		}
		g.dirty = true;
	}
}

void Drawable::updateFrames(Frame *f)
{
	if (f->parent)
		f->ltm = f->parent->ltm * f->modelMat;
	else
		f->ltm = f->modelMat;

	// urgh, probably better do bone matrices per skin
	if (f->boneId != -1) {
		if (f->parent &&
		    (f->geo < 0 || (f->geo >= 0 && !geoList[f->geo].isSkinned)))
			f->boneMat = f->parent->boneMat * f->modelMat;
		// this is a guess, but it seems to work
		else
			f->boneMat = glm::mat4(1.0);
	}

	for (size_t i = 0; i < f->children.size(); i++)
		updateFrames(f->children[i]);

	if (f->boneId != -1)
		f->boneMat = f->boneMat * f->boneInverseMat;
}

void Drawable::setVertexColors(void)
{
	THREADCHECK();
	for (size_t i = 0; i < clump->geometryList.size(); i++) {
		rw::Geometry &rwg = clump->geometryList[i];
		Geometry &geo = geoList[i];

		if (!rwg.hasNightColors || !(rwg.flags && rw::FLAGS_PRELIT))
			continue;
		if (rwg.nightColors.size() != rwg.vertexColors.size()) {
			cerr << "vertex color size mismatch\n";
			continue;
		}
		// 0.0 is day, 1.0 is night
		float a = timeCycle.getColorStep() / 5.0f;

		for (size_t j = 0; j < rwg.nightColors.size(); j++)
			geo.vertexColors[j] = rwg.nightColors[j]*a + 
			                      rwg.vertexColors[j]*(1-a);

		int numVertices = rwg.vertices.size()/3;
		GLint offset = numVertices*3*sizeof(GLfloat);
		glBindBuffer(GL_ARRAY_BUFFER, geo.vbo);
		glBufferSubData(GL_ARRAY_BUFFER, offset,
				numVertices*4*sizeof(GLubyte),
				&geo.vertexColors[0]);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	currentColorStep = timeCycle.getColorStep();
}

void Drawable::draw(void)
{
	if (clump == 0)
		cout << "warning: no clump attached\n";
	if (currentColorStep != timeCycle.getColorStep())
		setVertexColors();

	if (frmList.empty())
		return;
//	drawFrame(root, true, true);
	drawFrame(root, true, false);
}

void Drawable::drawAtomic(uint ai)
{
	if (clump == 0)
		cout << "warning: no clump attached\n";
	if (currentColorStep != timeCycle.getColorStep())
		setVertexColors();

	if (ai < atomicList.size())
		drawFrame(frmList[atomicList[ai]], true, false);
}

void Drawable::drawFrame(Frame *f, bool recurse, bool transform)
{
	if (!f)
		return;

	glm::mat4 mvSave = gl::state.modelView;
	glm::mat3 nrmSave = gl::state.normalMat;

	if (transform && f->dotransform)
//		gl::state.modelView *= f->ltm;
		gl::state.modelView *= f->modelMat;

	gl::state.calculateNormalMat();
	gl::state.updateMatrices();

	if (f->geo >= 0)
		drawGeometry(f->geo);
//	else {
	if (0) {
		glVertexAttrib4f(in_Color, 1.0f, 1.0f, 1.0f, 1.0f);
		if (f == animRoot)
			glVertexAttrib4f(in_Color, 1.0f, 0.0f, 0.0f, 1.0f);
		glBindTexture(GL_TEXTURE_2D, renderer->whiteTex);
		gl::drawSphere(0.1f, 10, 10);
	}

	if (recurse)
		for (size_t i = 0; i < f->children.size(); i++)
//			drawFrame(f->children[i], true, transform);
			drawFrame(f->children[i], true, true);

	gl::state.modelView = mvSave;
	gl::state.normalMat = nrmSave;
	gl::state.updateMatrices();
}

void Drawable::drawGeometry(int gi)
{
	THREADCHECK();
	if (uint(gi) >= clump->geometryList.size())
		return;

	rw::Geometry &g = clump->geometryList[gi];

	if (geoList[gi].vbo == 0 || geoList[gi].ibo == 0)
		return;

	// sensible defaults
	glVertexAttrib4f(in_Color, 0.0f, 0.0f, 0.0f, 1.0f);
	glVertexAttrib3f(in_Normal, 0.0f, 0.0f, 0.0f);

	uint numVertices = g.vertices.size() / 3;
	glBindBuffer(GL_ARRAY_BUFFER, geoList[gi].vbo);
	if (geoList[gi].dirty) {
		glBufferSubData(GL_ARRAY_BUFFER, 0,
		                geoList[gi].vertices.size()*sizeof(GLfloat),
		                &geoList[gi].vertices[0]);
	}
	glEnableVertexAttribArray(in_Vertex);
	GLint offset = 0;
	glVertexAttribPointer(in_Vertex, 3, GL_FLOAT, GL_FALSE, 0,
			      (GLvoid*) offset);
	offset += numVertices*3*sizeof(GLfloat);
	if (g.flags & rw::FLAGS_PRELIT) {
		if (renderer->doVertexColors) {
			glEnableVertexAttribArray(in_Color);
			glVertexAttribPointer(in_Color, 4, GL_UNSIGNED_BYTE,
					      GL_TRUE, 0, (GLvoid*) offset);
		}
		offset += numVertices*4*sizeof(GLubyte);
	}
	if (g.flags & rw::FLAGS_NORMALS) {
		if (geoList[gi].dirty)
			glBufferSubData(GL_ARRAY_BUFFER, offset,
					geoList[gi].normals.size()*
			                                    sizeof(GLfloat),
					&geoList[gi].normals[0]);
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
	geoList[gi].dirty = false;


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geoList[gi].ibo);
	GLenum mode = (g.faceType == rw::FACETYPE_STRIP) ?
		      GL_TRIANGLE_STRIP : GL_TRIANGLES;
	offset = 0;

	for (size_t j = 0; j < g.splits.size(); j++) {
		rw::Split s = g.splits[j];
		bool isTransparent = false;

		// bind texture
		glActiveTexture(GL_TEXTURE0);
		int matid = s.matIndex;
		string texname = "";
		if (renderer->doTextures && texDict != 0 &&
		    g.materialList[matid].hasTex) {
			texname = g.materialList[matid].texture.name;
			Texture *t = texDict->get(texname);

			if (t != 0 && t->tex != 0) {
				if (t->hasAlpha)
					isTransparent = true;
				glBindTexture(GL_TEXTURE_2D,
				              t->tex);
			} else {
				glBindTexture(GL_TEXTURE_2D,
				              renderer->whiteTex);
			}
		} else {
			glBindTexture(GL_TEXTURE_2D, renderer->whiteTex);
		}
		glm::vec4 matCol;
		matCol.x = float(g.materialList[matid].color[0]) / 255.0f;
		matCol.y = float(g.materialList[matid].color[1]) / 255.0f;
		matCol.z = float(g.materialList[matid].color[2]) / 255.0f;
		matCol.w = float(g.materialList[matid].color[3]) / 255.0f;
		matCol.w *= renderer->globalAlpha;
		state.matColor = matCol;
		state.updateMaterial();

		if (matCol.w != 1.0f)
			isTransparent = true;

		renderer->wasTransparent |= isTransparent;
		if (isTransparent != renderer->drawTransparent) {
			offset += s.indices.size();
			continue;
		}

//		if (isTransparent) {
		if (1) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//			glEnable(GL_ALPHA_TEST);
//			glAlphaFunc(GL_GREATER, 0.5);
//			glAlphaFunc(GL_GREATER, alphaVal);
		} else {
			glDisable(GL_BLEND);
			glDisable(GL_ALPHA_TEST);
		}

		if (renderer->drawWire) {
			glBindTexture(GL_TEXTURE_2D, renderer->whiteTex);
			glDisableVertexAttribArray(in_Color);
			glDisableVertexAttribArray(in_Normal);
			glVertexAttrib4f(in_Color, 0.8f, 0.8f, 0.8f, 1.0f);
			glLineWidth(2);
			int add = (mode == rw::FACETYPE_STRIP) ? 1 : 3;
			for (size_t i = 0; i < s.indices.size()-2; i += add) {
				glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_INT,
					       (GLvoid*)
				               (offset*sizeof(rw::uint32)));
				offset += add;
			}
		} else {
			glDrawElements(mode, s.indices.size(),
				       GL_UNSIGNED_INT,
				       (GLvoid*) (offset*sizeof(rw::uint32)));

//			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			offset += g.splits[j].indices.size();
		}
	}

	glDisableVertexAttribArray(in_Vertex);
	glDisableVertexAttribArray(in_Color);
	glDisableVertexAttribArray(in_Normal);
	glDisableVertexAttribArray(in_TexCoord);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

quat Drawable::getPosition(void)
{
	if (root == 0)
		return quat(0,0,0,0);
	Frame *r = root;
	if (animRoot)
		r = animRoot;
	return quat(r->pos[0], r->pos[1], r->pos[2]);
}

quat Drawable::getMinPosition(void)
{
	quat q = manim.getPosition(0, animRoot->name);
	return q;
}

quat Drawable::getMaxPosition(void)
{
	quat q = manim.getPosition(manim.endTime+1, animRoot->name);
	return q;
}

Frame *Drawable::getFrame(string name)
{
	for (size_t i = 0; i < frmList.size(); i++)
		if (frmList[i]->name == name)
			return frmList[i];
	return 0;
}

void Drawable::resetFrames(void)
{
	for (size_t i = 0; i < frmList.size(); i++) {
		frmList[i]->pos = frmList[i]->defPos;
		frmList[i]->modelMat = frmList[i]->defMat;
	}
	updateFrames(root);
	updateGeometries();
}

bool Drawable::hasModel(void)
{
	return (clump != 0);
}

bool Drawable::hasTextures(void)
{
	return (texDict != 0 && texDict->isHierarchyLoaded());
}

void Drawable::dumpClump(bool detailed)
{
	clump->dump(detailed);
}

Drawable::Drawable(void)
{
	curTime = 0.0;
	curOvrTime = 0.0;
	currentColorStep = 0;
	clump = 0;
	texDict = 0;
	root = 0;
	animRoot = 0;
	overrideAnim = 0;
}

void Drawable::unload(void)
{
	if (clump)
		clump->clear();
	delete clump;
	clump = 0;
	bool gl = false;
	for (size_t i = 0; i < geoList.size(); i++) {
		if (geoList[i].vbo != 0) {
			glDeleteBuffers(1, &geoList[i].vbo);
			gl = true;
		}
		if (geoList[i].ibo != 0) {
			glDeleteBuffers(1, &geoList[i].ibo);
			gl = true;
		}
	}
	// just for debugging
	if (gl)
		THREADCHECK();
	geoList.clear();

	for (size_t i = 0; i < frmList.size(); i++)
		delete frmList[i];
	frmList.clear();
	atomicList.clear();

	if (texDict)
		texMan->release(texDict->fileName);
	texDict = 0;

	// TODO: handle virtual animations
	boneToFrame.clear();
	animRoot = root = 0;
	overrideAnim = 0;
	curTime = 0.0f;
	currentColorStep = 0;
}

Drawable::~Drawable(void)
{
	unload();
}

