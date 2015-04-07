#include <iostream>
#include <vector>
#include <string>

class globals
{
	unsigned long int framen;
public:
	static constexpr double K = 1;
	static constexpr double DT = 1;
	static constexpr double M = 1;
	
	globals() : framen(0) {}
	
	void inc() { ++framen; }
	
	globals& operator++() { inc(); return *this; }
};


template<class precision=double>
struct vec3
{
	precision x,y,z;
//public:
	vec3() = default;
	explicit vec3(precision _x, precision _y, precision _z) : x(_x), y(_y), z(_z) {}
	//~vec3() {}
	
	vec3<precision> operator+(vec3<precision> v)  { return vec3<precision>(x+v.x, y+v.y, z+v.z); }
	vec3<precision> operator-(vec3<precision> v)  { return vec3<precision>(x-v.x, y-v.y, z-v.z); }
	vec3<precision> operator*(vec3<precision> v)  { return vec3<precision>(x*v.x, y*v.y, z*v.z); }
	vec3<precision> operator*(double k)					 { return vec3<precision>(x*k, y*k, z*k); }
	
	precision length() const { return sqrt(x*x + y*y + z*z); }
	void normalize() { precision l = length(); if (l != 0) { x*=l; y*=l; z*=l; } }
	
	vec3<precision>& operator=(const vec3<precision>& v) { x=v.x; y=v.y; z=v.z; return *this; }
	bool operator==(const vec3<precision>& v) { return x==v.x && y==v.y && z==v.z; }
	
	vec3<precision>& operator+=(const vec3<precision>& v) { x+=v.x; y+=v.y; z+=v.z; return *this; }
	vec3<precision>& operator-=(const vec3<precision>& v) { x-=v.x; y-=v.y; z-=v.z; return *this; }
	
	precision getX() const { return x; }
	precision getY() const { return y; }
	precision getZ() const { return z; }
	
	void setX(precision v) { x=v; }
	void setY(precision v) { y=v; }
	void setZ(precision v) { z=v; }
	
	void zero() { setX(0); setY(0); setZ(0); }
	
	static vec3<precision> sum(vec3<precision> a, vec3<precision> b) { return a+b; }
	static vec3<precision> sub(vec3<precision> a, vec3<precision> b) { return a-b; }
	static vec3<precision> dot(vec3<precision> a, vec3<precision> b) { return a*b; }
	static vec3<precision> mul(vec3<precision> a, double b) { return a*b; }
	static vec3<precision> norm(vec3<precision> a) { return a.normalize(); }
	
	static vec3<precision> lerp(vec3<precision> a, vec3<precision> b, double k) { return a*(1-k)+b*k; }
	
	static const vec3<precision> ZERO;
	static const vec3<precision> UNITX;
	static const vec3<precision> UNITY;
	static const vec3<precision> UNITZ;
};
template<class precision>
const vec3<precision> vec3<precision>::ZERO = vec3<precision>();
template<class precision>
const vec3<precision> vec3<precision>::UNITX = vec3<precision>(1, 0, 0);
template<class precision>
const vec3<precision> vec3<precision>::UNITY = vec3<precision>(0, 1, 0);
template<class precision>
const vec3<precision> vec3<precision>::UNITZ = vec3<precision>(0, 0, 1);
	

template<class precision=double>
class node
{
	vec3<precision> pos;
	vec3<precision> spd;
	vec3<precision> acc;
	
public:
	node() : pos(), spd(), acc() {}
	node(vec3<precision> p) : pos(p), spd(), acc() {}
	
	vec3<precision> getPos() const { return pos; }
	vec3<precision> getSpd() const { return spd; }
	vec3<precision> getAcc() const { return acc; }

	void zero() { acc.zero(); }
	
	void update(precision dt) {
		pos = pos + spd*dt + acc*(dt*dt/2);
		spd = spd + (acc*dt);
	
//		vec3<precision> newspd = spd + (acc * dt);
//		vec3<precision> newpos = pos + (newspd * dt);
	}
	
	static void apply(node& n1, node& n2, vec3<precision> bounds_correct) {
		const vec3<precision> nf = (n2.pos - n1.pos + bounds_correct) * (globals::K/globals::M);
		n1.acc += nf;
		n2.acc -= nf;
	}
};


template<class precision=double>
class grid
{
	unsigned long L;
	precision spacing;
	
	globals g;

	std::vector<node<precision>> nodes;
	std::vector<std::pair<unsigned long,unsigned long>> links;
	std::vector<vec3<precision>> bound_correct;
	
	unsigned long index(unsigned long x, unsigned long y) const { return L*y+x; }
	unsigned long xindex(unsigned long i) { return i%L; }
	unsigned long yindex(unsigned long i) { return i/L; }
	
	void initialize(unsigned long L, precision spacing, bool periodic_bounds=true) noexcept {
		const unsigned long minidx = periodic_bounds? 0 : 1;
		const unsigned long maxidx = periodic_bounds? L : L-1;
	
		for (unsigned long y=minidx; y<maxidx; y++) {
			const precision ry = (y - (precision)L/2 + 0.5)*spacing;
		
			for (unsigned long x=minidx; x<maxidx; x++) {
				const precision rx = (x - (precision)L/2 + 0.5)*spacing;
				nodes.push_back(node<precision>(vec3<precision>(rx, ry, 0)));
			}
		}
		
		for (unsigned long y=minidx; y<maxidx; y++)
			for (unsigned long x=minidx; x<maxidx; x++) {
				const unsigned long nextx = x<L-1 ? x+1 : 0;
				links.push_back(std::pair<unsigned long, unsigned long>(index(x, y), index(nextx, y)));
				bound_correct.push_back(nextx != 0 ? vec3<precision>::ZERO : vec3<precision>::mul(vec3<precision>::UNITX, (spacing*L)));
				
				if (L==2 && periodic_bounds && x==0) break;	// (0,1). with periodic this would add (1,0)
			}
		
		for (unsigned long x=minidx; x<maxidx; x++)
			for (unsigned long y=minidx; y<maxidx; y++) {
				const unsigned long nexty = y<L-1 ? y+1 : 0;
				links.push_back(std::pair<unsigned long, unsigned long>(index(x, y), index(x, nexty)));
				bound_correct.push_back(nexty != 0 ? vec3<precision>::ZERO : vec3<precision>::mul(vec3<precision>::UNITY, (spacing*L)));
				
				if (L==2 && periodic_bounds && y==0) break;	// (0,1). with periodic this would add (1,0)
			}
	}
	
public:
	explicit grid(unsigned long _L, precision _spacing) noexcept : L(_L), spacing(_spacing), g(), nodes(), links(), bound_correct() { initialize(L, spacing); }

	void update(precision dt) {
		g.inc();
	
		for (auto& n : nodes)
			n.zero();
	
		const unsigned long len = links.size();
		
		for (unsigned long i=0; i<len; i++) {
			auto& p = links[i];
		
			//node<>::apply(nodes[p.first], nodes[p.second], p.first<p.second? vec3<>::ZERO : vec3<>::mul((xindex(p.first)!=xindex(p.second) ? vec3<>::UNITY : vec3<>::UNITX), spacing*L));
			node<>::apply(nodes[p.first], nodes[p.second], bound_correct[i]);
		}
		
		for (auto& n : nodes)
			n.update(dt);
	}
		
	void output() {
		const unsigned long len = nodes.size();
		
		std::cout << "frame:[" << std::endl;
		for (unsigned long i=0; i<len; i++) {
			const auto n = nodes[i];
			const auto pos = n.getPos();
			
			std::cout << "\tnode {" << std::endl;
			std::cout << "\t\t\"id\": " << i << std::endl;
			std::cout << "\t\t\"x\": " << pos.getX() << std::endl;
			std::cout << "\t\t\"y\": " << pos.getY() << std::endl;
			std::cout << "\t\t\"z\": " << pos.getZ() << std::endl;
			
			if (i != len-1)
				std::cout << "\t}," << std::endl;
			else
				std::cout << "\t}" << std::endl;
			
			//std::cout << i << "\t" << pos.getX() << " " << pos.getY() << " " << pos.getZ() << std::endl;
		}
		std::cout << "]" << std::endl;
	}
};


int main()
{
	grid<> g(4, 1);
	
	for (int t=0; t<10; t++) {
		g.output();
		g.update(1);	
	}
		
	return 0;
}
