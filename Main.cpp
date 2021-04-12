#include <SFML/Graphics.hpp>
#include <random>
#include <chrono>

using namespace sf;


int main() {
	auto start_time = std::chrono::steady_clock::now();

	Shader shader;

	int width = 720;
	int height = 480;

	if (!shader.loadFromFile("OutputShader.frag", Shader::Fragment)) {
		printf("can't load shader");
		return 1;
	}

	shader.setUniform("resolution", Vector2f(width, height));
	shader.setUniform("u_seed1", Vector2f(rand(), rand()*999));
	shader.setUniform("u_seed2", Vector2f(rand(), rand()*999));

	RenderTexture image;
	image.create(width, height);

	Sprite drawable(image.getTexture());

	image.draw(drawable, &shader);
	image.display();

	image.getTexture().copyToImage().saveToFile("output.png");

	printf("%i", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time));

	return 0;
}