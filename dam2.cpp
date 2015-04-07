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
	bool rule_MustCaptureIfPossibleHighest;
	
public:
	Level() : rd(), quads(), rule_PawnCanEatDraughts(false), rule_PawnsCanMoveBackward(false), rule_PawnsCanEatBackward(false), rule_MustCaptureIfPossible(true), rule_MustCaptureIfPossibleHighest(true) {
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

	
	struct Move {
		Pedina original;
		std::vector<int> moves;
		std::vector<int> eaten;
		int points;
		bool promotion;
		
		Move(Pedina orig, int from, int to, bool prom=false, int pts=0) : original(orig), moves(std::vector<int>{from, to}), eaten(), points(pts), promotion(prom) {}
		Move(const Move& mv) : original(mv.original), moves(mv.moves), eaten(mv.eaten), points(mv.points), promotion(mv.promotion) {}		
		Move(Pedina orig, std::vector<int> mv, std::vector<int> ea, bool prom=false, int pts=0) : original(orig), moves(mv), eaten(ea), points(pts), promotion(prom) {}
		
		bool isCapture() const		{ return eaten.size() > 0; }
		void addPoints(int p)		{ points += p; }
	};
	
	bool areTherePossibleMoves(Pedina current) { return  experimental_getAllPossibleMoves(current).size() > 0; }

	static void announceMoves(const std::vector<Move>& moves)
	{
		std::cout << "Possible moves: " << moves.size() << std::endl;
		int i=0;
		for (auto&& x : moves) {
			if (x.moves.size() > 0) {
				std::cout << i << ") " << "Move: ";
				for (auto v : x.moves)
					std::cout << v << "  ";
				std::cout << std::endl;
			}
			if (x.eaten.size() > 0) {
				std::cout << "\tCaptured=" << x.points << ": ";
				for (auto v : x.eaten)
					std::cout << v << "  ";
				std::cout << std::endl;
			}
			++i;
		}			
	}
	
	std::vector<Move> experimental_getAllPossibleMoves(Pedina current)
	{
		std::vector<Move> capt = experimental_getPossibleCaptures(current);
		if (!rule_MustCaptureIfPossible || capt.size() == 0) {
			std::vector<Move> moves = experimental_getPossibleMoves(current);
			moves.insert(moves.end(), capt.begin(), capt.end());
			return moves;
		}

		if (!rule_MustCaptureIfPossibleHighest)
			return capt;
		// must capture highest
		int hp = -1;
		std::vector<Move> gd;
		
		for (auto&& x : capt) {
			if (x.points > hp) {
				hp = x.points;
				gd.clear();
			}
			if (x.points >= hp)
				gd.push_back(x);
		}
		
		return gd;				
	}
	
	std::vector<Move> experimental_getPossibleMoves(Pedina current)
	{
		std::vector<Move> retv;	
		int idx=0;
		for (auto&& q : quads) {
			if (q != Pedina::Empty && static_cast<int>(q)%2 == static_cast<int>(current)%2) {	// se è una tua pedina
				std::vector<int> possibles;
				if (idx>=4 && idx%8!=0)
					possibles.push_back(idx-(idx%8>=4?4:5));
				if (idx>=4 && idx%8!=7)
					possibles.push_back(idx-(idx%8>=4?3:4));
				if (idx<=27 && idx%8!=0)
					possibles.push_back(idx+(idx%8>=4?4:3));
				if (idx<=27 && idx%8!=7)
					possibles.push_back(idx+(idx%8>=4?5:4));
					
				for (auto p : possibles)
					if (p >= 0 && p < 32)
						if (rule_PawnsCanMoveBackward || !((q == Pedina::A && p < idx) || (q == Pedina::B && p > idx)))
							if (quads.at(p) == Pedina::Empty)
								retv.push_back(Move(q==Pedina::A && p>=28 ? Pedina::AA : q==Pedina::B && p<=3 ? Pedina::BB :  q, idx, p, (q==Pedina::A && p>=28) || (q==Pedina::B && p<=3)));
			}
			++idx;
		}

		return retv;
	}
	
	std::vector<Move> experimental_getPossibleCaptures(Pedina current)
	{
		std::vector<Move> retv;
		int idx=0;
		for (auto&& q : quads) {
			if (q != Pedina::Empty && static_cast<int>(q)%2 == static_cast<int>(current)%2) {	// se è una tua pedina
				std::vector<Move> cur = experimental_getPossibleCaptures_rec(Move(q, std::vector<int>({idx}), std::vector<int>()));
				retv.insert(retv.end(), cur.begin(), cur.end());
			}
			++idx;
		}

		return retv;
	}

	std::vector<Move> experimental_getPossibleCaptures_rec(Move cur_move)
	{
		Pedina q = cur_move.original;
		std::vector<Move> retv;
	
		const int idx = cur_move.moves.back();	// punto di partenza

		std::vector<int> possibles;
		std::vector<int> captures;
		
		if (idx>=8 && idx%4!=0) {
			possibles.push_back(idx-9);
			captures.push_back(idx - (idx%8>=4?4:5));
		}
		if (idx>=8 && idx%4!=3) {
			possibles.push_back(idx-7);
			captures.push_back(idx - (idx%8>=4?3:4));
		}
		if (idx<=23 && idx%4!=0) {
			possibles.push_back(idx+7);
			captures.push_back(idx + (idx%8>=4?4:3));
		}
		if (idx<=23 && idx%4!=3) {
			possibles.push_back(idx+9);
			captures.push_back(idx + (idx%8>=4?5:4));
		}

		int pm=0;
		for (auto p : possibles) {					// per le 4 mosse possibili	
			int capture_idx = captures.at(pm++);
			if (std::find(cur_move.eaten.begin(), cur_move.eaten.end(), capture_idx) != cur_move.eaten.end())	// non già-mangiata
				continue;
			
			if (p < 0 || p >= 32 || capture_idx < 0 || capture_idx >= 32)
				continue;
			auto eaten_content = quads.at(capture_idx);
			
			// non puoi mangiare una tua pedina, regole->una pedina non può mangiare una dama
			if (eaten_content==Pedina::Empty || (static_cast<int>(q)%2 == static_cast<int>(eaten_content)%2) || (!rule_PawnCanEatDraughts && (q==Pedina::A||q==Pedina::B) && (eaten_content==Pedina::AA||eaten_content==Pedina::BB)))
				continue;
		
			if (!rule_PawnsCanEatBackward && ((q == Pedina::A && p < idx) || (q == Pedina::B && p > idx)))				// se le regole permettono al pezzo di mangiare nella direzione
				continue;
				
			if (quads.at(p) == Pedina::Empty || std::find(cur_move.moves.begin(), cur_move.moves.end()-1, p)!=cur_move.moves.end()-1) {			// se si mangia una pedina dell'altro giocatore e il quadrato d'arrivo è vuoto
				Move newmove(cur_move);					// mossa valida. clone move
				newmove.moves.push_back(p);
				newmove.eaten.push_back(capture_idx);
				newmove.addPoints(eaten_content == Pedina::AA || eaten_content == Pedina::BB ? 2 : 1);
				
				if (newmove.original == Pedina::A && p >= 28) {
					newmove.original = Pedina::AA;
					newmove.promotion = true;
				}
				if (newmove.original == Pedina::B && p <= 3) {
					newmove.original = Pedina::BB;
					newmove.promotion = true;
				}
			
				retv.push_back(newmove);
				
				std::vector<Move> recurs = experimental_getPossibleCaptures_rec(newmove);
				retv.insert(retv.end(), recurs.begin(), recurs.end());
			}
		}
		
		return retv;
	}
	
	bool move_ai(Pedina current) {
		auto moves = experimental_getAllPossibleMoves(current);
		if (moves.size() == 0)
			return false;
		
		std::uniform_int_distribution<> dis(0, moves.size()-1);
	    std::mt19937 gen2(rd());
		apply(moves.at(dis(gen2)), true);
		return true;
	}

	void draw() const {
		const std::string spacing = "   ";

		for (int y=0; y<8; y++) {
			if (y % 2)
				std::cout << "  ";

			for (int k=0; k<4; ++k) {
				std::cout << (4*y+k) << spacing;
				if (4*y+k<10) std::cout << ' ';
			}

			std::cout << spacing << spacing;		

			for (int x=0; x<4; x++)
				std::cout << static_cast<int>(quads.at(4*y+x)) << spacing;
				
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}

	void apply(Move move, bool output_move=true) {
		if (move.moves.size() > 0) {
			const int from = move.moves.front();
			const int to = move.moves.back();
			quads.at(from) = Pedina::Empty;
			quads.at(to) = move.original;
		
			for (auto x : move.eaten)
				quads.at(x) = Pedina::Empty;
			if (output_move) {
				std::cout << "Move " << from << " -> " << to;
				if (move.isCapture())
					std::cout << "  CAPTURE";
				if (move.promotion)
					std::cout << "  PROMOTION";
				std::cout << std::endl;
			}
		}
	}
};

class Match
{
	Level lvl;
	std::vector<bool> human;
	int moves;
	
public:
	Match(bool human0, bool human1) : lvl(), human({human0, human1}), moves(0) {}
	
	int play() {
		lvl.draw();
		while (lvl.game_end() == 0) {
			auto possibleMoves = lvl.experimental_getAllPossibleMoves(moves%2?Pedina::B:Pedina::A);

			if (possibleMoves.size() == 0) {
				std::cout << "No possible moves." << std::endl;
				if (!lvl.areTherePossibleMoves(moves%2?Pedina::A:Pedina::B)) {
					std::cout << "...for both players!" << std::endl;
					break;
				}
			}
			if (human.at(moves%2)) {
				Level::announceMoves(possibleMoves);
				std::string input;

	reinput:	std::cin >> input;
				if (input.compare("quit") == 0) {
					std::cout << "Player " << std::to_string((moves%2)+1) << " resigned." << std::endl;
					return (moves%2)+1;
				}

				try { 
					const int selected_move = std::stoi(input);
					if (selected_move < 0 || selected_move >= (int) possibleMoves.size()) {
						std::cout << "Invalid." << std::endl;
						goto reinput;
					}
					
					lvl.apply(possibleMoves.at(selected_move), true);
				}
				catch (...) {
					std::cout << "Input is not a number." << std::endl;
					goto reinput;
				}
			}
			else if (!lvl.move_ai(moves%2?Pedina::B:Pedina::A)) {
				std::cout << "Error?!" << std::endl;
				return -1;
			}
			
			lvl.draw();
			++moves;
		}
		const int pwon = lvl.game_end(true);
		std::cout << "Player " << std::to_string(pwon) << " won in " << (moves/2) << " moves." << std::endl;
		return pwon;
	}
};


int main(int argc, char* argv[])
{
	const bool human0 = argc>1 && (std::string(argv[1]).compare("1")==0 || std::string(argv[1]).compare("2")==0);
	const bool human1 = argc>1 && std::string(argv[1]).compare("2")==0;
	Match match(human0, human1);
	return match.play();
}
