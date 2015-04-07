#include <vector>
#include <iostream>
#include <fstream>

class Level
{
	enum class QuadContent : char { Empty=' ', Wall='#', Pg='P', Chest='C', Target='+', ChestAndTarget='*', PgAndTarget='Q' };

	std::vector<QuadContent> quads;
	int sizex;
	int sizey;

	int index(int x, int y) const	{ return y*sizex+x; }
	int getPlayerPos() const {
		int idx=0;
		for (auto&& q : quads) {
			if (q == QuadContent::Pg || q == QuadContent::PgAndTarget)
				return idx;
			++idx;
		}
		
		return -1;
	}

	QuadContent getContent(int x, int y) const 			{ return quads.at(index(x, y)); }
	void setContent(int x, int y, QuadContent content)	{ quads.at(index(x, y)) = content; }
	
public:
	Level(int dx, int dy, const std::vector<QuadContent>& content) : quads(content), sizex(dx), sizey(dy) {}
	
	static Level createLevel(std::string filename) {
		std::ifstream myReadFile(filename);
		if (!myReadFile.is_open())
			throw LevelException();
		
		std::vector<QuadContent> quads;
		int sx=-1, sy=0;
		
		while (!myReadFile.eof()) {
			std::string line;
			std::getline(myReadFile, line);
			if (sx < 0)
				sx = line.size();
				
			for (char& c : line)
				quads.push_back(static_cast<QuadContent>(c));
			++sy;
		}
		
		return Level(sx>=0?sx:0, sy, quads);
	}

	bool game_won() const {
		for (auto&& q : quads)
			if (q == QuadContent::Target || q == QuadContent::PgAndTarget)
				return false;
				
		return true;
	}
	
	bool move(std::string dir) {
		const int dx = dir.compare("d")==0 ? +1 : dir.compare("a")==0 ? -1 : 0;
		const int dy = dir.compare("s")==0 ? +1 : dir.compare("w")==0 ? -1 : 0;
		if (dx == 0 && dy == 0)
			return false;
			
		const int ppos = getPlayerPos();
		const int px = ppos % sizex;
		const int py = ppos / sizex;
		QuadContent qpos = getContent(px, py);
		
		if (px+dx < 0 || px+dx >= sizex || py+dy < 0 || py+dy >= sizey)
			return false;
		const QuadContent qnext = getContent(px+dx, py+dy);
		if (qnext == QuadContent::Wall)
			return false;
		else if (qnext != QuadContent::Chest && qnext != QuadContent::ChestAndTarget) {
			setContent(px, py, qpos == QuadContent::Pg ? QuadContent::Empty : QuadContent::Target);
			setContent(px+dx, py+dy, qnext == QuadContent::Empty ? QuadContent::Pg : QuadContent::PgAndTarget);
			return true;
		}
		
		if (px+2*dx < 0 || px+2*dx >= sizex || py+2*dy < 0 || py+2*dy >= sizey)
			return false;
		const QuadContent qnext2 = getContent(px+2*dx, py+2*dy);
		if (qnext2 == QuadContent::Wall || qnext2 == QuadContent::Chest || qnext2 == QuadContent::ChestAndTarget)
			return false;
		else {
			setContent(px, py, qpos == QuadContent::Pg ? QuadContent::Empty : QuadContent::Target);
			setContent(px+dx, py+dy, qnext == QuadContent::Chest ? QuadContent::Pg : QuadContent::PgAndTarget);
			setContent(px+2*dx, py+2*dy, qnext2 == QuadContent::Empty ? QuadContent::Chest : QuadContent::ChestAndTarget);
			return true;
		}
	}

	void draw() const {
		int count = 0;
		for (auto&& q : quads) {
			std::cout << static_cast<char>(q);
			if (++count % sizex == 0)
				std::cout << std::endl;
		}
	}
	
	class LevelException {};
};

int main() try
{
	for (int i=1; ; i++) {
		Level lvl(Level::createLevel(std::string("soko") + std::to_string(i) + std::string(".txt")));
		lvl.draw();
		
		while (!lvl.game_won()) {
			std::string input;
			std::cin >> input;
			if (input.compare("quit") == 0)
				return 0;
			else if (lvl.move(input))
				lvl.draw();
			else
				std::cout << "invalid" << std::endl;
		}
		
		std::cout << "congratz" << std::endl;
	}
}
catch (...) { std::cout << "Error loading level." << std::endl; }