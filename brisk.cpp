
#include <vector>
#include <iostream>
#include <algorithm>    // std::random_shuffle
#include <ctime>        // std::time
#include <cstdlib>      // std::rand, std::srand
#include <numeric>

class Card
{
public:
	enum class Seme : char { Denari=0, Coppe=1, Spade=2, Bastoni=3 };
	
	static char getSemeSymbol(Card::Seme c) {
		return card_seme_symbols[static_cast<int>(c)];
	}
private:
	const static int card_values[];
	const static char card_symbols[];
	const static char card_seme_symbols[];
	const static int card_order[];

	Card::Seme seme;
	char value;
	
public:
	Card(Seme s, char v) : seme(s), value(v) {}
	
	Seme getSeme() const { return seme; }
	
	char getSymbol() const { 
		return card_symbols[value-1];
	}		
	int getPoints() const {
		return card_values[value-1];
	}
	int getOrder() const {
		return card_order[value-1];
	}
	char getSemeSymbol() const {
		getSemeSymbol(seme);
	}
};

const int Card::card_order[] = { 8, 0, 9, 1, 2, 3, 4, 5, 6, 7 };
const int Card::card_values[] = { 11, 0, 10, 0, 0, 0, 0, 2, 3, 4 };
const char Card::card_symbols[] = { 'A', '2', '3', '4', '5', '6', '7', 'P', 'Q', 'K' };
const char Card::card_seme_symbols[] = { 'D', 'C', 'S', 'B' };

std::ostream &operator<<(std::ostream &os, Card const &m) { 
	return os << m.getSymbol() << m.getSemeSymbol();
}

class Mazzo
{
	std::vector<Card> carte;
public:
	Mazzo() : carte() { init(); }
	
	Card pesca()
	{
		Card c = *carte.begin();
		carte.erase(carte.begin());
		return c;
	}
	
	Card::Seme briscola()
	{
		return carte.end()->getSeme();
	}
	
	void init()
	{
		const Card::Seme semi[4] = { Card::Seme::Denari, Card::Seme::Coppe, Card::Seme::Spade, Card::Seme::Bastoni };
		
		for (auto&& s : semi)
			for (int i=1; i<=10; i++)
				carte.push_back(Card(s, i));		
	}
	
	int size() const { return carte.size(); }
	
	void mescola()
	{
		std::random_shuffle(carte.begin(), carte.end());
	}
	
	void debug()
	{
		std::cout << "{ ";
		for (auto it = carte.begin(); it != carte.end(); ++it) {
			std::cout << *it;
			if (it != carte.end()-1)
				std::cout << ", ";
		}
		std::cout << " } " << carte.size() << std::endl;
	}		
};

class Player
{
	std::vector<Card> carte;
public:
	Player() : carte() {}
	
	void pesca(const Card& c) { carte.push_back(c); }
	int ncarte() const { return carte.size(); }
	Card gioca(std::vector<Card> giocate, int i=0) { 
		Card c = carte.at(i);
		carte.erase(carte.begin()+i);
		return c;
	}
	
	void debug()
	{
		for (auto&& x : carte)
			std::cout << "\t\t" << x << std::endl;
	}
};

class Team
{
	std::vector<Player> players;
public:
	Team() : players() {}
	
	void addPlayer(const Player& p) { players.push_back(p); }
	int nplayers() const { return players.size(); }
	Player& getPlayer(int i)	{ return players.at(i); }
	
	void debug(int pts=-1)
	{
		std::cout << "Team" << std::endl;
		for (auto&& p : players) {
			std::cout << "\tPlayer" << std::endl;
			
			if (pts >= 0)
				std::cout << "\t\tPunti: " << pts << std::endl;
			
			p.debug();
		}
	}
};

class Match
{
	Mazzo mazzo;
	std::vector<Team> teams;
	std::vector<int> punti;
	Card::Seme briscola;
	
	int evaluate(std::vector<Card> giocata, Card::Seme briscola, int firstone)
	{
		int pts = 0;
		int besti = -1;
		int bestv = -1;
		
		std::cout << firstone;
		for (auto&& x : giocata)
			std::cout << " " << x;
		std::cout << std::endl;
		
		for (int i=0; i<giocata.size(); i++) {
			const Card thiscard = giocata.at(i);
			pts += thiscard.getPoints();
			
			const bool thisbrisc = thiscard.getSeme()==briscola;
			const bool thisasfirst = thiscard.getSeme()==giocata.at(0).getSeme();
			const int thisvalue = (thisbrisc ? 10 : 0) + (thisbrisc|thisasfirst ? thiscard.getOrder() : 0);

			if (thisvalue > bestv) {
				bestv = thisvalue;
				besti = i;
			}
		}

		punti.at((besti+firstone) % teams.size()) += pts;
		return (besti+firstone) % nplayers();
	}
	
	Player& getPlayer(int i)
	{
		i %= nplayers();
		return teams.at(i % teams.size()).getPlayer(i / teams.size());
	}
	
public:
	Match(int teamn, int teamsize) : mazzo(), teams(), punti(), briscola() {
		mazzo.mescola();
		
		for (int t=0; t<teamn; t++) {
			Team team;
			
			for (int s=0; s<teamsize; s++) {
				Player player;
				
				for (int i=0; i<3; i++)
					player.pesca(mazzo.pesca());
				
				team.addPlayer(player);
			}
			
			teams.push_back(team);
			punti.push_back(0);
		}				
	}

	int nplayers() const
	{
		return std::accumulate(teams.begin(), teams.end(), 0, [](int x, const Team& t) { return x + t.nplayers(); });
	}
	
	void debug()
	{
		for (int i=0; i<teams.size(); i++)
			teams.at(i).debug(punti.at(i));
			
		std::cout << std::endl;
		mazzo.debug();
	}
	
	bool gameover()
	{
		for (auto&& t : teams) {
			const int npl = t.nplayers();
			for (int i=0; i<npl; i++)
				if (t.getPlayer(i).ncarte() > 0)
					return false;
		}
		
		return true;
	}
	
	void play()
	{
		const int npl = nplayers();
		const Card::Seme briscola = mazzo.briscola();
		std::cout << "Briscola: " << Card::getSemeSymbol(briscola) << std::endl;
		int firstone = 0;

		while (!gameover()) {
			debug();
			
			const bool pescare = mazzo.size() >= npl;
		
			std::vector<Card> giocata;
			
			for (int i=0; i<npl; i++) {
					Player& p = getPlayer(firstone + i);
					giocata.push_back( p.gioca(giocata) );
					if (pescare)
						p.pesca(mazzo.pesca());
				}
			
			firstone = evaluate(giocata, briscola, firstone);
		}
		
		int imax=0;
		for (int i=1; i<punti.size(); i++)
			if (punti.at(i) > punti.at(imax))
				imax = i;
		for (int i=0; i<punti.size(); i++) {
			std::cout << "Team" << i << ": " << punti.at(i);
			if (i == imax)
				std::cout << " *** WIN *** ";
			std::cout << std::endl;
		}
	}
};

int main()
{
	Match(2,2).play();
}
