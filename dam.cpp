#include <vector>
#include <iostream>
#include <random>
#include <algorithm>

enum class Pedina : int { Empty=0, A=1, B=2, AA=3, BB=4 };

class Level
{
	std::random_device rd;

	std::vector<Pedina> quads;

	bool rule_PawnCanEatDraughts;
	bool rule_PawnsCanMoveBackward;
	bool rule_PawnsCanEatBackward;
	bool rule_MustCaptureIfPossible;
	
public:
	Level() : rd(), quads(), rule_PawnCanEatDraughts(false), rule_PawnsCanMoveBackward(false), rule_PawnsCanEatBackward(true), rule_MustCaptureIfPossible(true) {
		for (int x=0; x<32; ++x)
			if (x < 12)
				quads.push_back(Pedina::A);
			else if (x < 12+4+4)
				quads.push_back(Pedina::Empty);
			else
				quads.push_back(Pedina::B);
	}

	int game_end(bool forced=false) const {
		int ca=0, cb=0;
	
		for (auto&& q : quads)
			if (q == Pedina::A || q == Pedina::AA)
				++ca;
			else if (q == Pedina::B || q == Pedina::BB)
				++cb;
		
		return ca==0?2:cb==0?1:0;
	}
		
	bool move(int from, int to, Pedina current, bool apply=true) {
		if (from < 0 || from >= 32 || to < 0 || to >= 32 || to == from)
			return false;	// fuori dalla scacchiera
	
		const auto from_content = quads.at(from);
		if (from_content == Pedina::Empty || static_cast<int>(from_content)%2 != static_cast<int>(current)%2)
			return false;	// non è una tua pedina
			
		const auto to_content = quads.at(to);
		if (to_content != Pedina::Empty)
			return false;	// non puoi mettere una pedina dove ce n'è già un'altra

		// capture, se è un pedone le regole possono permettergli di mangiare all'indietro
		if (rule_PawnsCanEatBackward || !((from_content == Pedina::A && to < from) || (from_content == Pedina::B && to > from))) {
			int eaten = -1;
			if (from == to+9 && from>=8 && from%4 != 0)		 	// ordine, frecce: 7913 = qezc
				eaten = from - (from%8>=4?4:5);
			else if (from == to+7 && from>=8 && from%4 != 3)
				eaten = from - (from%8>=4?3:4);
			else if (from == to-7 && from<=23 && from%4 != 0)
				eaten = from + (from%8>=4?4:3);
			else if (from == to-9 && from<=23 && from%4 != 3)
				eaten = from + (from%8>=4?5:4);

			if (eaten >= 0 && eaten < 32) {						// otherwise, it is not a capture
				const auto eaten_content = quads.at(eaten);
				if (eaten_content == Pedina::Empty || static_cast<int>(eaten_content)%2 == static_cast<int>(from_content)%2)
					return false;	// non puoi saltare un quadrato vuoto , non puoi mangiare una tua pedina

				if (!rule_PawnCanEatDraughts && (from_content == Pedina::A || from_content == Pedina::B) && (eaten_content == Pedina::AA || eaten_content == Pedina::BB))
					return false;	// se regole, una pedina non può mangiare una dama
		
				if (apply) {
					quads.at(eaten) = Pedina::Empty;
					apply_move(from_content, from, to, true);
				}

				return true;
			}
		}

		// mossa normale, se è un pedone le regole possono permettergli di muoversi all'indietro
		if (rule_PawnsCanMoveBackward || !((from_content == Pedina::A && to < from) || (from_content == Pedina::B && to > from))) {
			if ((from == to + (from%8>=4?4:5) && from>=4 && from%8!=0) ||
				(from == to + (from%8>=4?3:4) && from>=4 && from%8!=7) ||
				(from == to - (from%8>=4?4:3) && from<=27 && from%8!=0) ||
				(from == to - (from%8>=4?5:4) && from<=27 && from%8!=7)) {
				if (apply)
					apply_move(from_content, from, to, false);
				return true;
			}
		}

		return false;
	}
	
	bool areTherePossibleMoves(Pedina current) { return getPossibleMoves(current).size() > 0; }
	
	std::vector<int> getPossibleMoves(Pedina current) {
		std::vector<int> retv;

		for (int from=0; from<32; ++from) {
			const auto content = quads.at(from);
			if (content != Pedina::Empty  &&  static_cast<int>(content)%2 == static_cast<int>(current)%2)
				for (int to=0; to<32; ++to)
					if (quads.at(to) == Pedina::Empty)
						if (move(from, to, current, false))
							retv.push_back(32*from + to);
		}

		if (rule_MustCaptureIfPossible) {
			std::vector<int> captures;

			for (auto x : retv) {
				int from = x/32;
				int to = x%32;
				if (from == to+7 || from == to-7 || from == to+9 || from == to-9)
					captures.push_back(x);
			}

			if (captures.size() > 0)
				return captures;
		}
		
		return retv;
	}

	bool move_ai(Pedina current) {
		std::vector<int> moves = getPossibleMoves(current);
		if (moves.size() == 0)
			return false;
		
		std::uniform_int_distribution<> dis(0, moves.size()-1);
	    std::mt19937 gen2(rd());
		int selected = moves.at(dis(gen2));

		return move(selected/32, selected%32, current);
	}

	void draw() const {
		const std::string spacing = "   ";

		for (int y=0; y<8; y++) {
			if (y % 2 == 1)
				std::cout << "  ";

			for (int k=0; k<4; ++k) {
				std::cout << (4*y+k) << spacing;
				if (4*y+k < 10) std::cout << ' ';
			}

			std::cout << spacing << spacing;		

			for (int x=0; x<4; x++)
				std::cout << static_cast<int>(quads.at(4*y+x)) << spacing;
				
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}

private:
	void apply_move(Pedina from_content, int from, int to, bool capture=false, bool output_move=true)
	{
		bool promoted = false;
		if (from_content == Pedina::A && to>=28){
			quads.at(to) = Pedina::AA;
			promoted = true;
		}
		else if (from_content == Pedina::B && to<4) {
			quads.at(to) = Pedina::BB;
			promoted = true;
		}
		else		
			quads.at(to) = quads.at(from);
	
		quads.at(from) = Pedina::Empty;
		if (output_move) {
			std::cout << "Move " << from << " -> " << to;
			if (promoted) std::cout << "  PROMOTED";
			if (capture) std::cout << "  CAPTURE";
			std::cout << std::endl;
		}
	}
};

class Match
{
	Level lvl;
	Pedina current;
	std::vector<bool> human;
	int moves;
	
public:
	Match(bool human0, bool human1) : lvl(), current(Pedina::A), human({human0, human1}), moves(0) {}
	
	int play() {
		lvl.draw();
		while (lvl.game_end() == 0) {
			std::vector<int> possibleMoves = lvl.getPossibleMoves(current);

			if (possibleMoves.size() == 0) {
				std::cout << "No possible moves." << std::endl;
				if (!lvl.areTherePossibleMoves(current==Pedina::A?Pedina::B:Pedina::A)) {
					std::cout << "...for both players!" << std::endl;
					break;
				}
				
				goto applied_move;
			}
			if (human.at(current==Pedina::A?0:1)) {
				int from, to;
				std::cin >> from >> to;

				if (std::find(possibleMoves.begin(), possibleMoves.end(), 32*from+to) != possibleMoves.end()) {
					lvl.move(from, to, current);
	applied_move:	lvl.draw();
					current = current == Pedina::A ? Pedina::B : Pedina::A;
					++moves;
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
		std::cout << std::string("Player ") + std::to_string(pwon) + std::string(" won in ") << (moves/2) << std::string(" moves.") << std::endl;
		return pwon;
	}
};


int main(int argc, char* argv[])
{
	Match match( argc>1 && (std::string(argv[1]).compare("1")==0 || std::string(argv[1]).compare("2")==0), argc>1 && std::string(argv[1]).compare("2")==0);
	match.play();
}
