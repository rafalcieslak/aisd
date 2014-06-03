#include <cstdio>
#include <set>
#include <queue>
#include <list>

enum SegTypes{ SegType_DR, SegType_R, SegType_UR, SegType_U, SegType_FakeSearch};
enum EvTypes{ EvType_START, EvType_END, EvType_CROSSING };
unsigned int currx, curry;

struct V2d{
	V2d(int x_, int y_) : x(x_), y(y_) {}
	int x ,y;
	V2d operator+(const V2d& other) const __attribute__((pure)) {return V2d(x+other.x, y+other.y);}
	V2d operator-(const V2d& other) const __attribute__((pure)) {return V2d(x-other.x, y-other.y);}
	V2d operator*(const int k)      const __attribute__((pure)) {return V2d(x*k, y*k);}
	int operator^(const V2d& other) const __attribute__((pure)) { return x*other.y - y*other.x; } // Magnitude of cross product
};
V2d normals[5]{ {1,-1}, {1,0}, {1,1}, {0,1}, {0,0} };
struct Segment{
	unsigned int x, y;
	int dx, dy;
	SegTypes type;
	Segment(unsigned int a, unsigned int b, int c, int d, bool fake = false) : x(a),y(c),dx(b),dy(d) {
		if(fake) type = SegType_FakeSearch;
		else if(dx == 0) type = SegType_U;
		else if(dy == 0) type = SegType_R;
		else if(dy < 0 ) type = SegType_DR;
		else             type = SegType_UR;
	}
	unsigned int ycorr_at_curr() const __attribute__((hot, pure)){
		if     (type == SegType_U && currx == x) return curry;
		else if(type == SegType_U && currx != x) return curry+2;
		else if(type == SegType_R) return y;
		else if(type == SegType_DR) return y-(currx-x);
		else if(type == SegType_UR) return y+(currx-x);
		else return curry; // for FakeSearch
	}
	V2d get_normal() const __attribute__((hot,const)){ return normals[type];}
	int len() const __attribute__((hot,const)){ return (type==SegType_U)?dy:dx; }
};
std::vector<Segment> segments;

struct Event{
	int x, y, segment_id;
	EvTypes type;
	bool operator<(const Event& other) const __attribute__((hot,pure)){ return !(x < other.x || (x == other.x && y < other.y) ); }
};
struct ActiveSegment{ mutable unsigned int segment_id; };
bool operator<(const ActiveSegment& a, const ActiveSegment& b){ return (segments[a.segment_id].ycorr_at_curr() < segments[b.segment_id].ycorr_at_curr()); }

V2d find_intersect(unsigned int seg1, unsigned int seg2) __attribute__((hot,const));
V2d find_intersect(unsigned int seg1, unsigned int seg2){
	Segment s1 = segments[seg1], s2 = segments[seg2];
	V2d p(s1.x, s1.y), q(s2.x, s2.y);
	V2d r = s1.get_normal(), s = s2.get_normal();
	int m = s^r;
	if(m == 0) return V2d(-1,-1);
	int l = ((p-q)^r)/m, k = ((p-q)^s)/m;
	if(0 <= k && k <= s1.len() && 0 <= l && l <= s2.len()) return p+r*k;
	return V2d(-1,-1);
}
std::priority_queue<Event> event_queue;
std::set<ActiveSegment> active_segments;

bool add_potential_intersection(unsigned int s1, unsigned int s2){
	V2d i = find_intersect( s1,s2 );
	if(i.x == -1) return false;
	if(i.x > (int)currx || (i.x == (int)currx && i.y > (int)curry)) event_queue.push({i.x,i.y,-1,EvType_CROSSING});
	else if(i.x == (int)currx && i.y == (int)curry) return true; // probably a crossing which we'll never see anymore
	return false;
}

int main() __attribute__((hot));
int main(){
	unsigned int N;
	bool anything = false;
	scanf("%d",&N);
	segments.reserve(N);
	for(unsigned int i = 0; i < N; i++){
		int x1,x2,y1,y2;
		scanf("%d %d %d %d",&x1,&y1,&x2,&y2);
		if(x2 < x1 || (x1 == x2 && y2 < y1) ){
			std::swap(x1,x2);
			std::swap(y1,y2);
		}
		segments.push_back(Segment(4*x1,4*(x2-x1),4*y1,4*(y2-y1)));
		event_queue.push({4*x1,4*y1,(int)i,EvType_START});
		event_queue.push({4*x2,4*y2,(int)i,EvType_END});
	}
	segments.push_back(Segment(0,0,0,0,true)); // fakesearch
	while(!event_queue.empty()){
		std::list<unsigned int> starts, ends;
		currx = event_queue.top().x;
		curry = event_queue.top().y;
		bool this_point_is_a_crossing = false;
		while(!event_queue.empty() && event_queue.top().x == (int)currx && event_queue.top().y == (int)curry){
			Event currev = event_queue.top();
			event_queue.pop();
			if(currev.type == EvType_START)  starts.push_back(currev.segment_id);
			else if(currev.type == EvType_END) ends.push_back(currev.segment_id);
			else if(currev.type == EvType_CROSSING) anything |= this_point_is_a_crossing = true;
		}
		if(this_point_is_a_crossing){
			std::set<ActiveSegment>::iterator it = active_segments.find({N}); // search by fake activesegment (totally looking forward to C++14)
			std::vector< std::set<ActiveSegment>::iterator > iters;
			while(it != active_segments.end() && segments[it->segment_id].ycorr_at_curr() == curry) {iters.push_back(it++);}
			unsigned int n = iters.size();
			if(n == 2)      std::swap(iters[0]->segment_id, iters[1]->segment_id);
			else if(n == 3) std::swap(iters[0]->segment_id, iters[2]->segment_id);
			else if(n == 4){
				std::swap(iters[0]->segment_id, iters[3]->segment_id);
				std::swap(iters[1]->segment_id, iters[2]->segment_id);
			}
			if(iters[0] != active_segments.begin()){
				unsigned int x = iters[0]->segment_id;
				iters[0]--; // crossing with the topmost swapped element
				add_potential_intersection(x,iters[0]->segment_id);
			}
			unsigned int x = iters[n-1]->segment_id;
			iters[n-1]++;	// crossing with the downmost swapped segment
			if(iters[n-1] != active_segments.end()) add_potential_intersection(x, iters[n-1]->segment_id );
		}
		if(starts.size() + ends.size()>=2) anything |= this_point_is_a_crossing = true;
		for(unsigned int endid : ends){
			auto it = active_segments.find({ endid });
			while(it->segment_id != endid) it++;
			auto next = it; next++;
			if(it != active_segments.begin() && next != active_segments.end()){
				auto prev = it; prev--;
				add_potential_intersection( prev->segment_id, next->segment_id );
			}
			active_segments.erase(it);
		}
		for(unsigned int startid : starts){
			currx++; // timewind!
			auto it = active_segments.insert({ startid }).first;
			currx--;
			if(it != active_segments.begin()){
				auto prev = it; prev--;
				anything |= this_point_is_a_crossing |= add_potential_intersection( it->segment_id, prev->segment_id );
			}
			auto next = it; next++;
			if(next != active_segments.end()){
				anything |= this_point_is_a_crossing |= add_potential_intersection( it->segment_id, next->segment_id );
			}
		}
		if(this_point_is_a_crossing) printf("%.1f %.1f\n", currx/4.0, curry/4.0);
	}
	if(!anything) printf("BRAK\n");
	return 0;
}