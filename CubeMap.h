#ifndef	CUBE_MAP_H
#define CUBE_MAP_H

#include "ofMain.h"

class CubeMap
{
public:

	unsigned int textureObject;

    // loads images
	void loadImages(string pos_x, string pos_y, string pos_z, string neg_x, string neg_y, string neg_z)
	{
		ofImage images[6];
		bool loaded1 = images[0].loadImage(pos_x);
		bool loaded2 = images[1].loadImage(neg_x);
		bool loaded3 = images[2].loadImage(pos_y);
		bool loaded4 = images[3].loadImage(neg_y);
		bool loaded5 = images[4].loadImage(pos_z);
		bool loaded6 = images[5].loadImage(neg_z);


		loadFromOfImages(images[0],
			images[2],
			images[4],
			images[1],
			images[3],
			images[5]);

	}

	void loadFromOfImages(ofImage pos_x, ofImage pos_y, ofImage pos_z, ofImage neg_x, ofImage neg_y, ofImage neg_z);

	void bindMulti(int pos)
	{
		glActiveTexture(GL_TEXTURE0 + pos);
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, textureObject);
	}

	void bind()
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, textureObject);
	}

	void unbind()
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, 0);
	}

	void enableFixedMapping()
	{
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP);
		glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_GEN_R);
		glEnable(GL_TEXTURE_CUBE_MAP);
	}

	void disableFixedMapping()
	{
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_GEN_R);
		glDisable(GL_TEXTURE_CUBE_MAP);
	}
};

#endif