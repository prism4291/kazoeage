#include <iostream>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <array>
#include <chrono>
#include <boost/multiprecision/cpp_int.hpp>

inline unsigned int pattern_at(const uint64_t &pattern,const int index){
    return (pattern>>(index*2))&3;
}
inline void pattern_set(uint64_t &pattern,const int index,const unsigned int value){
    pattern = (pattern & ~(static_cast<uint64_t>(3) << (index << 1))) | (static_cast<uint64_t>(value) << (index << 1));
}

inline unsigned int indices_at(const uint32_t &indices,const int index){
    return (indices>>index)&1;
}

void print_pattern(const uint64_t &pattern){
    static constexpr std::array<const char, 4> char_map = {'O', 'M', 'A', 'B'};
    for(auto i=0;i<32;++i){
        std::cout<<char_map.at(pattern_at(pattern,i));
    }
    std::cout<<std::endl;
}
bool horizon(uint64_t &pattern,const int index){
    const unsigned int l=pattern_at(pattern,index);
    const unsigned int r=pattern_at(pattern,index+1);
    //A-B is invalid
    //A-S S-B S-S cannot appear
    if(l==0&&r==0){
        //O-O
        pattern_set(pattern,index,2);
        pattern_set(pattern,index+1,3);
    }else if(l==3&&r==2){
        //B-A
        pattern_set(pattern,index,0);
        pattern_set(pattern,index+1,1);
    }else if(l==0){
        //O-A O-B O-S
        pattern_set(pattern,index,pattern_at(pattern,index+1));
        pattern_set(pattern,index+1,1);
    }else if(r==0){
        //A-O B-O S-O
        pattern_set(pattern,index+1,pattern_at(pattern,index));
        pattern_set(pattern,index,0);
    }else if(l==3){
        //B-B B-S
        int nests=0;
        for(int index2=index;index2>=0;--index2){
            if(pattern_at(pattern,index2)==2){
                ++nests;
            }else if(pattern_at(pattern,index2)==3){
                --nests;
            }
            if(nests==0){
                pattern_set(pattern,index2,pattern_at(pattern,index+1));
                break;
            }
        }
        pattern_set(pattern,index,0);
        pattern_set(pattern,index+1,1);
    }else if(r==2){
        //A-A S-A
        int nests=0;
        for(int index2=index+1;index2<32;++index2){
            if(pattern_at(pattern,index2)==2){
                ++nests;
            }else if(pattern_at(pattern,index2)==3){
                --nests;
            }
            if(nests==0){
                pattern_set(pattern,index2,pattern_at(pattern,index));
                break;
            }
        }
        pattern_set(pattern,index,0);
        pattern_set(pattern,index+1,1);
    }else {
        return false;
    }
    return true;
}

void solve(int N,int M){
    std::cout<<N<<"x"<<M<<" --------------------------------"<<std::endl;
    uint64_t initial_pattern=2;
    std::unordered_map<uint64_t,boost::multiprecision::cpp_int> patterns={{initial_pattern,1}};
    std::vector<int> indexes={};
    for (int i=0;i<N;++i) {
        for (int j=i%2;j<i+1;j+=2) {
            indexes.push_back(j);
        }
    }
    for (int i=0;i<(N-M+(N+1)/2)*2;++i) {
        for (int j=(N+i)%2;j<N;j+=2) {
            indexes.push_back(j);
        }
    }
    for (int i=N%2;i<N;++i) {
        for (int j=i;j<N;j+=2) {
            indexes.push_back(j);
        }
    }
    auto patterns_new=patterns;
    for (const int index:indexes) {
        for (const auto& [p, c] : patterns) {
            auto p_new=p;
            if (pattern_at(p_new,index)==1) {
                pattern_set(p_new,index,0);
                if (patterns_new.contains(p_new)) {
                    patterns_new.at(p_new)+=c;
                }else {
                    patterns_new.emplace(p_new,c);
                }
                patterns_new.erase(p);
                continue;
            }
            if (horizon(p_new,index)) {
                if (index+1==N&&pattern_at(p_new,index+1)==1) {
                    pattern_set(p_new,index+1,0);
                }
                if (patterns_new.contains(p_new)) {
                    patterns_new.at(p_new)+=c;
                }else {
                    patterns_new.emplace(p_new,c);
                }
            }
        }
        patterns=patterns_new;
    }
    std::cout<<patterns.at(initial_pattern<<(N*2))<<std::endl;
}


int main() {
    for(auto i=1;i<13;++i) {
        auto start = std::chrono::system_clock::now();
        solve(i, i);
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed2 = end - start;
        printf("time %lf[s]\n", elapsed2.count());
    }
    return 0;
}
