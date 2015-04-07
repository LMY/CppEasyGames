//g++ $filename.cpp -o $filename -std=c++14 -lsfml-graphics -lsfml-window -lsfml-system
#include <SFML/Graphics.hpp>
#include <iostream>
#include <thread>

const char* FONT_FILENAME = "/usr/share/fonts/TTF/DejaVuSans.ttf";
const unsigned int FPS = 30;	// fps, -1:vsync
const int size = 20;			// how many quads
const int QUADSIZE = 30;
const int SPACING = 10;
const int SLEEP_MSEC = 1000;	// when pc moves, sleep
const int WIDTH = 10*(QUADSIZE+SPACING);
const int HEIGHT = 2*10*(QUADSIZE+SPACING);

enum class Player : int { A=0, B=1 };

class Level
{
	static auto initial(int playern, int howmany=8) {
		auto qc = std::vector<Player> {};
		for (int i=0; i<howmany; i++)
			qc.push_back(playern==0 ? Player::A : Player::B);
		return qc;
	}

	std::default_random_engine rd;
    std::mt19937 gen;

	std::vector<std::vector<Player>> quads;
	int ptA, ptB;

	sf::Font font;
	
	void drawPts(Player p, sf::RenderWindow& window) const {
		sf::Text pts(std::to_string(p==Player::A ? ptA : ptB), font);
		pts.setPosition(p==Player::A ? 0 : (size/2-1)*(QUADSIZE+SPACING), 0);
		pts.setColor(p==Player::A ? sf::Color::Blue : sf::Color::Red);
		window.draw(pts);
	}
	
public:
	Level() : rd(), gen(rd()), quads(), ptA(0), ptB(0), font() {

		if (!font.loadFromFile(FONT_FILENAME))
			std::cout << "error loading font" << std::endl;
		
		for (int x=0; x<size; ++x)
			if (x == 0)
				quads.push_back(Level::initial(0));
			else if (size>2 && x == size/2-1)
				quads.push_back(Level::initial(1));
			else
				quads.push_back(std::vector<Player>());
	}

	int game_end() const {
		if (ptA >= 5)
			return 1;
		if (ptB >= 5)
			return 2;
		return 0;
	}

	bool checkmove(int x, Player current) const {
		const auto& content = quads.at(x);
		return !(content.size() <= 0 || current != content.back());
	}

	bool move(int x, Player current, bool apply=true) {
		if (!checkmove(x, current))
			return false;
		
		auto& content = quads.at(x);
		const auto content_size = content.size();

		if (apply) {
			const auto dx = int { current==Player::A ? +1 : -1 };
			auto next = x;

			for (auto iter = content.begin() + (content_size > 5 ? content_size-5 : 0); iter != content.end(); ++iter) {
				const auto cur = *iter;
				next += dx;
				while (next < 0 || next >= size || quads.at(next).size() >= 5) {
					if (next < 0)
						next += size;
					else if (next >= size)
						next -= size;
					else
						next += dx;
				}

				if (cur == Player::A && (next==size-1 || (next < x && dx == +1) ))
					ptA++;
				else if (cur == Player::B && (next==size/2 || (x > size/2 && next <= size/2 && dx == -1) ))
					ptB++;
				else
					quads.at(next).push_back(cur);
			}

			content.resize(content_size > 5 ? content_size-5 : 0);
		}

		return true;
	}

	auto getPossibleMoves(Player current) const {
		auto retv = std::vector<int> {};

		for (int x=0; x<size; ++x)
			if (checkmove(x, current))
				retv.push_back(x);

		return retv;
	}

	bool areTherePossibleMoves(Player current) const { return getPossibleMoves(current).size() > 0; }

	bool move_ai(Player current) {
		const auto moves = getPossibleMoves(current);
		if (moves.size() == 0)
			return false;

		std::uniform_int_distribution<> dis(0, moves.size()-1);
		const auto selected = moves.at(dis(gen));
		return move(selected, current);
	}
	
	void draw(sf::RenderWindow& window) const {
		static sf::RectangleShape rectangle(sf::Vector2f(QUADSIZE, QUADSIZE)); // allocate once

		for (int y=0; y<2; y++)
			for (int posx=0; posx<size/2; posx++) {
				auto content = quads.at((size/2)*y+posx);
				const auto x = int { y==0?posx:size/2-1-posx };

				auto cnt = int { 0 };
				for (auto& c : content) {
					rectangle.setPosition(SPACING/2 + x*(QUADSIZE+SPACING), HEIGHT/2 -QUADSIZE -QUADSIZE/2 + y*(QUADSIZE+SPACING)+(y==0?-1:+1)*(QUADSIZE+SPACING)*cnt);
					rectangle.setFillColor(c==Player::A ? sf::Color::Blue : sf::Color::Red);
					window.draw(rectangle);
					++cnt;
				}

				if (cnt == 0) { // content.empty()
					rectangle.setPosition(SPACING/2 + x*(QUADSIZE+SPACING), HEIGHT/2 - QUADSIZE - QUADSIZE/2 + y*(QUADSIZE+SPACING)+(y==0?-1:+1)*(QUADSIZE+SPACING)*cnt);
					rectangle.setFillColor(y==1 && x==0 ? sf::Color(128,128,255) : y==1 && x==size/2-1 ? sf::Color(255,128,128) :   sf::Color::White);
					window.draw(rectangle);
				}
			}

		drawPts(Player::A, window);
		drawPts(Player::B, window);
	}
};

class Match
{
	Level lvl;
	Player current;
	std::vector<bool> human;

	void switchPlayer() { current = current == Player::A ? Player::B : Player::A; }
	
public:
	Match(bool human0, bool human1) : lvl(), current(Player::A), human({human0, human1}) {}

	static sf::Vector2i coordScreenToGame(sf::Vector2i x) {
		auto ix = int { (x.x - SPACING/2)/(QUADSIZE+SPACING) };
		const auto iy = int { (x.y - (HEIGHT/2 - SPACING/2) + QUADSIZE)/QUADSIZE };

		if (iy == 1 && ix >= 0 && ix <= (size/2-1))
			ix = (size/2-1)-ix;

		return sf::Vector2i(ix>=0&&ix<=(size/2-1)? ix : -1, iy>=0&&iy<=2? iy : -1);
	}

	int play() {
		sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Pyramidais");
		if (FPS > 0)
			window.setFramerateLimit(FPS);
		else
			window.setVerticalSyncEnabled(true);

		while (window.isOpen()) {

			// read input
			auto newmove = int { -1 };
			sf::Event event;
		    while (window.pollEvent(event))
		        if (event.type == sf::Event::Closed)
				    window.close();
				else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
					const auto tile = coordScreenToGame(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
					if (tile.x >= 0 && tile.y >= 0 && tile.y <=1 && tile.x <= (size/2-1))
						newmove = size/2*tile.y+tile.x;
				}

			// update
			if (lvl.game_end() != 0) {
				const auto pwon = lvl.game_end();
				std::cout << std::string("Player ") + std::to_string(pwon) + std::string(" won.") << std::endl;
				return pwon;
			}

			if (!lvl.areTherePossibleMoves(current))
				switchPlayer();

			if (human.at(static_cast<int>(current))) {
				if (newmove >= 0 && lvl.move(newmove, current))
					switchPlayer();
			}
			else {
				lvl.move_ai(current);    // if (lvl.move_ai(current))
				std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MSEC));
				switchPlayer();
			}

			// draw
			window.clear();
			lvl.draw(window);
			window.display();
		}

		return -1;
	}	
};

int main(int argc, char* argv[])
{
	Match match(argc>1 && (std::string(argv[1]).compare("1")==0 || std::string(argv[1]).compare("2")==0), argc>1 && std::string(argv[1]).compare("2")==0);
	match.play();
}
