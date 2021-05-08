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

	object objects[10] = {
		
	};

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
	earth.loadFromFile("textures/earth_2.jpg");
	earth.setRepeated(true);
	shader.setUniform("tex1", earth);

	Texture sky;
	sky.loadFromFile("textures/skybox.png");
	shader.setUniform("sky", sky);


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