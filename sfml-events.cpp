//g++ sfml-events.cpp -o sfml-events --std=c++14 -lsfml-graphics -lsfml-window -lsfml-system
#include <SFML/Graphics.hpp>
#include <iostream>

int main()
{
    sf::RenderWindow window(sf::VideoMode(200, 200), "SFML works!");
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
		    else if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Escape) {
					std::cout << "the escape key was pressed" << std::endl;
					std::cout << "control:" << event.key.control << std::endl;
					std::cout << "alt:" << event.key.alt << std::endl;
					std::cout << "shift:" << event.key.shift << std::endl;
					std::cout << "system:" << event.key.system << std::endl;
				}
			}
			else if (event.type == sf::Event::MouseWheelMoved)
			{
				std::cout << "wheel movement: " << event.mouseWheel.delta << std::endl;
				std::cout << "mouse x: " << event.mouseWheel.x << std::endl;
				std::cout << "mouse y: " << event.mouseWheel.y << std::endl;
			}
			else if (event.type == sf::Event::KeyReleased)
				std::cout << "key released" << std::endl;
			else if (event.type == sf::Event::MouseButtonPressed)
			{
				if (event.mouseButton.button == sf::Mouse::Right)
				{
					std::cout << "the right button was pressed" << std::endl;
					std::cout << "mouse x: " << event.mouseButton.x << std::endl;
					std::cout << "mouse y: " << event.mouseButton.y << std::endl;
				}
			}
			else if (event.type == sf::Event::MouseMoved)
			{
				std::cout << "new mouse x: " << event.mouseMove.x << std::endl;
				std::cout << "new mouse y: " << event.mouseMove.y << std::endl;
			}
			else if (event.type == sf::Event::MouseEntered)
				std::cout << "the mouse cursor has entered the window" << std::endl;
			else if (event.type == sf::Event::MouseLeft)
				std::cout << "the mouse cursor has left the window" << std::endl;
        }

        window.clear();
        window.draw(shape);
        window.display();
    }

    return 0;
}
