#include"includes\h\GL\freeglut.h"
#include<iostream>
#include<cmath>
#include<ctime>
#include<deque>

CONST GLfloat DEF_ASPECT_RATIO = 16.0f / 9.0f;
CONST GLint DEF_WINDOW_WIDTH = 900, DEF_WINDOW_HEIGHT = GLint(DEF_WINDOW_WIDTH / DEF_ASPECT_RATIO);
GLint curr_window_width, curr_window_height;
CONST GLint FPS = 0;
CONST GLfloat PI = acos(-1.0f);
bool key_32_press = false;
GLfloat cubic_rubiks_size = 240;
GLfloat interspace = 4.0f;
GLfloat cubic_size = (cubic_rubiks_size - interspace * 2) / 3;
bool keys_rotation_press[6];
GLfloat cR_speed = 6.0f;
GLfloat cR_z_angle = 0.0f, cR_y_angle = 8.0f, cR_x_angle = 8.0f;
char num_key_press = 0;
GLfloat pR_speed = 12.0f;
std::deque<int> parts_rots_order;//0-8=way-, 9-17=way+
bool rpR_active = false;
GLfloat rpR_boost = 12.0f;

//0=yellow, 1=orange, 2=red, 3=green, 4=blue, 5=white 6=inner, 7=edge
GLubyte default_colors[][3] =
{{255,255,0},
{255,128,0},
{255,0,0},
{0,255,0},
{0,0,255},
{255,255,255},
{8,8,8},
{12,12,12}};

class Cubic
{
private:
	GLfloat vertexes[8][3];
	int facets[6];
public:
	friend void swap_facets(Cubic&, Cubic&);
	Cubic()
	{
		fillFacets();
	}
	Cubic(GLfloat ix, GLfloat iy, GLfloat iz)
	{
		vertexes[0][0] = ix, vertexes[0][1] = iy, vertexes[0][2] = iz;
		for(int j = 1; j < 8; j++)
		{
			vertexes[j][0] = vertexes[0][0] + ((j / 4) % 2 ? cubic_size : 0);
			vertexes[j][1] = vertexes[0][1] + ((j / 2) % 2 ? cubic_size : 0);
			vertexes[j][2] = vertexes[0][2] + (j % 2 ? cubic_size : 0);
		}
		fillFacets();
	}
private:
	void fillFacets()
	{
		for(int j = 0; j < 6; j++)
			facets[j] = 7;
	}
public:
	void setFacetColor(int c_face, int c_color)
	{
		facets[c_face] = c_color;
	}
	void draw()
	{
		//facets drawing
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBegin(GL_QUADS);
		glColor3ubv(default_colors[facets[0]]);
		glVertex3fv(vertexes[0]);
		glVertex3fv(vertexes[2]);
		glVertex3fv(vertexes[6]);
		glVertex3fv(vertexes[4]);
		glColor3ubv(default_colors[facets[1]]);
		glVertex3fv(vertexes[0]);
		glVertex3fv(vertexes[1]);
		glVertex3fv(vertexes[3]);
		glVertex3fv(vertexes[2]);
		glColor3ubv(default_colors[facets[2]]);
		glVertex3fv(vertexes[4]);
		glVertex3fv(vertexes[6]);
		glVertex3fv(vertexes[7]);
		glVertex3fv(vertexes[5]);
		glColor3ubv(default_colors[facets[3]]);
		glVertex3fv(vertexes[0]);
		glVertex3fv(vertexes[4]);
		glVertex3fv(vertexes[5]);
		glVertex3fv(vertexes[1]);
		glColor3ubv(default_colors[facets[4]]);
		glVertex3fv(vertexes[2]);
		glVertex3fv(vertexes[3]);
		glVertex3fv(vertexes[7]);
		glVertex3fv(vertexes[6]);
		glColor3ubv(default_colors[facets[5]]);
		glVertex3fv(vertexes[1]);
		glVertex3fv(vertexes[5]);
		glVertex3fv(vertexes[7]);
		glVertex3fv(vertexes[3]);
		glEnd();
		//edges drawing
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(2.0f);
		glColor3ubv(default_colors[6]);
		glBegin(GL_QUADS);
		glVertex3fv(vertexes[0]);
		glVertex3fv(vertexes[2]);
		glVertex3fv(vertexes[6]);
		glVertex3fv(vertexes[4]);
		glVertex3fv(vertexes[0]);
		glVertex3fv(vertexes[1]);
		glVertex3fv(vertexes[3]);
		glVertex3fv(vertexes[2]);
		glVertex3fv(vertexes[4]);
		glVertex3fv(vertexes[6]);
		glVertex3fv(vertexes[7]);
		glVertex3fv(vertexes[5]);
		glVertex3fv(vertexes[0]);
		glVertex3fv(vertexes[4]);
		glVertex3fv(vertexes[5]);
		glVertex3fv(vertexes[1]);
		glVertex3fv(vertexes[2]);
		glVertex3fv(vertexes[3]);
		glVertex3fv(vertexes[7]);
		glVertex3fv(vertexes[6]);
		glVertex3fv(vertexes[1]);
		glVertex3fv(vertexes[5]);
		glVertex3fv(vertexes[7]);
		glVertex3fv(vertexes[3]);
		glEnd();
	}
	//0=xoy, 1=xoz, 2=yoz, swap(1,2)
	void rotate(int vector, bool way)
	{
		int f_order[][8] =
		{{1,4,2,3},
		{2,0,1,5},
		{3,5,4,0}};
		if(way)
			std::reverse(&(f_order[vector][0]), &(f_order[vector][4]));
		for(int j = 0; j < 3; j++)
			std::swap(facets[f_order[vector][j]], facets[f_order[vector][j + 1]]);
	}
};

class RubiksCubic
{
private:
public:
	Cubic cubics[3][3][3];
	GLfloat rotation[9];//0-2=xoy, 3-5=xoz, 6-8=yoz
public:
	RubiksCubic()
	{
		GLfloat x0 = -cubic_rubiks_size / 2, y0 = -cubic_rubiks_size / 2, z0 = -cubic_rubiks_size / 2;
		GLfloat curr_x = x0;
		for(int j = 0; j < 3; j++)
		{
			GLfloat curr_y = y0;
			for(int i = 0; i < 3; i++)
			{
				GLfloat curr_z = z0;
				for(int k = 0; k < 3; k++)
				{
					cubics[j][i][k] = {curr_x,curr_y,curr_z};
					curr_z += cubic_size + interspace;
				}
				curr_y += cubic_size + interspace;
			}
			curr_x += cubic_size + interspace;
		}
		for(int j = 0; j < 6; j++)
			setFacetColor(j, j);
	}
	void setFacetColor(int c_face, int c_color)
	{
		int x = 0, y = 0, z = 0, *it1 = 0, *it2 = 0;
		switch(c_face)
		{
			case 0:
				it1 = &x;
				it2 = &y;
				break;
			case 1:
				it1 = &y;
				it2 = &z;
				break;
			case 2:
				x = 2;
				it1 = &y;
				it2 = &z;
				break;
			case 3:
				it1 = &x;
				it2 = &z;
				break;
			case 4:
				it1 = &x;
				y = 2;
				it2 = &z;
				break;
			case 5:
				it1 = &x;
				it2 = &y;
				z = 2;
				break;
		}
		while(*it1 < 3)
		{
			while(*it2 < 3)
			{
				cubics[x][y][z].setFacetColor(c_face, c_color);
				++*it2;
			}
			*it2 -= 3;
			++*it1;
		}
	}
	void draw()
	{
		for(int j = 0; j < 3; j++)
			for(int i = 0; i < 3; i++)
				for(int k = 0; k < 3; k++)
				{
					glPushMatrix();
					glRotatef(-rotation[j + 6], 1, 0, 0);
					glRotatef(rotation[i + 3], 0, 1, 0);
					glRotatef(-rotation[k], 0, 0, 1);
					cubics[j][i][k].draw();
					glPopMatrix();
				}
	}
	//0-2=xoy, 3-5=xoz, 6-8=yoz
	void rotatePart(int part, bool way)
	{
		int x, y, z, *it1 = 0, *it2 = 0;
		switch(part / 3)
		{
			case 0:
				it1 = &x;
				it2 = &y;
				z = part % 3;
				break;
			case 1:
				it1 = &x;
				y = part % 3;
				it2 = &z;
				break;
			case 2:
				x = part % 3;
				it1 = &y;
				it2 = &z;
				break;
		}
		*it1 = 0, *it2 = 0;
		if(way)
			std::swap(it1, it2);
		Cubic buffer[2];
		while(*it1 != 2)
		{
			swap_facets(buffer[*it1], cubics[x][y][z]);
			buffer[*it1].rotate(part / 3, way);
			*it1 += 1;
		}
		while(*it2 != 2)
		{
			swap_facets(buffer[*it2], cubics[x][y][z]);
			buffer[*it2].rotate(part / 3, way);
			*it2 += 1;
		}
		while(*it1 != 0)
		{
			swap_facets(buffer[2 - *it1], cubics[x][y][z]);
			buffer[2 - *it1].rotate(part / 3, way);
			*it1 += -1;
		}
		while(*it2 != 0)
		{
			swap_facets(buffer[2 - *it2], cubics[x][y][z]);
			buffer[2 - *it2].rotate(part / 3, way);
			*it2 += -1;
		}
		while(*it1 != 2)
		{
			swap_facets(buffer[*it1], cubics[x][y][z]);
			*it1 += 1;
		}
		*it1 = 1, *it2 = 1;
		cubics[x][y][z].rotate(part / 3, way);
	}
	//0-2=xoy, 3-5=xoz, 6-8=yoz
	void addPartRotation(int part, GLfloat degree)
	{
		rotation[part] += degree;
		if(rotation[part] < -45.0f)
		{
			rotation[part] += 90.0f;
			rotatePart(part, false);
		}
		else if(rotation[part] > 45.0f)
		{
			rotation[part] -= 90.0f;
			rotatePart(part, true);
		}
	}
} rubiks_cubic;

GLfloat convertDegToRad(GLfloat degrees)
{
	return degrees * PI / 180.0f;
}

void swap_facets(Cubic &cub1, Cubic &cub2)
{
	std::swap(cub1.facets, cub2.facets);
}

int getDegSector(GLfloat &deg)
{
	if(deg < 45.0f)
		return 0;
	if(deg < 135.0f)
		return 1;
	if(deg < 225.0f)
		return 2;
	if(deg < 315.0f)
		return 3;
	return 0;
}

void onDisplay(void)
{
	//background drawing
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -cubic_rubiks_size * 2.5f);
	glDisable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_POLYGON);
	{
		glColor3ub(255, 255, 255);
		glVertex2f(0.0f, 0.0f);
		glColor3ub(0, 0, 0);
		for(GLfloat degrees = 0.0f, theta; degrees <= 360.0f; degrees += 6.0f)
		{
			theta = convertDegToRad(degrees);
			glVertex2f(DEF_WINDOW_WIDTH * cos(theta) + 0.0f, DEF_WINDOW_HEIGHT * sin(theta) + 0.0f);
		}
	}
	glEnd();
	glEnable(GL_DEPTH_TEST);
	//cubics drawing
	glRotatef(cR_y_angle, 0, 1, 0);
	glRotatef(cR_x_angle, 1, 0, 0);
	glRotatef(cR_z_angle, 0, 0, 1);
	rubiks_cubic.draw();
	glPopMatrix();
	glutSwapBuffers();
}

void onReshape(GLsizei w, GLsizei h)
{
	if(h == 0)
		h = 1;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//GLfloat aspectRatio = (GLfloat)w / (GLfloat)h / DEF_ASPECT_RATIO;;
	//if(aspectRatio <= 1)
	//{
	//	curr_window_height = GLint(DEF_WINDOW_HEIGHT / aspectRatio);
	//	glOrtho(-DEF_WINDOW_WIDTH / 2, DEF_WINDOW_WIDTH / 2, -curr_window_height / 2, curr_window_height / 2, -cubic_rubiks_size, cubic_rubiks_size);
	//}
	//else
	//{
	//	curr_window_width = GLint(DEF_WINDOW_WIDTH * aspectRatio);
	//	glOrtho(-curr_window_width / 2, curr_window_width / 2, -DEF_WINDOW_HEIGHT / 2, DEF_WINDOW_HEIGHT / 2, -cubic_rubiks_size, cubic_rubiks_size);
	//}
	gluPerspective(45, (GLfloat)w / (GLfloat)h, 0.1f, cubic_rubiks_size * 4);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void RedisplayTimer(int timer)
{
	glutTimerFunc(timer, RedisplayTimer, timer);
	glutPostRedisplay();
}

void RotationsTimer(int timer)
{
	glutTimerFunc(timer, RotationsTimer, timer);
	//implementation of complete rotations control
	int xOy_initiation = keys_rotation_press[1] - keys_rotation_press[0];
	int xOz_initiation = keys_rotation_press[3] - keys_rotation_press[2];
	int yOz_initiation = keys_rotation_press[5] - keys_rotation_press[4];
	if(xOy_initiation || xOz_initiation || yOz_initiation)
	{
		//complete rotations control axis normalization
		GLfloat *xOy_ptr = &cR_z_angle, *yOz_ptr = &cR_x_angle,
			xoy_flush = cR_speed, xoz_flush = cR_speed, yoz_flush = cR_speed;
		switch(getDegSector(cR_y_angle))
		{
			case 1:
				xoy_flush *= -1;
				std::swap(xOy_ptr, yOz_ptr);
				break;
			case 2:
				xoy_flush *= -1;
				yoz_flush *= -1;
				break;
			case 3:
				std::swap(xOy_ptr, yOz_ptr);
				yoz_flush *= -1;
				break;
		}
		switch(getDegSector(*yOz_ptr))
		{
			case 2:
				xoy_flush *= -1;
				break;
		}
		//complete rotations computing
		if(xOy_initiation)
		{
			*xOy_ptr += xoy_flush * (keys_rotation_press[1] - keys_rotation_press[0]);
			if(*xOy_ptr > 360.0f)
				*xOy_ptr -= 360.0f;
			if(*xOy_ptr < 0.0f)
				*xOy_ptr += 360.0f;
		}
		if(xOz_initiation)
		{
			cR_y_angle -= xoz_flush * (keys_rotation_press[3] - keys_rotation_press[2]);
			if(cR_y_angle > 360.0f)
				cR_y_angle -= 360.0f;
			if(cR_y_angle < 0.0f)
				cR_y_angle += 360.0f;
		}
		if(yOz_initiation)
		{
			*yOz_ptr -= yoz_flush * (keys_rotation_press[5] - keys_rotation_press[4]);
			if(*yOz_ptr > 360.0f)
				*yOz_ptr -= 360.0f;
			if(*yOz_ptr < 0.0f)
				*yOz_ptr += 360.0f;
		}
	}
	//implementation of parts rotations
	if(rpR_active && parts_rots_order.size() < 2)
		parts_rots_order.push_back(rand() % 18);
	static int current_part = -1;
	if(parts_rots_order.size() || current_part != -1)
	{
		static bool current_way;
		static GLfloat curr_part_rot_deg = 0;
		if(current_part == -1)
		{
			std::cout << getDegSector(cR_y_angle) << ' '
				<< getDegSector(cR_x_angle) << ' '
				<< getDegSector(cR_z_angle) << '\n';//debug
			//new part rotation axis normalization
			//xOz normalization
			std::cout << parts_rots_order.front() << '\n';//debug
			int new_rot_dimension = parts_rots_order.front() / 3,
				side_reflect_term = (parts_rots_order.front() % 3 - 1) * -2;
			switch(getDegSector(cR_y_angle))
			{
				case 1:
					switch(new_rot_dimension)
					{
						case 0:
							parts_rots_order.front() += 15 + side_reflect_term;
							break;
						case 3:
							parts_rots_order.front() += -3 + side_reflect_term;
							break;
						case 2:
						case 5:
							parts_rots_order.front() += -6;
							break;
					}
					break;
				case 2:
					switch(new_rot_dimension)
					{
						case 0:
						case 2:
							parts_rots_order.front() += 9 + side_reflect_term;
							break;
						case 3:
						case 5:
							parts_rots_order.front() += -9 + side_reflect_term;
							break;
					}
					break;
				case 3:
					switch(new_rot_dimension)
					{
						case 0:
							parts_rots_order.front() += 6;
							break;
						case 2:
							parts_rots_order.front() += 3 + side_reflect_term;
							break;
						case 3:
							parts_rots_order.front() += 6;
							break;
						case 5:
							parts_rots_order.front() += -15 + side_reflect_term;
							break;
					}
					break;
			}
	//yOz normalization
	std::cout << parts_rots_order.front() << '\n';//debug
	new_rot_dimension = parts_rots_order.front() / 3;
	side_reflect_term = (parts_rots_order.front() % 3 - 1) * -2;
	switch(getDegSector(cR_x_angle))
	{
		case 1:
			switch(new_rot_dimension)
			{
				case 0:
					parts_rots_order.front() += 12;
					break;
				case 3:
					parts_rots_order.front() += -6;
					break;
				case 1:
				case 4:
					parts_rots_order.front() += -3 + side_reflect_term;
					break;
			}
			break;
		case 2:
			switch(new_rot_dimension)
			{
				case 0:
				case 1:
					parts_rots_order.front() += 9 + side_reflect_term;
					break;
				case 3:
				case 4:
					parts_rots_order.front() += -9 + side_reflect_term;
					break;
			}
			break;
		case 3:
			switch(new_rot_dimension)
			{
				case 0:
					parts_rots_order.front() += 3 + side_reflect_term;
					break;
				case 1:
					parts_rots_order.front() += 6;
					break;
				case 3:
					parts_rots_order.front() += 3 + side_reflect_term;
					break;
				case 4:
					parts_rots_order.front() += -12;
					break;
			}
			break;
	}
	//xOy normalization
	std::cout << parts_rots_order.front() << '\n';//debug
	new_rot_dimension = parts_rots_order.front() / 3;
	side_reflect_term = (parts_rots_order.front() % 3 - 1) * -2;
	switch(getDegSector(cR_z_angle))
	{
		case 1:
			switch(new_rot_dimension)
			{
				case 1:
					parts_rots_order.front() += 12;
					break;
				case 2:
					parts_rots_order.front() += -3 + side_reflect_term;
					break;
				case 4:
					parts_rots_order.front() += -6;
					break;
				case 5:
					parts_rots_order.front() += -3 + side_reflect_term;
					break;
			}
			break;
		case 2:
			switch(new_rot_dimension)
			{
				case 1:
				case 2:
					parts_rots_order.front() += 9 + side_reflect_term;
					break;
				case 4:
				case 5:
					parts_rots_order.front() += -9 + side_reflect_term;
					break;
			}
			break;
		case 3:
			switch(new_rot_dimension)
			{
				case 1:
					parts_rots_order.front() += 3 + side_reflect_term;
					break;
				case 2:
					parts_rots_order.front() += 6;
					break;
				case 4:
					parts_rots_order.front() += 3 + side_reflect_term;
					break;
				case 5:
					parts_rots_order.front() += -12;
					break;
			}
			break;
	}
			//new part rotation launching
			std::cout << parts_rots_order.front() << '\n';//debug
			current_part = parts_rots_order.front() % 9;
			current_way = parts_rots_order.front() / 9 ? 0 : 1;
			parts_rots_order.pop_front();

		}
		else
		{
			//part rotation computing
			if(90.0f - curr_part_rot_deg < pR_speed)
			{
				rubiks_cubic.addPartRotation(current_part, (90.0f - curr_part_rot_deg) * (current_way ? 1 : -1));
				curr_part_rot_deg = 0.0f;
				current_part = -1;
			}
			else
			{
				rubiks_cubic.addPartRotation(current_part, pR_speed * (current_way ? 1 : -1));
				curr_part_rot_deg += abs(pR_speed);
			}
		}
	}
}

void procNumKey(unsigned char key)
{
	int rot_ways_list[][4] =
	{{3,7,3,6},
	{8,0,7,0},
	{1,9,12,8},
	{6,0,4,0},
	{0,0,0,0},
	{4,0,13,0},
	{1,9,15,5},
	{2,0,16,0},
	{3,7,17,14}};
	if(rot_ways_list[num_key_press - '1'][0] == key - '0')
		parts_rots_order.push_back(rot_ways_list[num_key_press - '1'][2]);
	else if(rot_ways_list[num_key_press - 49][1] == key - '0')
		parts_rots_order.push_back(rot_ways_list[num_key_press - '1'][3]);
}

void onKeyPressed(unsigned char key, int, int)
{
	if(key != num_key_press && key >= '1' && key <= '9')
	{
		if(!num_key_press)
			num_key_press = key;
		else if(!parts_rots_order.size())
			procNumKey(key);
	}
	switch(key)
	{
		case 27://esc
			glutLeaveMainLoop();
			break;
		case 32://space
			if(!key_32_press)
			{
				key_32_press = true;
				glutFullScreenToggle();
			}
			break;
		case 97://a
			keys_rotation_press[2] = true;
			break;
		case 100://d
			keys_rotation_press[3] = true;
			break;
		case 101://e
			keys_rotation_press[0] = true;
			break;
		case 113://q
			keys_rotation_press[1] = true;
			break;
		case 114://r
			if(!rpR_active)
			{
				pR_speed += rpR_boost;
				rpR_active = true;
			}
			else
			{
				rpR_active = false;
				pR_speed -= rpR_boost;
			}
			break;
		case 115://s
			keys_rotation_press[5] = true;
			break;
		case 119://w
			keys_rotation_press[4] = true;
			break;
		default://debug
			std::cout << "~KP: " << (int)key << " '" << key << "'\n";
			break;
	}
}

void onKeyReleased(unsigned char key, int, int)
{
	if(key == num_key_press)
		num_key_press = 0;
	switch(key)
	{
		case 32://space
			key_32_press = false;
			break;
		case 97://a
			keys_rotation_press[2] = false;
			break;
		case 100://d
			keys_rotation_press[3] = false;
			break;
		case 101://e
			keys_rotation_press[0] = false;
			break;
		case 113://q
			keys_rotation_press[1] = false;
			break;
		case 115://s
			keys_rotation_press[5] = false;
			break;
		case 119://w
			keys_rotation_press[4] = false;
			break;
	}
}

void onSpecialKeyPressed(int key, int, int)
{
	switch(key)
	{
		case 100://arrow left
			keys_rotation_press[2] = true;
			break;
		case 101://arrow up
			keys_rotation_press[4] = true;
			break;
		case 102://arrow right
			keys_rotation_press[3] = true;
			break;
		case 103://arrow down
			keys_rotation_press[5] = true;
			break;
		default://debug
			std::cout << "~SKP: " << key << '\n';
			break;
	}
}

void onSpecialKeyReleased(int key, int, int)
{
	switch(key)
	{
		case 100://arrow left
			keys_rotation_press[2] = false;
			break;
		case 101://arrow up
			keys_rotation_press[4] = false;
			break;
		case 102://arrow right
			keys_rotation_press[3] = false;
			break;
		case 103://arrow down
			keys_rotation_press[5] = false;
			break;
	}
}

int main(int argc, char**argv)
{
	srand((unsigned)time(0));
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB | GLUT_MULTISAMPLE);
	glutInitWindowSize(DEF_WINDOW_WIDTH, DEF_WINDOW_HEIGHT);
	glutCreateWindow("Rubik's Cubic");
	glDepthFunc(GL_LEQUAL);
	glEnable(GLUT_MULTISAMPLE);
	glutSetOption(GLUT_MULTISAMPLE, 8);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glutDisplayFunc(onDisplay);
	glutReshapeFunc(onReshape);
	glutKeyboardFunc(onKeyPressed);
	glutKeyboardUpFunc(onKeyReleased);
	glutSpecialFunc(onSpecialKeyPressed);
	glutSpecialUpFunc(onSpecialKeyReleased);
	glutTimerFunc((unsigned int)round(FPS == 0 ? 0 : 1000.0f / FPS), RedisplayTimer, (int)round(FPS == 0 ? 0 : 1000.0f / FPS));
	glutTimerFunc(25, RotationsTimer, 25);
	glutMainLoop();
	return 0;
}
