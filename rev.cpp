#include <vector>
#include <iostream>
#include <random>

class Level
{
	enum class QuadContent : char 						{ Empty=' ', A='1', B='2' };

	std::default_random_engine rd;
    std::mt19937 gen;
	std::vector<QuadContent> quads;
	int sizex, sizey;

	int index(int x, int y) const						{ return y*sizex+x; }
	QuadContent getContent(int x, int y) const 			{ return quads.at(index(x, y)); }
	void setContent(int x, int y, QuadContent content)	{ quads.at(index(x, y)) = content; }
	
public:
	Level() : rd(), gen(rd()), quads(), sizex(10), sizey(10) {
		for (int y=0; y<sizey; ++y)
			for (int x=0; x<sizex; ++x)
				if ((x==4&&y==4) || (x==5&&y==5))
					quads.push_back(QuadContent::A);
				else if ((x==5&&y==4) || (x==4&&y==5))
					quads.push_back(QuadContent::B);
				else
					quads.push_back(QuadContent::Empty);
	}

	int game_end(bool forced=false) const {
		int ca=0, cb=0, ce=0;
	
		for (auto&& q : quads)
			if (q == QuadContent::A)
				++ca;
			else if (q == QuadContent::B)
				++cb;
			else
				++ce;
		
		if (forced)
			return ca>cb?1:cb>ca?2:0;
		
		if ((ce == 0 && (ca > cb)) || (cb == 0))
			return 1;
		if ((ce == 0 && (cb > ca)) || (ca == 0))
			return 2;
		return 0;
	}
		
	bool move(int x, int y, int current, bool apply=true) {
		if (getContent(x, y) != QuadContent::Empty)
			return false;
			
		const QuadContent cp = current==0  ?  QuadContent::A  :  QuadContent::B;
		bool retv = false;
		
		for (int dy=-1; dy<=+1; ++dy)
			for (int dx=-1; dx<=+1; ++dx)
				if (dx != 0 || dy != 0) {
					bool good_line = true;
				
					for (int k=1; ; k++) {
						if (x+k*dx < 0 || x+k*dx >= sizex || y+k*dy < 0 || y+k*dy >= sizey) {
							good_line = false;
							break;
						}
					
						const QuadContent cc = getContent(x+k*dx, y+k*dy);
						
						if (cc == QuadContent::Empty) {
							good_line = false;
							break;
						}
						else if (cc == cp) {
							good_line = (k>1);
							break;
						}
					}
					
					if (good_line) {
						if (apply) {
							setContent(x, y, cp);
							for (int j=1; getContent(x+j*dx, y+j*dy) != cp; j++)
								setContent(x+j*dx, y+j*dy, cp);
						}

						retv = true;
					}
				}		
		
		if (retv && apply)
			std::cout << "Move: (" << x << ", " << y << ")" << std::endl;
		
		return retv;
	}
	
	bool areTherePossibleMoves(int current) { return getPossibleMoves(current).size() > 0; }
	
	std::vector<int> getPossibleMoves(int current) {
		std::vector<int> retv;
	
		for (int y=0; y<sizey; ++y)
			for (int x=0; x<sizex; ++x)
				if (move(x, y, current, false))
					retv.push_back(sizex*y+x);
		
		return retv;
	}

	bool move_ai(int current) {
		std::vector<int> moves = getPossibleMoves(current);
		if (moves.size() == 0)
			return false;
		
		std::uniform_int_distribution<> dis(0, moves.size()-1);
		int selected = moves.at(dis(gen));
		return move(selected%sizex, selected/sizex, current);
	}

	void draw() const {
		int count = 0;
		for (auto&& q : quads) {
			std::cout << static_cast<char>(q);
			if (++count % sizex == 0)
				std::cout << std::endl;
		}
	}
};

int main(int argc, char* argv[])
{
	Level lvl;
	int current=0;
	// usage: ./argv[0]   pc vs pc,     ./argv[0] 1   human vs pc,      ./argv[0]   human vs human
	std::vector<bool> human { argc>1 && (std::string(argv[1]).compare("1")==0 || std::string(argv[1]).compare("2")==0), argc>1 && std::string(argv[1]).compare("2")==0 };	
	lvl.draw();
	
	while (lvl.game_end() == 0) {
		if (!lvl.areTherePossibleMoves(current)) {
			std::cout << "No possible moves!" << std::endl;
			
			if (!lvl.areTherePossibleMoves(current==0?1:0)) {
				std::cout << std::string("..no player can move!") << std::endl;
				break;
			}
			
			current = (current+1)%2;
		}
		else if (human.at(current)) {
			int x, y;
			std::cin >> x >> y;
			if (lvl.move(x, y, current)) {
applied_move:	lvl.draw();
				current = (current+1)%2;
			}
			else
				std::cout << "invalid" << std::endl;
		}
		else if (lvl.move_ai(current))
			goto applied_move;
	}
	std::cout << std::string("Player ") + std::to_string(lvl.game_end(true)) + std::string(" won.") << std::endl;
}