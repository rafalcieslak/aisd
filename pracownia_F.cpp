#include <cstdio>
#include <list>
#include <vector>

int H[1001][1001];
struct coords{int y; int x;};
bool operator==(const coords& c1, const coords& c2) {return (c1.x == c2.x) && (c1.y == c2.y);}
struct UFelem{coords parent; int rank;};
std::vector<int> T, results;
std::vector<std::list<coords>> levels;
UFelem UF[1001][1001];

inline coords& GetParent(coords c) {return UF[c.y][c.x].parent;};
inline int& GetRank(coords c) {return UF[c.y][c.x].rank;};
coords FindRoot(coords c){
	if(!(GetParent(c) == c)) GetParent(c) = FindRoot(GetParent(c));
	return GetParent(c);
}
bool Union(coords c1, coords c2){
	coords r1 = FindRoot(c1), r2 = FindRoot(c2);
	if(r1 == r2) return false;
	if(GetRank(r1) > GetRank(r2))       GetParent(r2) = r1;
	else if(GetRank(r1) < GetRank(r2))  GetParent(r1) = r2;
	else{
		GetParent(r1) = r2;
		GetRank(r2)++;
	}
	return true;
}

int binf(int q, int start, int end){
	if(start+1 == end) return start;
	int m = (start+end)/2;
	if(q > T[m]) return binf(q,m,end);
	else return binf(q,start,m);
}
int binf(int q) __attribute__((const));
int binf(int q) {
	if(q <= T[0]) return 0;
	if(q > T[T.size()-1]) return T.size();
	return binf(q,0,T.size());
}

int main() __attribute__((hot));
int main(){
	int n, regions = 0, x_size, y_size, inf = 2000000000;
	scanf("%d %d", &y_size, &x_size);
	for(int y = 0; y < y_size; y++) for(int x = 0; x < x_size; x++) scanf("%d", &H[y][x]);
	scanf("%d",&n);
	T.resize(n+2); results.resize(n); levels.resize(n+2);
	T[0]=0; T[n+1]=inf;
	for(int i = 0; i < n; i++) scanf("%d",&T[i+1]);
	for(int y = 0; y < y_size; y++)
		for(int x = 0; x < x_size; x++){
			levels[binf(H[y][x])].push_back(coords{y,x});
			UF[y][x] = UFelem{coords{y,x},0};
		}
	for(int i = T.size()-1; i >= 0; i--){
		for(coords c : levels[i]){
			regions++;
			if(c.x>0        && H[c.y][c.x-1] > T[i] && Union(coords{c.y,c.x},coords{c.y,c.x-1})) regions--;
			if(c.x<x_size-1 && H[c.y][c.x+1] > T[i] && Union(coords{c.y,c.x},coords{c.y,c.x+1})) regions--;
			if(c.y>0        && H[c.y-1][c.x] > T[i] && Union(coords{c.y,c.x},coords{c.y-1,c.x})) regions--;
			if(c.y<y_size-1 && H[c.y+1][c.x] > T[i] && Union(coords{c.y,c.x},coords{c.y+1,c.x})) regions--;
		}
		if(i != n+1 && i != 0) results[i-1] = regions;
	}
	for(auto &x : results) printf("%d ",x);
}