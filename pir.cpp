#include <vector>
#include <iostream>
#include <random>
#include <algorithm>

enum class Player : int { A=0, B=1 };

class Level
{
	static std::vector<Player> initial(int playern, int howmany=8)
	{
		std::vector<Player> qc;
		for (int i=0; i<howmany; i++)
			qc.push_back(playern==0 ? Player::A : Player::B);
		return qc;
	}
	
	std::default_random_engine rd;
    std::mt19937 gen;

	std::vector<std::vector<Player>> quads;
	int size;
	int ptA, ptB;
	
public:
	Level() : rd(), gen(rd()), quads(), size(20), ptA(0), ptB(0) {
		for (int x=0; x<size; ++x)
			if (x == 0)
				quads.push_back(Level::initial(0));
			else if (size>2 && x == size/2-1)
				quads.push_back(Level::initial(1));
			else
				quads.push_back(std::vector<Player>());
	}

	int game_end(bool forced=false) const {
		if (ptA >= 5)
			return 1;
		if (ptB >= 5)
			return 2;
		return 0;
	}
		
	bool move(int x, Player current, bool apply=true) {
		auto& content = quads.at(x);	// quel & mi ha fatto perdere tipo 2ore... senza chiama il costruttore di copia e poi sembrano non funzionare le rimozioni dal vector
		const int content_size = static_cast<int>(content.size());

		if (content_size <= 0)
			return false;
		const auto top = content.back();
		
		if (current != top)
			return false;

		if (apply) {
			//std::cout << "Move: " << x << std::endl;
			const int dx = current==Player::A ? +1 : -1;
			int next = x;

			auto iter = content.begin() + (content_size > 5 ? content_size-5 : 0);
			
			while (iter != content.end()) {
				const auto cur = *iter++;
				
				next += dx;
				while (next < 0 || next >= size || quads.at(next).size() >= 5) {
					if (next < 0)
						next += size;
					else if (next >= size)
						next -= size;
					else
						next += dx;
				}
				
				if (next <= 5 && x >= size-5 && cur == Player::A)	//y queste due condizioni sono incomplete, penso
					ptA++;
				else if (next <= 10 && x > 10 && cur == Player::B)
					ptB++;
				else
					quads.at(next).push_back(cur);				
			}
			
			content.resize(content_size > 5 ? content_size-5 : 0);
		}
		
		return true;
	}
	
	bool areTherePossibleMoves(Player current) { return getPossibleMoves(current).size() > 0; }
	
	std::vector<int> getPossibleMoves(Player current) {
		std::vector<int> retv;
	
		for (int x=0; x<size; ++x)
			if (move(x, current, false))
				retv.push_back(x);
		
		return retv;
	}

	bool move_ai(Player current) {
		std::vector<int> moves = getPossibleMoves(current);
		if (moves.size() == 0)
			return false;
		
		std::uniform_int_distribution<> dis(0, moves.size()-1);
		int selected = moves.at(dis(gen));
		return move(selected, current);
	}

	void draw() const {
		std::cout << "Points: [" << ptA << ", " << ptB << "]" << std::endl;
		int idx=0;
		for (const auto& q : quads) {
			std::cout << idx++ << ") ";

			for (const auto& qc : q)
				std::cout << static_cast<int>(qc);
			
			std::cout << std::endl;
		}
	}
};

class Match
{
	Level lvl;
	Player current;
	std::vector<bool> human;
	
public:
	Match(bool human0, bool human1) : lvl(), current(Player::A), human({human0, human1}) {}
	
	int play() {
		lvl.draw();
		
		while (lvl.game_end() == 0) {
			if (!lvl.areTherePossibleMoves(current)) {
				std::cout << "No possible moves." << std::endl;
				goto applied_move;
			}
		
			if (human.at(static_cast<int>(current))) {
				int x;
				std::cin >> x;
				if (lvl.move(x, current)) {
	applied_move:	lvl.draw();
					current = current == Player::A ? Player::B : Player::A;
				}
				else
					std::cout << "invalid" << std::endl;
			}
			else {
				if (lvl.move_ai(current))
					goto applied_move;
				
				std::cout << "Error?!" << std::endl;
				break;
			}
		}
		const int pwon = lvl.game_end(true);
		std::cout << std::string("Player ") + std::to_string(pwon) + std::string(" won.") << std::endl;
		return pwon;
	}
};


int main(int argc, char* argv[])
{
	Match match( argc>1 && (std::string(argv[1]).compare("1")==0 || std::string(argv[1]).compare("2")==0), argc>1 && std::string(argv[1]).compare("2")==0);
	match.play();
}