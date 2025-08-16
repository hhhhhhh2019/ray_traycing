#include <SFML/Graphics.hpp>
#include <random>

using namespace sf;


struct object {
	int type;
	Vector3f pos;
	Vector3f col;
	float refl;
	bool use_tex;
	int tex_index;
	Vector3f norm; // for plane
	float height; // for plane
};


int main() {
	Shader shader;

	int width = 1024;
	int height = 600;

	RenderWindow window(VideoMode(width, height), "Ray tracing", Style::Titlebar | Style::Close);
	window.setFramerateLimit(60);
	window.setMouseCursorVisible(false);

	RenderTexture image;
	image.create(width, height);
	Sprite drawable(image.getTexture());
	Sprite drawable_flip(image.getTexture());
	drawable_flip.setScale(1, -1);
	drawable_flip.setPosition(0, height);

	Clock clock;

	if (!shader.loadFromFile("OutputShader.frag", Shader::Fragment)) {
		printf("can't load shader");
		return 1;
	}

	Vector3f origin(0, 1, 8);
	Vector3f vel;
	Vector2f rotate(0, 0);


	Texture earth;
	earth.loadFromFile("textures/earth_1.jpg");
	shader.setUniform("tex1", earth);

	Texture normal;
	normal.loadFromFile("textures/normal2.png");
	shader.setUniform("normal", normal);

	Texture a;
	a.loadFromFile("textures/a.jpg");
	a.setRepeated(true);
	shader.setUniform("side_a", a);
	Texture b;
	b.loadFromFile("textures/b.jpg");
	b.setRepeated(true);
	shader.setUniform("side_b", b);
	Texture c;
	c.loadFromFile("textures/c.jpg");
	c.setRepeated(true);
	shader.setUniform("side_c", c);
	Texture d;
	d.loadFromFile("textures/d.jpg");
	d.setRepeated(true);
	shader.setUniform("side_d", d);
	Texture e;
	e.loadFromFile("textures/e.jpg");
	e.setRepeated(true);
	shader.setUniform("side_e", e);
	Texture f;
	f.loadFromFile("textures/f.jpg");
	f.setRepeated(true);
	shader.setUniform("side_f", f);


	shader.setUniform("resolution", Vector2f(width, height));

	sf::Mouse::setPosition(sf::Vector2i(width / 2, height / 2), window);

	float rate = 1.f;

	while (window.isOpen()) {
		rate = 0.05f;

		shader.setUniform("u_seed1", Vector2f(rand(), rand()*999));
		shader.setUniform("u_seed2", Vector2f(rand(), rand()*999));

		Event event;
		while (window.pollEvent(event)) {
			if (event.type == Event::Closed) {
				window.close();
			}
			else if (event.type == Event::MouseMoved) {
				float mx = event.mouseMove.x - width / 2;
				float my = event.mouseMove.y - height / 2;

				rotate.x += my / 20;
				rotate.y -= mx / 20;

				sf::Mouse::setPosition(sf::Vector2i(width / 2, height / 2), window);

				if (mx != 0.f || my != 0.f) rate = 1.f;
			}
			else if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Escape) {
					window.setMouseCursorVisible(true);
					window.close();
				}
				else if (event.key.code == Keyboard::W){rate = 1.f; vel.z = 0.1;}
				else if (event.key.code == Keyboard::A){rate = 1.f; vel.x = -0.1;}
				else if (event.key.code == Keyboard::S){rate = 1.f; vel.z = -0.1;}
				else if (event.key.code == Keyboard::D){rate = 1.f; vel.x = 0.1;}
			}
			else if (event.type == Event::KeyReleased) {
				if (event.key.code == Keyboard::W){rate = 1.f; vel.z = 0;}
				else if (event.key.code == Keyboard::A){rate = 1.f; vel.x = 0;}
				else if (event.key.code == Keyboard::S){rate = 1.f; vel.z = 0;}
				else if (event.key.code == Keyboard::D){rate = 1.f; vel.x = 0;}
			}
		}

		Vector3f nvel;

		nvel.x = vel.x;
		nvel.y = vel.y * cos(rotate.x) - vel.z * sin(rotate.x);
		nvel.z = vel.y * sin(rotate.x) + vel.z * cos(rotate.x);

		nvel.x = vel.x * cos(rotate.y) - vel.z * sin(rotate.y);
		nvel.y = nvel.y;
		nvel.z = vel.x * sin(rotate.y) + vel.z * cos(rotate.y);

		origin += nvel;

		shader.setUniform("origin", origin);
		shader.setUniform("rotate", rotate);

		shader.setUniform("backbuffer", image.getTexture());
		shader.setUniform("rate", rate);

		image.draw(drawable, &shader);
		
        window.draw(drawable_flip);

        window.display();
	}

	return 0;
}