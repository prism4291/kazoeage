#include <iostream>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <array>
#include <chrono>
#include <boost/multiprecision/cpp_int.hpp>

struct Edge{
    uint64_t from;
    uint64_t to;
};

inline unsigned int pattern_at(const uint64_t &pattern,const int index){
    return (pattern>>(index*2))&3;
}
inline void pattern_set(uint64_t &pattern,const int index,const unsigned int value){
    pattern = (pattern & ~(static_cast<uint64_t>(3) << (index * 2))) | (static_cast<uint64_t>(value) << (index * 2));
}

inline unsigned int indices_at(const uint32_t &indices,const int index){
    return (indices>>index)&1;
}

void print_pattern(const uint64_t &pattern){
    static const std::array<const char, 4> char_map = {'O', 'S', 'A', 'B'};
    for(auto i=0;i<32;++i){
        std::cout<<char_map.at(pattern_at(pattern,i));
    }
    std::cout<<std::endl;
}

bool validate(const uint64_t &pattern,const uint64_t &new_pattern,const int index,const uint32_t indices){
    if(pattern_at(new_pattern,index)==2&&pattern_at(new_pattern,index+1)==3){
        //A-B
        return false;
    }
    if(index>0){
        if(pattern_at(pattern,index)!=0&&indices_at(indices,index-1)!=0){
            //-A- -B- -S-
            return false;
        }
    }
    return true;
}

uint64_t horizon(std::unordered_map<uint64_t,std::array<uint64_t,32>> &horizon_cache,const uint64_t &pattern,const int index){
    if(horizon_cache.contains(pattern)){
        if(horizon_cache.at(pattern).at(index)!=0) {
            return horizon_cache.at(pattern).at(index);
        }
    }else{
        horizon_cache.emplace(pattern,std::array<uint64_t,32>{});
    }
    uint64_t new_pattern=pattern;
    const unsigned int l=pattern_at(new_pattern,index);
    const unsigned int r=pattern_at(new_pattern,index+1);
    //A-B is invalid
    //A-S S-B S-S cannot appear
    if(l==0&&r==0){
        //O-O
        pattern_set(new_pattern,index,2);
        pattern_set(new_pattern,index+1,3);
    }else if(l==3&&r==2){
        //B-A
        pattern_set(new_pattern,index,0);
        pattern_set(new_pattern,index+1,0);
    }else if(l==0){
        //O-A O-B O-S
        pattern_set(new_pattern,index,pattern_at(new_pattern,index+1));
        pattern_set(new_pattern,index+1,0);
    }else if(r==0){
        //A-O B-O S-O
        pattern_set(new_pattern,index+1,pattern_at(new_pattern,index));
        pattern_set(new_pattern,index,0);
    }else if(l==3){
        //B-B B-S
        int nests=0;
        for(int index2=index;index2>=0;--index2){
            if(pattern_at(new_pattern,index2)==2){
                ++nests;
            }else if(pattern_at(new_pattern,index2)==3){
                --nests;
            }
            if(nests==0){
                pattern_set(new_pattern,index2,pattern_at(new_pattern,index+1));
                break;
            }
        }
        pattern_set(new_pattern,index,0);
        pattern_set(new_pattern,index+1,0);
    }else if(r==2){
        //A-A S-A
        int nests=0;
        for(int index2=index+1;index2<32;++index2){
            if(pattern_at(new_pattern,index2)==2){
                ++nests;
            }else if(pattern_at(new_pattern,index2)==3){
                --nests;
            }
            if(nests==0){
                pattern_set(new_pattern,index2,pattern_at(new_pattern,index));
                break;
            }
        }
        pattern_set(new_pattern,index,0);
        pattern_set(new_pattern,index+1,0);
    }
    horizon_cache.at(pattern).at(index)=new_pattern;
    return new_pattern;
}

void solve(int N,int M){
    //std::cout<<N<<"x"<<M<<" --------------------------------"<<std::endl;
    uint64_t initial_pattern=1;
    std::unordered_map<uint64_t,std::array<uint64_t,32>> horizon_cache={};
    std::vector<uint64_t> patterns={initial_pattern};
    std::unordered_map<uint64_t,uint64_t> patterns_map={{initial_pattern,0}};
    std::vector<Edge> edges={};
    size_t t=0;
    while(t<patterns.size()){
        edges.emplace_back(t,t);
        std::vector<uint32_t> next_indices={0};
        //print_pattern(patterns.at(t));
        std::unordered_map<uint32_t,uint64_t> pattern_cache={{0,patterns.at(t)}};
        size_t k=0;
        while(k < next_indices.size()){
            int index=0;
            while((next_indices.at(k) >> index) != 0){
                ++index;
            }
            while(index<N){
                if(validate(patterns.at(t), pattern_cache.at(next_indices.at(k)), index, next_indices.at(k))){
                    uint32_t new_indice= next_indices.at(k) | (1U << index);
                    next_indices.push_back(new_indice);
                    uint64_t new_pattern= horizon(horizon_cache,pattern_cache.at(next_indices.at(k)), index);
                    if(!patterns_map.contains(new_pattern)){
                        patterns.push_back(new_pattern);
                        patterns_map.emplace(new_pattern, patterns.size() - 1);
                    }
                    //std::cout<<"t:"<<t<<",n:"<<patterns_map.at(new_pattern)<<std::endl;
                    edges.emplace_back(t, patterns_map.at(new_pattern));
                    pattern_cache.emplace(new_indice, new_pattern);

                }
                ++index;
            }
            ++k;
        }
        ++t;
        //if(t%1000==0){
            //std::cout<<edges.size()<<"/"<<edges.capacity()<<std::endl;
            //std::cout<<t<<"/"<<patterns.size()<<"   "<<horizon_cache.size()<<std::endl;
        //}
    }
    //std::cout<<patterns.size()<<","<<edges.size()<<std::endl;
    std::vector<boost::multiprecision::cpp_int> state(patterns.size(),0);
    state.at(0)=1;
    for(auto h=0;h<M+1;++h){
        //std::cout<<h<<"/"<<M+1<<std::endl;
        std::vector<boost::multiprecision::cpp_int> new_state(patterns.size(),0);
        for(const Edge &edge:edges) {
            new_state.at(edge.to) +=state.at(edge.from);
        }
        state=std::move(new_state);

    }
    //std::cout<<M+1<<"/"<<M+1<<std::endl;
    std::cout<<N<<"x"<<M<<" : "<<state.at(patterns_map.at(static_cast<uint64_t>(1)<<(N*2)))<<" ----------------"<<std::endl;
}


int main() {
    for(auto i=1;i<15;++i) {
        auto start = std::chrono::system_clock::now();
        solve(i, i);
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed2 = end - start;
        printf("time %lf[s]\n", elapsed2.count());
    }
    return 0;
}
