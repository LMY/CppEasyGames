
#include <iostream>
#include <vector>
#include <array>
#include <math.h>

using namespace std;

template<typename C>
C sign(C x) { return copysign(1.0, x); }


class PositionIdentifier
{
	int pos : 6;
	
	template<typename A, typename B>
	int Index(A x, B y) const { return 8*y+x; }
	
public:
	template<typename A, typename B>
	PositionIdentifier(A x, B y) : pos(Index(x,y)) {}
	
	int getX() const { return pos%8; }
	int getY() const { return pos/8; }
};



enum class Color { None, White, Black };
enum class PieceType { Empty, Pawn, Knight, Bishop, Rook, Queen, King };

class Piece
{
public:
	Piece(Color c=Color::None, PieceType t=PieceType::Empty) : color(c), type(t) {}
	Piece(const Piece& p) : color(p.color), type(p.type) {}
	Piece& operator=(const Piece& p) { color = p.color; type = p.type; return *this; }
	
	Color color;
	PieceType type;
	
	static Piece EmptyQuad;
};

Piece Piece::EmptyQuad(Color::None, PieceType::Empty);


class Quad
{
public:
	Quad(int _x=0, int _y=0) : x(_x), y(_y) {}
	Quad(const Quad& q) : x(q.x), y(q.y) {}
	Quad& operator=(const Quad& q) { x=q.x; y=q.y; return *this; }

	bool isValid() const { return x>0 && y>0; }
	
	int x;
	int y;
	
	static Quad NoWhere;
	static Quad createQuad(string s);
};

Quad Quad::NoWhere(-1, -1);

Quad Quad::createQuad(string s)
{
	return Quad(s[0]-'a', s[1]-'1');
}


class Move
{
public:
	Move(Quad _from, Quad _to) : from(_from), to(_to) {}
	Move(const Move& q) : from(q.from), to(q.to) {}
	Move& operator=(const Move& q) { from=q.from; to=q.to; return *this; }

	Quad getFrom() const { return from; }
	Quad getTo() const { return to; }
	
	Quad from;
	Quad to;
	
	static Move createMove(string s);
};

Move Move::createMove(string s)
{
	return Move(Quad::createQuad(s), Quad::createQuad(&s[2]));
}


class Rules
{
public:
	static bool CanMove(PieceType type, Move move, Color current);
	static bool CanEat(PieceType type, Move move, Color current);

	static bool pawnCanMove(Move move, Color current);
	static bool knightCanMove(Move move);
	static bool bishopCanMove(Move move);
	static bool rookCanMove(Move move);
	static bool queenCanMove(Move move);
	static bool kingCanMove(Move move);
	
	static bool pawnCanEat(Move move, Color current);
	static bool knightCanEat(Move move)	{ return knightCanMove(move); }
	static bool bishopCanEat(Move move)	{ return bishopCanMove(move); }
	static bool rookCanEat(Move move)	{ return rookCanMove(move); }
	static bool queenCanEat(Move move)	{ return queenCanMove(move); }
	static bool kingCanEat(Move move)	{ return kingCanMove(move); }
};

bool Rules::CanEat(PieceType type, Move move, Color current)
{
	switch (type)
	{
		case PieceType::Pawn: return pawnCanEat(move, current);
		case PieceType::Knight: return knightCanEat(move);
		case PieceType::Bishop: return bishopCanEat(move);
		case PieceType::Rook: return rookCanEat(move);
		case PieceType::Queen: return queenCanEat(move);
		case PieceType::King: return kingCanEat(move);
	}
	
	return false;
}

bool Rules::CanMove(PieceType type, Move move, Color current)
{
	switch (type)
	{
		case PieceType::Pawn: return pawnCanMove(move, current);
		case PieceType::Knight: return knightCanMove(move);
		case PieceType::Bishop: return bishopCanMove(move);
		case PieceType::Rook: return rookCanMove(move);
		case PieceType::Queen: return queenCanMove(move);
		case PieceType::King: return kingCanMove(move);
	}
	
	return false;
}

bool Rules::pawnCanMove(Move move, Color current)
{
	if (move.to.x != move.from.x)
		return false;

	int dy = move.to.y - move.from.y;
	
	return ((current == Color::White && (dy == 1 || (dy == 2 && move.from.y == 1))) || (current == Color::Black && (dy == -1 || (dy == -2 && move.from.y == 6))));
}

bool Rules::pawnCanEat(Move move, Color current)
{
	int dx = move.to.x - move.from.x;
	if (dx != +1 && dx != -1)
		return false;

	int dy = move.to.y - move.from.y;
	return (current == Color::White && dy == 1) || (current == Color::Black && dy == -1);
}

bool Rules::knightCanMove(Move move)
{
	int dx = move.to.x - move.from.x;
	int dy = move.to.y - move.from.y;
	return ((dx==2 || dx==-1) && (dy==1 || dy==-1))  ||  ((dy==2 || dy==-1) && (dx==1 || dx==-1));
}

bool Rules::bishopCanMove(Move move)
{
	int dx = move.to.x - move.from.x;
	int dy = move.to.y - move.from.y;
	return (dx==dy || dx==-dy) && (dx != 0 || dy != 0);
}

bool Rules::rookCanMove(Move move)
{
	int dx = move.to.x - move.from.x;
	int dy = move.to.y - move.from.y;
	return (dx==0 || dy == 0) && (dx != 0 || dy != 0);
}

bool Rules::queenCanMove(Move move)
{
	return rookCanMove(move) || bishopCanMove(move);
}

bool Rules::kingCanMove(Move move)
{
	int dx = move.to.x - move.from.x;
	int dy = move.to.y - move.from.y;
	return (dx >= -1 && dx <= 1 && dy >= -1 && dy <= 1) && (dx != 0 || dy != 0);
}



class ChessBoard
{
	array<Piece, 64> board;
	
	template<typename T>
	static inline int getIndex(const T x, const T y) { return reinterpret_cast<int>(y*8 + x); }
	
	template<typename T>
	static inline int getX(const T index) { return reinterpret_cast<int>(index) % 8; }

	template<typename T>
	static inline int getY(const T index) { return reinterpret_cast<int>(index) / 8; }
	
public:
	ChessBoard() { reset(); }
	
	void reset();

	Piece get(const int x, const int y) const;
	bool isMoveValid(const Move& m, Color current) const;
	bool ApplyMove(const Move& m, Color current);
	
	bool isPinned(Quad q, Color current) const;
	Quad getKingPosition(Color color) const;
};

void ChessBoard::reset()
{
	for (int c=0; c<2; c++)
	{
		int y= c==0?0:7;
		Color color= c==0?Color::White : Color::Black;
	
		board[getIndex(0, y)] = Piece(color, PieceType::Rook);
		board[getIndex(1, y)] = Piece(color, PieceType::Knight);
		board[getIndex(2, y)] = Piece(color, PieceType::Bishop);
		board[getIndex(3, y)] = Piece(color, PieceType::Queen);
		board[getIndex(4, y)] = Piece(color, PieceType::King);
		board[getIndex(5, y)] = Piece(color, PieceType::Bishop);
		board[getIndex(6, y)] = Piece(color, PieceType::Knight);
		board[getIndex(7, y)] = Piece(color, PieceType::Rook);

		y= y==0?1:6;
		
		for (int x=0; x<8; x++)
			board[getIndex(x, y)] = Piece(color, PieceType::Pawn);
	}
}

Piece ChessBoard::get(const int x, const int y) const
{
	return board[getIndex(x, y)];
}

bool ChessBoard::isMoveValid(const Move& m, Color current) const
{
	Piece source = board[getIndex(m.from.x, m.from.y)];	
	if (source.color != current)
		return false;
		
	if (isPinned(m.from, current))
		return false;
	
	Piece dest = board[getIndex(m.to.x, m.to.y)];	
	if (dest.color == current)
		return false;

	return dest.type == PieceType::Empty ? Rules::CanMove(source.type, m, current) : Rules::CanEat(source.type, m, current);
}

bool ChessBoard::ApplyMove(const Move& m, Color current)
{
	if (!isMoveValid(m, current))
		return false;
	
	board[getIndex(m.to.x, m.to.y)] = board[getIndex(m.from.x, m.from.y)];
	board[getIndex(m.from.x, m.from.y)] = Piece::EmptyQuad;
		
	return true;
}

Quad ChessBoard::getKingPosition(Color color) const
{
	for (int i=0; i<board.size(); i++)
		if (board[i].type == PieceType::King  && board[i].color == color)
			return Quad(getX(i), getY(i));

	return Quad::NoWhere;
}

bool ChessBoard::isPinned(Quad q, Color current) const
{
	Color othercolor = current == Color::White ? Color::Black : Color::White;
	Quad kingPos = getKingPosition(current);
	
	int dx = q.x - kingPos.x;
	int dy = q.y - kingPos.y;
	
	if (dx == 0)
		for (int y=q.y+sign(dy); y>=0 && y<8; y+= sign(dy))
		{
			auto content = board[getIndex(q.x, y)];
			if (content.color == othercolor)
				return (content.type == PieceType::Queen || content.type == PieceType::Rook);
		}
	else if (dy == 0)
		for (int x=q.x+sign(dx); x>=0 && x<8; x+= sign(dy))
		{
			auto content = board[getIndex(x, q.y)];
			if (content.color == othercolor)
				return (content.type == PieceType::Queen || content.type == PieceType::Rook);
		}
	else if (dx == dy)
		for (int d=sign(dx); q.x+d >= 0 && q.x+d < 8 && q.y+d >= 0 && q.y+d < 8; d+=sign(dx))
		{
			auto content = board[getIndex(q.x+d, q.y+d)];
			if (content.color == othercolor)
				return (content.type == PieceType::Queen || content.type == PieceType::Bishop);
		}
	else if (dx == -dy)
		for (int d=sign(dx); q.x+d >= 0 && q.x+d < 8 && q.y-d >= 0 && q.y-d < 8; d+=sign(dx))
		{
			auto content = board[getIndex(q.x+d, q.y-d)];
			if (content.color == othercolor)
				return (content.type == PieceType::Queen || content.type == PieceType::Bishop);
		}
	
	return false;
}

class Drawer
{
public:
	static void Draw(const ChessBoard& b);
	static char getChar(const Piece& p);
};

char Drawer::getChar(const Piece& p)
{
	if (p.color == Color::Black)
		switch (p.type)
		{
			case PieceType::Pawn : return 'p';
			case PieceType::Knight : return 'n';
			case PieceType::Bishop : return 'b';
			case PieceType::Rook : return 'r';
			case PieceType::Queen : return 'q';
			case PieceType::King : return 'k';
		}
	else if (p.color == Color::White)
		switch (p.type)
		{
			case PieceType::Pawn : return 'P';
			case PieceType::Knight : return 'N';
			case PieceType::Bishop : return 'B';
			case PieceType::Rook : return 'R';
			case PieceType::Queen : return 'Q';
			case PieceType::King : return 'K';
		}

	return ' ';
}

void Drawer::Draw(const ChessBoard& b)
{
	for (int y=7; y>=0; y--)
	{
		for (int x=0; x<8; x++)
			cout << getChar(b.get(x, y));
	
		cout << endl;
	}
}


class MoveList
{
	vector<Move> moves;
public:
	MoveList() : moves() {}
	
	Move get(int i) const { return moves.at(i); }
	void add(const Move& m) { moves.push_back(m); }
	void add(Move&& m) { moves.push_back(m); }

	int length() const { return moves.size(); }
};


class Match
{
	ChessBoard board;
	MoveList moves;
	
public:
	Match() : board(), moves() {}
	
	int getTurn() const { return moves.length(); }
	Color getCurrentPlayer() const	{ return getTurn() % 2 == 0 ? Color::White : Color::Black; }
	
	void play();
};

void Match::play()
{
	Drawer::Draw(board);

	while (1)
	{
		string input;
		cin >> input;
		if (input.compare("quit") == 0)
			break;
			
		Move move = Move::createMove(input);
		if (board.ApplyMove(move, getCurrentPlayer()))
		{
			moves.add(move);
			Drawer::Draw(board);
		}
		else
			cout << "invalid move." << endl;
	}	
	
	cout << "terminated." << endl;
}


int main()
{
	Match m;
	m.play();
}
